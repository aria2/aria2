/* <!-- copyright */
/*
 * aria2 - The high speed download utility
 *
 * Copyright (C) 2006 Tatsuhiro Tsujikawa
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 *
 * In addition, as a special exception, the copyright holders give
 * permission to link the code of portions of this program with the
 * OpenSSL library under certain conditions as described in each
 * individual source file, and distribute linked combinations
 * including the two.
 * You must obey the GNU General Public License in all respects
 * for all of the code used other than OpenSSL.  If you modify
 * file(s) with this exception, you may extend this exception to your
 * version of the file(s), but you are not obligated to do so.  If you
 * do not wish to do so, delete this exception statement from your
 * version.  If you delete this exception statement from all source
 * files in the program, then also delete it here.
 */
/* copyright --> */
#include "DHTEntryPointNameResolveCommand.h"
#include "DownloadEngine.h"
#include "NameResolver.h"
#include "DlAbortEx.h"
#include "prefs.h"
#include "message.h"
#include "util.h"
#include "Option.h"
#include "DHTNode.h"
#include "DHTTaskQueue.h"
#include "DHTTaskFactory.h"
#include "DHTRoutingTable.h"
#include "DHTTask.h"
#include "RequestGroupMan.h"
#include "Logger.h"
#include "LogFactory.h"
#include "fmt.h"
#ifdef ENABLE_ASYNC_DNS
#include "AsyncNameResolver.h"
#endif // ENABLE_ASYNC_DNS

