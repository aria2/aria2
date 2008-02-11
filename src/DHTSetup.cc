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
#include "DHTSetup.h"
#include "LogFactory.h"
#include "Logger.h"
#include "Util.h"
#include "File.h"
#include "DHTNode.h"
#include "DHTConnectionImpl.h"
#include "DHTRoutingTable.h"
#include "DHTMessageFactoryImpl.h"
#include "DHTMessageTracker.h"
#include "DHTMessageDispatcherImpl.h"
#include "DHTMessageReceiver.h"
#include "DHTTaskQueueImpl.h"
#include "DHTTaskFactoryImpl.h"
#include "DHTPeerAnnounceStorage.h"
#include "DHTTokenTracker.h"
#include "DHTInteractionCommand.h"
#include "DHTTokenUpdateCommand.h"
#include "DHTBucketRefreshCommand.h"
#include "DHTPeerAnnounceCommand.h"
#include "DHTEntryPointNameResolveCommand.h"
#include "DHTAutoSaveCommand.h"
#include "DHTTask.h"
#include "DHTRoutingTableDeserializer.h"
#include "DHTRegistry.h"
#include "DHTBucketRefreshTask.h"
#include "CUIDCounter.h"
#include "prefs.h"
#include "Option.h"
#include "SocketCore.h"
#include "DlAbortEx.h"
#include "RecoverableException.h"
#include <fstream>

