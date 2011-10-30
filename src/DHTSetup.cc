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
#include "DHTSetup.h"

#include <algorithm>

#include "LogFactory.h"
#include "Logger.h"
#include "util.h"
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
#include "DHTMessageCallback.h"
#include "prefs.h"
#include "Option.h"
#include "SocketCore.h"
#include "DlAbortEx.h"
#include "RecoverableException.h"
#include "a2functional.h"
#include "DownloadEngine.h"
#include "fmt.h"

namespace aria2 {

DHTSetup::DHTSetup() {}

DHTSetup::~DHTSetup() {}

void DHTSetup::setup
(std::vector<Command*>& commands, DownloadEngine* e, int family)
{
  if(family != AF_INET && family != AF_INET6) {
    return;
  }
  if((family == AF_INET && DHTRegistry::isInitialized()) ||
     (family == AF_INET6 && DHTRegistry::isInitialized6())) {
    return;
  }
  try {
    std::vector<Command*>* tempCommands = new std::vector<Command*>();
    auto_delete_container<std::vector<Command*> > commandsDel(tempCommands);
    // load routing table and localnode id here

    SharedHandle<DHTNode> localNode;

    DHTRoutingTableDeserializer deserializer(family);
    const std::string& dhtFile =
      e->getOption()->get(family == AF_INET?PREF_DHT_FILE_PATH:
                          PREF_DHT_FILE_PATH6);
    try {
      deserializer.deserialize(dhtFile);
      localNode = deserializer.getLocalNode();
    } catch(RecoverableException& e) {
      A2_LOG_ERROR_EX
        (fmt("Exception caught while loading DHT routing table from %s",
             dhtFile.c_str()),
         e);
    }
    if(!localNode) {
      localNode.reset(new DHTNode());
    }

    SharedHandle<DHTConnectionImpl> connection(new DHTConnectionImpl(family));
    {
      SegList<int> sgl;
      util::parseIntSegments(sgl, e->getOption()->get(PREF_DHT_LISTEN_PORT));
      sgl.normalize();
      uint16_t port;
      const std::string& addr =
        e->getOption()->get(family == AF_INET?PREF_DHT_LISTEN_ADDR:
                            PREF_DHT_LISTEN_ADDR6);
      if(!connection->bind(port, addr, sgl)) {
        throw DL_ABORT_EX("Error occurred while binding port for DHT");
      }
      localNode->setPort(port);
    }
    A2_LOG_DEBUG(fmt("Initialized local node ID=%s",
                     util::toHex(localNode->getID(), DHT_ID_LENGTH).c_str()));
    SharedHandle<DHTRoutingTable> routingTable(new DHTRoutingTable(localNode));

    SharedHandle<DHTMessageFactoryImpl> factory
      (new DHTMessageFactoryImpl(family));

    SharedHandle<DHTMessageTracker> tracker(new DHTMessageTracker());

    SharedHandle<DHTMessageDispatcherImpl> dispatcher(new DHTMessageDispatcherImpl(tracker));

    SharedHandle<DHTMessageReceiver> receiver(new DHTMessageReceiver(tracker));

    SharedHandle<DHTTaskQueue> taskQueue(new DHTTaskQueueImpl());

    SharedHandle<DHTTaskFactoryImpl> taskFactory(new DHTTaskFactoryImpl());

    SharedHandle<DHTPeerAnnounceStorage> peerAnnounceStorage(new DHTPeerAnnounceStorage());

    SharedHandle<DHTTokenTracker> tokenTracker(new DHTTokenTracker());

    const time_t messageTimeout = e->getOption()->getAsInt(PREF_DHT_MESSAGE_TIMEOUT);
    // wiring up
    tracker->setRoutingTable(routingTable);
    tracker->setMessageFactory(factory);

    dispatcher->setTimeout(messageTimeout);

    receiver->setConnection(connection);
    receiver->setMessageFactory(factory);
    receiver->setRoutingTable(routingTable);

    taskFactory->setLocalNode(localNode);
    taskFactory->setRoutingTable(routingTable.get());
    taskFactory->setMessageDispatcher(dispatcher.get());
    taskFactory->setMessageFactory(factory.get());
    taskFactory->setTaskQueue(taskQueue.get());
    taskFactory->setTimeout(messageTimeout);

    routingTable->setTaskQueue(taskQueue);
    routingTable->setTaskFactory(taskFactory);

    peerAnnounceStorage->setTaskQueue(taskQueue);
    peerAnnounceStorage->setTaskFactory(taskFactory);

    factory->setRoutingTable(routingTable.get());
    factory->setConnection(connection.get());
    factory->setMessageDispatcher(dispatcher.get());
    factory->setPeerAnnounceStorage(peerAnnounceStorage.get());
    factory->setTokenTracker(tokenTracker.get());
    factory->setLocalNode(localNode);

    // assign them into DHTRegistry
    if(family == AF_INET) {
      DHTRegistry::getMutableData().localNode = localNode;
      DHTRegistry::getMutableData().routingTable = routingTable;
      DHTRegistry::getMutableData().taskQueue = taskQueue;
      DHTRegistry::getMutableData().taskFactory = taskFactory;
      DHTRegistry::getMutableData().peerAnnounceStorage = peerAnnounceStorage;
      DHTRegistry::getMutableData().tokenTracker = tokenTracker;
      DHTRegistry::getMutableData().messageDispatcher = dispatcher;
      DHTRegistry::getMutableData().messageReceiver = receiver;
      DHTRegistry::getMutableData().messageFactory = factory;
    } else {
      DHTRegistry::getMutableData6().localNode = localNode;
      DHTRegistry::getMutableData6().routingTable = routingTable;
      DHTRegistry::getMutableData6().taskQueue = taskQueue;
      DHTRegistry::getMutableData6().taskFactory = taskFactory;
      DHTRegistry::getMutableData6().peerAnnounceStorage = peerAnnounceStorage;
      DHTRegistry::getMutableData6().tokenTracker = tokenTracker;
      DHTRegistry::getMutableData6().messageDispatcher = dispatcher;
      DHTRegistry::getMutableData6().messageReceiver = receiver;
      DHTRegistry::getMutableData6().messageFactory = factory;
    }
    // add deserialized nodes to routing table
    const std::vector<SharedHandle<DHTNode> >& desnodes =
      deserializer.getNodes();
    for(std::vector<SharedHandle<DHTNode> >::const_iterator i =
          desnodes.begin(), eoi = desnodes.end(); i != eoi; ++i) {
      routingTable->addNode(*i);
    }
    if(!desnodes.empty()) {
      SharedHandle<DHTBucketRefreshTask> task
        (static_pointer_cast<DHTBucketRefreshTask>
         (taskFactory->createBucketRefreshTask()));
      task->setForceRefresh(true);
      taskQueue->addPeriodicTask1(task);
    }

    const Pref* prefEntryPointHost =
      family == AF_INET?PREF_DHT_ENTRY_POINT_HOST:PREF_DHT_ENTRY_POINT_HOST6;
    if(!e->getOption()->get(prefEntryPointHost).empty()) {
      {
        const Pref* prefEntryPointPort =
          family == AF_INET?PREF_DHT_ENTRY_POINT_PORT:
          PREF_DHT_ENTRY_POINT_PORT6;
        std::pair<std::string, uint16_t> addr
          (e->getOption()->get(prefEntryPointHost),
           e->getOption()->getAsInt(prefEntryPointPort));
        std::vector<std::pair<std::string, uint16_t> > entryPoints;
        entryPoints.push_back(addr);
        DHTEntryPointNameResolveCommand* command =
          new DHTEntryPointNameResolveCommand(e->newCUID(), e, entryPoints);
        command->setBootstrapEnabled(true);
        command->setTaskQueue(taskQueue);
        command->setTaskFactory(taskFactory);
        command->setRoutingTable(routingTable);
        command->setLocalNode(localNode);
        tempCommands->push_back(command);
      }
    } else {
      A2_LOG_INFO("No DHT entry point specified.");
    }
    {
      DHTInteractionCommand* command =
        new DHTInteractionCommand(e->newCUID(), e);
      command->setMessageDispatcher(dispatcher);
      command->setMessageReceiver(receiver);
      command->setTaskQueue(taskQueue);
      command->setReadCheckSocket(connection->getSocket());
      tempCommands->push_back(command);
    }
    {
      DHTTokenUpdateCommand* command =
        new DHTTokenUpdateCommand(e->newCUID(), e, DHT_TOKEN_UPDATE_INTERVAL);
      command->setTokenTracker(tokenTracker);
      tempCommands->push_back(command);
    }
    {
      DHTBucketRefreshCommand* command =
        new DHTBucketRefreshCommand(e->newCUID(), e,
                                    DHT_BUCKET_REFRESH_CHECK_INTERVAL);
      command->setTaskQueue(taskQueue);
      command->setRoutingTable(routingTable);
      command->setTaskFactory(taskFactory);
      tempCommands->push_back(command);
    }
    {
      DHTPeerAnnounceCommand* command =
        new DHTPeerAnnounceCommand(e->newCUID(), e,
                                   DHT_PEER_ANNOUNCE_CHECK_INTERVAL);
      command->setPeerAnnounceStorage(peerAnnounceStorage);
      tempCommands->push_back(command);
    }
    {
      DHTAutoSaveCommand* command =
        new DHTAutoSaveCommand(e->newCUID(), e, family, 30*60);
      command->setLocalNode(localNode);
      command->setRoutingTable(routingTable);
      tempCommands->push_back(command);
    }
    if(family == AF_INET) {
      DHTRegistry::setInitialized(true);
    } else {
      DHTRegistry::setInitialized6(true);
    }
    commands.insert(commands.end(), tempCommands->begin(), tempCommands->end());
    tempCommands->clear();
  } catch(RecoverableException& e) {
    A2_LOG_ERROR_EX(fmt("Exception caught while initializing DHT functionality."
                        " DHT is disabled."),
                    e);
    if(family == AF_INET) {
      DHTRegistry::clearData();
    } else {
      DHTRegistry::clearData6();
    }
  }
}

} // namespace aria2
