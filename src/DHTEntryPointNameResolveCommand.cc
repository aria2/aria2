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
#ifdef ENABLE_ASYNC_DNS
#include "AsyncNameResolver.h"
#endif // ENABLE_ASYNC_DNS
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
#include "StringFormat.h"
#include "FileEntry.h"

namespace aria2 {

DHTEntryPointNameResolveCommand::DHTEntryPointNameResolveCommand(int32_t cuid, DownloadEngine* e, const std::deque<std::pair<std::string, uint16_t> >& entryPoints):
  Command(cuid),
  _e(e),
  _entryPoints(entryPoints),
  _bootstrapEnabled(false)
{}

DHTEntryPointNameResolveCommand::~DHTEntryPointNameResolveCommand()
{
#ifdef ENABLE_ASYNC_DNS
  disableNameResolverCheck(_resolver);
#endif // ENABLE_ASYNC_DNS
}

bool DHTEntryPointNameResolveCommand::execute()
{
  if(_e->_requestGroupMan->downloadFinished() || _e->isHaltRequested()) {
    return true;
  }
#ifdef ENABLE_ASYNC_DNS
  if(_resolver.isNull()) {
    _resolver.reset(new AsyncNameResolver());
  }
#endif // ENABLE_ASYNC_DNS
  try {
#ifdef ENABLE_ASYNC_DNS
    if(_e->option->getAsBool(PREF_ASYNC_DNS)) {
      while(_entryPoints.size()) {
        std::string hostname = _entryPoints.front().first;
        try {
          if(resolveHostname(hostname, _resolver)) {
            hostname = _resolver->getResolvedAddresses().front();
            std::pair<std::string, uint16_t> p(hostname,
                                               _entryPoints.front().second);
            _resolvedEntryPoints.push_back(p);
            addPingTask(p);
          } else {
            _e->commands.push_back(this);
            return false;
          }
        } catch(RecoverableException& e) {
          logger->error(EX_EXCEPTION_CAUGHT, e);
        }
        _resolver->reset();
        _entryPoints.erase(_entryPoints.begin());
      }
    } else
#endif // ENABLE_ASYNC_DNS
      {
        NameResolver res;
        res.setSocktype(SOCK_DGRAM);
        while(_entryPoints.size()) {
          std::string hostname = _entryPoints.front().first;
          try {
            std::deque<std::string> addrs;
            res.resolve(addrs, hostname);
          
            std::pair<std::string, uint16_t> p(addrs.front(),
                                               _entryPoints.front().second);
            _resolvedEntryPoints.push_back(p);
            addPingTask(p);
          } catch(RecoverableException& e) {
            logger->error(EX_EXCEPTION_CAUGHT, e);
          }
          _entryPoints.erase(_entryPoints.begin());
        }
      }
    if(_bootstrapEnabled && _resolvedEntryPoints.size()) {
      _taskQueue->addPeriodicTask1(_taskFactory->createNodeLookupTask(_localNode->getID()));
      _taskQueue->addPeriodicTask1(_taskFactory->createBucketRefreshTask());
    }
  } catch(RecoverableException& e) {
    logger->error(EX_EXCEPTION_CAUGHT, e);
  }
  return true;
}

void DHTEntryPointNameResolveCommand::addPingTask(const std::pair<std::string, uint16_t>& addr)
{
  SharedHandle<DHTNode> entryNode(new DHTNode());
  entryNode->setIPAddress(addr.first);
  entryNode->setPort(addr.second);
  
  _taskQueue->addPeriodicTask1(_taskFactory->createPingTask(entryNode, 10));
}

#ifdef ENABLE_ASYNC_DNS

bool DHTEntryPointNameResolveCommand::resolveHostname
(const std::string& hostname,
 const SharedHandle<AsyncNameResolver>& resolver)
{
  switch(resolver->getStatus()) {
  case AsyncNameResolver::STATUS_READY:
    logger->info(MSG_RESOLVING_HOSTNAME, cuid, hostname.c_str());
    resolver->resolve(hostname);
    setNameResolverCheck(resolver);
    return false;
  case AsyncNameResolver::STATUS_SUCCESS:
    logger->info(MSG_NAME_RESOLUTION_COMPLETE, cuid,
                 resolver->getHostname().c_str(),
                 resolver->getResolvedAddresses().front().c_str());
    return true;
    break;
  case AsyncNameResolver::STATUS_ERROR:
    throw DL_ABORT_EX
      (StringFormat(MSG_NAME_RESOLUTION_FAILED, cuid,
                    hostname.c_str(),
                    resolver->getError().c_str()).str());
  default:
    return false;
  }
}

void DHTEntryPointNameResolveCommand::setNameResolverCheck
(const SharedHandle<AsyncNameResolver>& resolver)
{
  _e->addNameResolverCheck(resolver, this);
}

void DHTEntryPointNameResolveCommand::disableNameResolverCheck
(const SharedHandle<AsyncNameResolver>& resolver)
{
  _e->deleteNameResolverCheck(resolver, this);
}
#endif // ENABLE_ASYNC_DNS

void DHTEntryPointNameResolveCommand::setBootstrapEnabled(bool f)
{
  _bootstrapEnabled = f;
}

void DHTEntryPointNameResolveCommand::setTaskQueue(const SharedHandle<DHTTaskQueue>& taskQueue)
{
  _taskQueue = taskQueue;
}

void DHTEntryPointNameResolveCommand::setTaskFactory(const SharedHandle<DHTTaskFactory>& taskFactory)
{
  _taskFactory = taskFactory;
}

void DHTEntryPointNameResolveCommand::setRoutingTable(const SharedHandle<DHTRoutingTable>& routingTable)
{
  _routingTable = routingTable;
}

void DHTEntryPointNameResolveCommand::setLocalNode(const SharedHandle<DHTNode>& localNode)
{
  _localNode = localNode;
}

} // namespace aria2
