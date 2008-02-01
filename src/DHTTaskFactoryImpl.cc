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
#include "DHTTaskFactoryImpl.h"
#include "DHTNode.h"
#include "DHTRoutingTable.h"
#include "DHTMessageDispatcher.h"
#include "DHTMessageFactory.h"
#include "DHTTaskQueue.h"
#include "LogFactory.h"
#include "DHTPingTask.h"
#include "DHTNodeLookupTask.h"
#include "DHTBucketRefreshTask.h"
#include "DHTPeerLookupTask.h"
#include "DHTReplaceNodeTask.h"
#include "Peer.h"
#include "DHTNodeLookupEntry.h"
#include "BtContext.h"
#include "PeerStorage.h"
#include "BtRuntime.h"

DHTTaskFactoryImpl::DHTTaskFactoryImpl():_localNode(0),
					 _routingTable(0),
					 _dispatcher(0),
					 _factory(0),
					 _taskQueue(0),
					 _logger(LogFactory::getInstance()) {}

DHTTaskFactoryImpl::~DHTTaskFactoryImpl() {}

DHTTaskHandle
DHTTaskFactoryImpl::createPingTask(const DHTNodeHandle& remoteNode,
				   size_t numRetry)
{
  SharedHandle<DHTPingTask> task = new DHTPingTask(remoteNode, numRetry);
  setCommonProperty(task);
  return task;
}

DHTTaskHandle
DHTTaskFactoryImpl::createNodeLookupTask(const unsigned char* targetID)
{
  SharedHandle<DHTNodeLookupTask> task = new DHTNodeLookupTask(targetID);
  setCommonProperty(task);
  return task;
}

DHTTaskHandle
DHTTaskFactoryImpl::createBucketRefreshTask()
{
  SharedHandle<DHTBucketRefreshTask> task = new DHTBucketRefreshTask();
  setCommonProperty(task);
  return task;
}

DHTTaskHandle
DHTTaskFactoryImpl::createPeerLookupTask(const BtContextHandle& ctx)
{
  SharedHandle<DHTPeerLookupTask> task = new DHTPeerLookupTask(ctx);
  setCommonProperty(task);
  return task;
}

DHTTaskHandle
DHTTaskFactoryImpl::createPeerAnnounceTask(const unsigned char* infoHash)
{
  // TODO
  return 0;
}

DHTTaskHandle
DHTTaskFactoryImpl::createReplaceNodeTask(const DHTBucketHandle& bucket,
					  const DHTNodeHandle& newNode)
{
  SharedHandle<DHTReplaceNodeTask> task = new DHTReplaceNodeTask(bucket, newNode);
  setCommonProperty(task);
  return task;
}

void DHTTaskFactoryImpl::setCommonProperty(const DHTAbstractTaskHandle& task)
{
  task->setRoutingTable(_routingTable);
  task->setMessageDispatcher(_dispatcher);
  task->setMessageFactory(_factory);
  task->setTaskQueue(_taskQueue);
  task->setLocalNode(_localNode);
}

void DHTTaskFactoryImpl::setRoutingTable(const WeakHandle<DHTRoutingTable> routingTable)
{
  _routingTable = routingTable;
}

void DHTTaskFactoryImpl::setMessageDispatcher(const WeakHandle<DHTMessageDispatcher> dispatcher)
{
  _dispatcher = dispatcher;
}

void DHTTaskFactoryImpl::setMessageFactory(const WeakHandle<DHTMessageFactory> factory)
{
  _factory = factory;
}

void DHTTaskFactoryImpl::setTaskQueue(const WeakHandle<DHTTaskQueue> taskQueue)
{
  _taskQueue = taskQueue;
}

void DHTTaskFactoryImpl::setLocalNode(const DHTNodeHandle& localNode)
{
  _localNode = localNode;
}
