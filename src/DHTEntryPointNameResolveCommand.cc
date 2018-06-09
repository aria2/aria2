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
#include "SocketCore.h"
#ifdef ENABLE_ASYNC_DNS
#  include "AsyncNameResolverMan.h"
#endif // ENABLE_ASYNC_DNS

namespace aria2 {

DHTEntryPointNameResolveCommand::DHTEntryPointNameResolveCommand(
    cuid_t cuid, DownloadEngine* e, int family,
    const std::vector<std::pair<std::string, uint16_t>>& entryPoints)
    : Command{cuid},
      e_{e},
#ifdef ENABLE_ASYNC_DNS
      asyncNameResolverMan_{make_unique<AsyncNameResolverMan>()},
#endif // ENABLE_ASYNC_DNS
      taskQueue_{nullptr},
      taskFactory_{nullptr},
      routingTable_{nullptr},
      entryPoints_(std::begin(entryPoints), std::end(entryPoints)),
      family_{family},
      numSuccess_{0},
      bootstrapEnabled_{false}
{
#ifdef ENABLE_ASYNC_DNS
  configureAsyncNameResolverMan(asyncNameResolverMan_.get(), e_->getOption());
  asyncNameResolverMan_->setIPv4(family_ == AF_INET);
  asyncNameResolverMan_->setIPv6(family_ == AF_INET6);
#endif // ENABLE_ASYNC_DNS
}

DHTEntryPointNameResolveCommand::~DHTEntryPointNameResolveCommand()
{
#ifdef ENABLE_ASYNC_DNS
  asyncNameResolverMan_->disableNameResolverCheck(e_, this);
#endif // ENABLE_ASYNC_DNS
}

bool DHTEntryPointNameResolveCommand::execute()
{
  if (e_->getRequestGroupMan()->downloadFinished() || e_->isHaltRequested()) {
    return true;
  }
  try {
#ifdef ENABLE_ASYNC_DNS
    if (e_->getOption()->getAsBool(PREF_ASYNC_DNS)) {
      while (!entryPoints_.empty()) {
        std::string hostname = entryPoints_.front().first;
        std::vector<std::string> res;
        int rv = resolveHostname(res, hostname);
        if (rv == 0) {
          e_->addCommand(std::unique_ptr<Command>(this));
          return false;
        }
        else {
          if (rv == 1) {
            ++numSuccess_;
            std::pair<std::string, uint16_t> p(res.front(),
                                               entryPoints_.front().second);
            addPingTask(p);
          }
          asyncNameResolverMan_->reset(e_, this);
        }
        entryPoints_.pop_front();
      }
    }
    else
#endif // ENABLE_ASYNC_DNS
    {
      NameResolver res;
      res.setSocktype(SOCK_DGRAM);
      res.setFamily(family_);
      while (!entryPoints_.empty()) {
        std::string hostname = entryPoints_.front().first;
        try {
          std::vector<std::string> addrs;
          res.resolve(addrs, hostname);

          ++numSuccess_;
          std::pair<std::string, uint16_t> p(addrs.front(),
                                             entryPoints_.front().second);
          addPingTask(p);
        }
        catch (RecoverableException& e) {
          A2_LOG_ERROR_EX(EX_EXCEPTION_CAUGHT, e);
        }
        entryPoints_.pop_front();
      }
    }
    if (bootstrapEnabled_ && numSuccess_) {
      taskQueue_->addPeriodicTask1(
          taskFactory_->createNodeLookupTask(localNode_->getID()));
      taskQueue_->addPeriodicTask1(taskFactory_->createBucketRefreshTask());
    }
  }
  catch (RecoverableException& e) {
    A2_LOG_ERROR_EX(EX_EXCEPTION_CAUGHT, e);
  }
  return true;
}

void DHTEntryPointNameResolveCommand::addPingTask(
    const std::pair<std::string, uint16_t>& addr)
{
  auto entryNode = std::make_shared<DHTNode>();
  entryNode->setIPAddress(addr.first);
  entryNode->setPort(addr.second);

  taskQueue_->addPeriodicTask1(taskFactory_->createPingTask(entryNode, 10));
}

#ifdef ENABLE_ASYNC_DNS

int DHTEntryPointNameResolveCommand::resolveHostname(
    std::vector<std::string>& res, const std::string& hostname)
{
  if (!asyncNameResolverMan_->started()) {
    asyncNameResolverMan_->startAsync(hostname, e_, this);
  }

  switch (asyncNameResolverMan_->getStatus()) {
  case -1:
    A2_LOG_INFO(fmt(MSG_NAME_RESOLUTION_FAILED, getCuid(), hostname.c_str(),
                    asyncNameResolverMan_->getLastError().c_str()));
    return -1;
  case 0:
    return 0;
  case 1:
    asyncNameResolverMan_->getResolvedAddress(res);
    if (res.empty()) {
      A2_LOG_INFO(fmt(MSG_NAME_RESOLUTION_FAILED, getCuid(), hostname.c_str(),
                      "No address returned"));
      return -1;
    }
    else {
      A2_LOG_INFO(fmt(MSG_NAME_RESOLUTION_COMPLETE, getCuid(), hostname.c_str(),
                      res.front().c_str()));
      return 1;
    }
  }

  // Unreachable
  return 0;
}

#endif // ENABLE_ASYNC_DNS

void DHTEntryPointNameResolveCommand::setBootstrapEnabled(bool f)
{
  bootstrapEnabled_ = f;
}

void DHTEntryPointNameResolveCommand::setTaskQueue(DHTTaskQueue* taskQueue)
{
  taskQueue_ = taskQueue;
}

void DHTEntryPointNameResolveCommand::setTaskFactory(
    DHTTaskFactory* taskFactory)
{
  taskFactory_ = taskFactory;
}

void DHTEntryPointNameResolveCommand::setRoutingTable(
    DHTRoutingTable* routingTable)
{
  routingTable_ = routingTable;
}

void DHTEntryPointNameResolveCommand::setLocalNode(
    const std::shared_ptr<DHTNode>& localNode)
{
  localNode_ = localNode;
}

} // namespace aria2