namespace aria2 {

size_t DHTSetup::_initialized = 0;

DHTSetup::DHTSetup():_logger(LogFactory::getInstance()) {}

DHTSetup::~DHTSetup() {}

Commands DHTSetup::setup(DownloadEngine* e, const Option* option)
{
  if(_initialized) {
    return Commands();
  }
  try {
    // load routing table and localnode id here

    SharedHandle<DHTNode> localNode = 0;

    DHTRoutingTableDeserializer deserializer;
    std::string dhtFile = option->get(PREF_DHT_FILE_PATH);
    if(File(dhtFile).isFile()) {
      try {
	std::ifstream in(dhtFile.c_str());
	in.exceptions(std::ios::failbit);
	deserializer.deserialize(in);
	localNode = deserializer.getLocalNode();
      } catch(RecoverableException* e) {
	_logger->error("Exception caught while loading DHT routing table from %s",
		       e, dhtFile.c_str());
	delete e;
      }
    }
    if(localNode.isNull()) {
      localNode = new DHTNode();
    }

    SharedHandle<DHTConnectionImpl> connection = new DHTConnectionImpl();
    {
      IntSequence seq = Util::parseIntRange(option->get(PREF_DHT_LISTEN_PORT));
      uint16_t port = connection->bind(seq);
      if(port == 0) {
	throw new DlAbortEx("Error occurred while binding port for DHT");
      }
      localNode->setPort(port);
    }
    _logger->debug("Initialized local node ID=%s",
		   Util::toHex(localNode->getID(), DHT_ID_LENGTH).c_str());

    SharedHandle<DHTRoutingTable> routingTable = new DHTRoutingTable(localNode);

    SharedHandle<DHTMessageFactoryImpl> factory = new DHTMessageFactoryImpl();

    SharedHandle<DHTMessageTracker> tracker = new DHTMessageTracker();

    SharedHandle<DHTMessageDispatcherImpl> dispatcher = new DHTMessageDispatcherImpl(tracker);

    SharedHandle<DHTMessageReceiver> receiver = new DHTMessageReceiver(tracker);

    SharedHandle<DHTTaskQueue> taskQueue = new DHTTaskQueueImpl();

    SharedHandle<DHTTaskFactoryImpl> taskFactory = new DHTTaskFactoryImpl();

    SharedHandle<DHTPeerAnnounceStorage> peerAnnounceStorage = new DHTPeerAnnounceStorage();

    SharedHandle<DHTTokenTracker> tokenTracker = new DHTTokenTracker();

    // wiring up
    tracker->setRoutingTable(routingTable);
    tracker->setMessageFactory(factory);

    receiver->setConnection(connection);
    receiver->setMessageFactory(factory);
    receiver->setRoutingTable(routingTable);

    taskFactory->setLocalNode(localNode);
    taskFactory->setRoutingTable(routingTable);
    taskFactory->setMessageDispatcher(dispatcher);
    taskFactory->setMessageFactory(factory);
    taskFactory->setTaskQueue(taskQueue);

    routingTable->setTaskQueue(taskQueue);
    routingTable->setTaskFactory(taskFactory);

    peerAnnounceStorage->setTaskQueue(taskQueue);
    peerAnnounceStorage->setTaskFactory(taskFactory);

    factory->setRoutingTable(routingTable);
    factory->setConnection(connection);
    factory->setMessageDispatcher(dispatcher);
    factory->setPeerAnnounceStorage(peerAnnounceStorage);
    factory->setTokenTracker(tokenTracker);
    factory->setLocalNode(localNode);

    // assign them into DHTRegistry
    DHTRegistry::_localNode = localNode;
    DHTRegistry::_routingTable = routingTable;
    DHTRegistry::_taskQueue = taskQueue;
    DHTRegistry::_taskFactory = taskFactory;
    DHTRegistry::_peerAnnounceStorage = peerAnnounceStorage;
    DHTRegistry::_tokenTracker = tokenTracker;
    DHTRegistry::_messageDispatcher = dispatcher;
    DHTRegistry::_messageReceiver = receiver;
    DHTRegistry::_messageFactory = factory;

    // add deserialized nodes to routing table
    const std::deque<SharedHandle<DHTNode> >& desnodes = deserializer.getNodes();
    for(std::deque<SharedHandle<DHTNode> >::const_iterator i = desnodes.begin(); i != desnodes.end(); ++i) {
      routingTable->addNode(*i);
    }
    if(!desnodes.empty() && deserializer.getSerializedTime().elapsed(DHT_BUCKET_REFRESH_INTERVAL)) {
      SharedHandle<DHTBucketRefreshTask> task = taskFactory->createBucketRefreshTask();
      task->setForceRefresh(true);
      taskQueue->addPeriodicTask1(task);
    }

    Commands commands;
    if(!option->get(PREF_DHT_ENTRY_POINT_HOST).empty()) {
      {
	DHTEntryPointNameResolveCommand* command = new DHTEntryPointNameResolveCommand(CUIDCounterSingletonHolder::instance()->newID(), e);
	command->setTaskQueue(taskQueue);
	command->setTaskFactory(taskFactory);
	command->setLocalNode(localNode);
	commands.push_back(command);
      }
    } else {
      _logger->info("No DHT entry point specified.");
    }
    {
      DHTInteractionCommand* command = new DHTInteractionCommand(CUIDCounterSingletonHolder::instance()->newID(), e);
      command->setMessageDispatcher(dispatcher);
      command->setMessageReceiver(receiver);
      command->setTaskQueue(taskQueue);
      command->setReadCheckSocket(connection->getSocket());
      commands.push_back(command);
    }
    {
      DHTTokenUpdateCommand* command = new DHTTokenUpdateCommand(CUIDCounterSingletonHolder::instance()->newID(), e, DHT_TOKEN_UPDATE_INTERVAL);
      command->setTokenTracker(tokenTracker);
      commands.push_back(command);
    }
    {
      DHTBucketRefreshCommand* command = new DHTBucketRefreshCommand(CUIDCounterSingletonHolder::instance()->newID(), e, DHT_BUCKET_REFRESH_CHECK_INTERVAL);
      command->setTaskQueue(taskQueue);
      command->setRoutingTable(routingTable);
      command->setTaskFactory(taskFactory);
      commands.push_back(command);
    }
    {
      DHTPeerAnnounceCommand* command = new DHTPeerAnnounceCommand(CUIDCounterSingletonHolder::instance()->newID(), e, DHT_PEER_ANNOUNCE_CHECK_INTERVAL);
      command->setPeerAnnounceStorage(peerAnnounceStorage);
      commands.push_back(command);
    }
    {
      DHTAutoSaveCommand* command = new DHTAutoSaveCommand(CUIDCounterSingletonHolder::instance()->newID(), e, 30*60);
      command->setLocalNode(localNode);
      command->setRoutingTable(routingTable);
      commands.push_back(command);
    }
    _initialized = true;

    return commands;
  } catch(RecoverableException* e) {
    _logger->error("Exception caught while initializing DHT functionality. DHT is disabled.", e);
    delete e;
    DHTRegistry::clear();
    return Commands();
  }
}

bool DHTSetup::initialized()
{
  return _initialized;
}

} // namespace aria2