namespace aria2 {

DHTEntryPointNameResolveCommand::DHTEntryPointNameResolveCommand
(cuid_t cuid, DownloadEngine* e,
 const std::vector<std::pair<std::string, uint16_t> >& entryPoints):
  Command(cuid),
  e_(e),
  entryPoints_(entryPoints.begin(), entryPoints.end()),
  bootstrapEnabled_(false)
{}

DHTEntryPointNameResolveCommand::~DHTEntryPointNameResolveCommand()
{
#ifdef ENABLE_ASYNC_DNS
  disableNameResolverCheck(resolver_);
#endif // ENABLE_ASYNC_DNS
}

bool DHTEntryPointNameResolveCommand::execute()
{
  if(e_->getRequestGroupMan()->downloadFinished() || e_->isHaltRequested()) {
    return true;
  }
#ifdef ENABLE_ASYNC_DNS
  if(!resolver_) {
    int family;
    if(e_->getOption()->getAsBool(PREF_ENABLE_ASYNC_DNS6)) {
      family = AF_UNSPEC;
    } else {
      family = AF_INET;
    }
    resolver_.reset(new AsyncNameResolver(family
#ifdef HAVE_ARES_ADDR_NODE
                                          , e_->getAsyncDNSServers()
#endif // HAVE_ARES_ADDR_NODE
                                          ));
  }
#endif // ENABLE_ASYNC_DNS
  try {
#ifdef ENABLE_ASYNC_DNS
    if(e_->getOption()->getAsBool(PREF_ASYNC_DNS)) {
      while(!entryPoints_.empty()) {
        std::string hostname = entryPoints_.front().first;
        try {
          if(util::isNumericHost(hostname)) {
            std::pair<std::string, uint16_t> p
              (hostname, entryPoints_.front().second);
            resolvedEntryPoints_.push_back(p);
            addPingTask(p);
          } else if(resolveHostname(hostname, resolver_)) {
            hostname = resolver_->getResolvedAddresses().front();
            std::pair<std::string, uint16_t> p(hostname,
                                               entryPoints_.front().second);
            resolvedEntryPoints_.push_back(p);
            addPingTask(p);
          } else {
            e_->addCommand(this);
            return false;
          }
        } catch(RecoverableException& e) {
          A2_LOG_ERROR_EX(EX_EXCEPTION_CAUGHT, e);
        }
        resolver_->reset();
        entryPoints_.pop_front();
      }
    } else
#endif // ENABLE_ASYNC_DNS
      {
        NameResolver res;
        res.setSocktype(SOCK_DGRAM);
        while(!entryPoints_.empty()) {
          std::string hostname = entryPoints_.front().first;
          try {
            std::vector<std::string> addrs;
            res.resolve(addrs, hostname);
          
            std::pair<std::string, uint16_t> p(addrs.front(),
                                               entryPoints_.front().second);
            resolvedEntryPoints_.push_back(p);
            addPingTask(p);
          } catch(RecoverableException& e) {
            A2_LOG_ERROR_EX(EX_EXCEPTION_CAUGHT, e);
          }
          entryPoints_.pop_front();
        }
      }
    if(bootstrapEnabled_ && resolvedEntryPoints_.size()) {
      taskQueue_->addPeriodicTask1(taskFactory_->createNodeLookupTask
                                   (localNode_->getID()));
      taskQueue_->addPeriodicTask1(taskFactory_->createBucketRefreshTask());
    }
  } catch(RecoverableException& e) {
    A2_LOG_ERROR_EX(EX_EXCEPTION_CAUGHT, e);
  }
  return true;
}

void DHTEntryPointNameResolveCommand::addPingTask
(const std::pair<std::string, uint16_t>& addr)
{
  SharedHandle<DHTNode> entryNode(new DHTNode());
  entryNode->setIPAddress(addr.first);
  entryNode->setPort(addr.second);
  
  taskQueue_->addPeriodicTask1(taskFactory_->createPingTask(entryNode, 10));
}

#ifdef ENABLE_ASYNC_DNS

bool DHTEntryPointNameResolveCommand::resolveHostname
(const std::string& hostname,
 const SharedHandle<AsyncNameResolver>& resolver)
{
  switch(resolver->getStatus()) {
  case AsyncNameResolver::STATUS_READY:
      A2_LOG_INFO(fmt(MSG_RESOLVING_HOSTNAME,
                      getCuid(),
                      hostname.c_str()));
    resolver->resolve(hostname);
    setNameResolverCheck(resolver);
    return false;
  case AsyncNameResolver::STATUS_SUCCESS:
    A2_LOG_INFO(fmt(MSG_NAME_RESOLUTION_COMPLETE,
                    getCuid(),
                    resolver->getHostname().c_str(),
                    resolver->getResolvedAddresses().front().c_str()));
    return true;
    break;
  case AsyncNameResolver::STATUS_ERROR:
    throw DL_ABORT_EX
      (fmt(MSG_NAME_RESOLUTION_FAILED,
           getCuid(),
           hostname.c_str(),
           resolver->getError().c_str()));
  default:
    return false;
  }
}

void DHTEntryPointNameResolveCommand::setNameResolverCheck
(const SharedHandle<AsyncNameResolver>& resolver)
{
  e_->addNameResolverCheck(resolver, this);
}

void DHTEntryPointNameResolveCommand::disableNameResolverCheck
(const SharedHandle<AsyncNameResolver>& resolver)
{
  e_->deleteNameResolverCheck(resolver, this);
}
#endif // ENABLE_ASYNC_DNS

void DHTEntryPointNameResolveCommand::setBootstrapEnabled(bool f)
{
  bootstrapEnabled_ = f;
}

void DHTEntryPointNameResolveCommand::setTaskQueue
(const SharedHandle<DHTTaskQueue>& taskQueue)
{
  taskQueue_ = taskQueue;
}

void DHTEntryPointNameResolveCommand::setTaskFactory
(const SharedHandle<DHTTaskFactory>& taskFactory)
{
  taskFactory_ = taskFactory;
}

void DHTEntryPointNameResolveCommand::setRoutingTable
(const SharedHandle<DHTRoutingTable>& routingTable)
{
  routingTable_ = routingTable;
}

void DHTEntryPointNameResolveCommand::setLocalNode
(const SharedHandle<DHTNode>& localNode)
{
  localNode_ = localNode;
}

} // namespace aria2
