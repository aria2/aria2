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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
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
#include "DNSCache.h"
#include "DlAbortEx.h"
#include "prefs.h"
#include "message.h"
#include "Util.h"
#include "Option.h"
#include "DHTNode.h"
#include "DHTTaskQueue.h"
#include "DHTTaskFactory.h"
#include "DHTTask.h"
#include "RequestGroupMan.h"

DHTEntryPointNameResolveCommand::DHTEntryPointNameResolveCommand(int32_t cuid, DownloadEngine* e):
  Command(cuid),
  _e(e),
  _resolver(new NameResolver()),
  _taskQueue(0),
  _taskFactory(0),
  _localNode(0)
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

  try {
    string hostname = _e->option->get(PREF_DHT_ENTRY_POINT_HOST);
    if(!Util::isNumbersAndDotsNotation(hostname)) {
      if(resolveHostname(hostname, _resolver)) {
	hostname = _resolver->getAddrString();
      } else {
	_e->commands.push_back(this);
	return false;
      }
    }
    
    DHTNodeHandle entryNode = new DHTNode();
    entryNode->setIPAddress(hostname);
    entryNode->setPort(_e->option->getAsInt(PREF_DHT_ENTRY_POINT_PORT));
 
    _taskQueue->addPeriodicTask1(_taskFactory->createPingTask(entryNode, 10));
    _taskQueue->addPeriodicTask1(_taskFactory->createNodeLookupTask(_localNode->getID()));
    _taskQueue->addPeriodicTask1(_taskFactory->createBucketRefreshTask());
  } catch(RecoverableException* e) {
    logger->error(EX_EXCEPTION_CAUGHT, e);
    delete e;
  }
  return true;
}

bool DHTEntryPointNameResolveCommand::resolveHostname(const string& hostname,
						      const NameResolverHandle& resolver)
{
  string ipaddr = DNSCacheSingletonHolder::instance()->find(hostname);
  if(ipaddr.empty()) {
#ifdef ENABLE_ASYNC_DNS
    switch(resolver->getStatus()) {
    case NameResolver::STATUS_READY:
      logger->info(MSG_RESOLVING_HOSTNAME, cuid, hostname.c_str());
      resolver->resolve(hostname);
      setNameResolverCheck(resolver);
      return false;
    case NameResolver::STATUS_SUCCESS:
      logger->info(MSG_NAME_RESOLUTION_COMPLETE, cuid,
		   hostname.c_str(), resolver->getAddrString().c_str());
      DNSCacheSingletonHolder::instance()->put(hostname, resolver->getAddrString());
      return true;
      break;
    case NameResolver::STATUS_ERROR:
      throw new DlAbortEx(MSG_NAME_RESOLUTION_FAILED, cuid,
			  hostname.c_str(),
			  resolver->getError().c_str());
    default:
      return false;
    }
#else
    logger->info(MSG_RESOLVING_HOSTNAME, cuid, hostname.c_str());
    resolver->resolve(hostname);
    logger->info(MSG_NAME_RESOLUTION_COMPLETE, cuid,
		 hostname.c_str(), resolver->getAddrString().c_str());
    DNSCacheSingletonHolder::instance()->put(hostname, resolver->getAddrString());
    return true;
#endif // ENABLE_ASYNC_DNS
  } else {
    logger->info(MSG_DNS_CACHE_HIT, cuid,
		 hostname.c_str(), ipaddr.c_str());
    resolver->setAddr(ipaddr);
    return true;
  }
}

#ifdef ENABLE_ASYNC_DNS
void DHTEntryPointNameResolveCommand::setNameResolverCheck(const NameResolverHandle& resolver) {
  _e->addNameResolverCheck(resolver, this);
}

void DHTEntryPointNameResolveCommand::disableNameResolverCheck(const NameResolverHandle& resolver) {
  _e->deleteNameResolverCheck(resolver, this);
}
#endif // ENABLE_ASYNC_DNS

void DHTEntryPointNameResolveCommand::setTaskQueue(const DHTTaskQueueHandle& taskQueue)
{
  _taskQueue = taskQueue;
}

void DHTEntryPointNameResolveCommand::setTaskFactory(const DHTTaskFactoryHandle& taskFactory)
{
  _taskFactory = taskFactory;
}

void DHTEntryPointNameResolveCommand::setLocalNode(const DHTNodeHandle& localNode)
{
  _localNode = localNode;
}
