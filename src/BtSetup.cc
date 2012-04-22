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
#include "BtSetup.h"

#include <cstring>

#include "RequestGroup.h"
#include "DownloadEngine.h"
#include "Option.h"
#include "BtRegistry.h"
#include "PeerListenCommand.h"
#include "TrackerWatcherCommand.h"
#include "SeedCheckCommand.h"
#include "PeerChokeCommand.h"
#include "ActivePeerConnectionCommand.h"
#include "PeerListenCommand.h"
#include "UnionSeedCriteria.h"
#include "TimeSeedCriteria.h"
#include "ShareRatioSeedCriteria.h"
#include "prefs.h"
#include "LogFactory.h"
#include "Logger.h"
#include "util.h"
#include "SegList.h"
#include "DHTGetPeersCommand.h"
#include "DHTPeerAnnounceStorage.h"
#include "DHTSetup.h"
#include "DHTRegistry.h"
#include "DHTNode.h"
#include "DHTRoutingTable.h"
#include "DHTTaskQueue.h"
#include "DHTTaskFactory.h"
#include "DHTTokenTracker.h"
#include "DHTMessageDispatcher.h"
#include "DHTMessageReceiver.h"
#include "DHTMessageFactory.h"
#include "DHTMessageCallback.h"
#include "BtProgressInfoFile.h"
#include "BtAnnounce.h"
#include "BtRuntime.h"
#include "bittorrent_helper.h"
#include "BtStopDownloadCommand.h"
#include "LpdReceiveMessageCommand.h"
#include "LpdDispatchMessageCommand.h"
#include "LpdMessageReceiver.h"
#include "LpdMessageDispatcher.h"
#include "message.h"
#include "SocketCore.h"
#include "DlAbortEx.h"
#include "array_fun.h"
#include "DownloadContext.h"
#include "PieceStorage.h"
#include "PeerStorage.h"
#include "fmt.h"

namespace aria2 {

BtSetup::BtSetup() {}

void BtSetup::setup(std::vector<Command*>& commands,
                    RequestGroup* requestGroup,
                    DownloadEngine* e,
                    const Option* option)
{
  if(!requestGroup->getDownloadContext()->hasAttribute(bittorrent::BITTORRENT)){
    return;
  }
  SharedHandle<TorrentAttribute> torrentAttrs =
    bittorrent::getTorrentAttrs(requestGroup->getDownloadContext());
  bool metadataGetMode = torrentAttrs->metadata.empty();
  const SharedHandle<BtRegistry>& btReg = e->getBtRegistry();
  const SharedHandle<BtObject>& btObject = btReg->get(requestGroup->getGID());
  const SharedHandle<PieceStorage>& pieceStorage = btObject->pieceStorage;
  const SharedHandle<PeerStorage>& peerStorage = btObject->peerStorage;
  const SharedHandle<BtRuntime>& btRuntime = btObject->btRuntime;
  const SharedHandle<BtAnnounce>& btAnnounce = btObject->btAnnounce;
  // commands
  {
    TrackerWatcherCommand* c =
      new TrackerWatcherCommand(e->newCUID(), requestGroup, e);
    c->setPeerStorage(peerStorage);
    c->setPieceStorage(pieceStorage);
    c->setBtRuntime(btRuntime);
    c->setBtAnnounce(btAnnounce);
    
    commands.push_back(c);
  }
  if(!metadataGetMode) {
    PeerChokeCommand* c =
      new PeerChokeCommand(e->newCUID(), e);
    c->setPeerStorage(peerStorage);
    c->setBtRuntime(btRuntime);
    
    commands.push_back(c);
  }
  {
    ActivePeerConnectionCommand* c =
      new ActivePeerConnectionCommand(e->newCUID(), requestGroup, e,
                                      metadataGetMode?2:10);
    c->setBtRuntime(btRuntime);
    c->setPieceStorage(pieceStorage);
    c->setPeerStorage(peerStorage);
    c->setBtAnnounce(btAnnounce);
            
    commands.push_back(c);
  }

  if(metadataGetMode || !torrentAttrs->privateTorrent) {
    if(DHTRegistry::isInitialized()) {
      DHTGetPeersCommand* command =
        new DHTGetPeersCommand(e->newCUID(), requestGroup, e);
      command->setTaskQueue(DHTRegistry::getData().taskQueue);
      command->setTaskFactory(DHTRegistry::getData().taskFactory);
      command->setBtRuntime(btRuntime);
      command->setPeerStorage(peerStorage);
      commands.push_back(command);
    }
    if(DHTRegistry::isInitialized6()) {
      DHTGetPeersCommand* command =
        new DHTGetPeersCommand(e->newCUID(), requestGroup, e);
      command->setTaskQueue(DHTRegistry::getData6().taskQueue);
      command->setTaskFactory(DHTRegistry::getData6().taskFactory);
      command->setBtRuntime(btRuntime);
      command->setPeerStorage(peerStorage);
      commands.push_back(command);
    }
  }
  if(!metadataGetMode) {
    SharedHandle<UnionSeedCriteria> unionCri(new UnionSeedCriteria());
    if(option->defined(PREF_SEED_TIME)) {
      SharedHandle<SeedCriteria> cri
        (new TimeSeedCriteria(option->getAsInt(PREF_SEED_TIME)*60));
      unionCri->addSeedCriteria(cri);
    }
    {
      double ratio = option->getAsDouble(PREF_SEED_RATIO);
      if(ratio > 0.0) {
        SharedHandle<ShareRatioSeedCriteria> cri
          (new ShareRatioSeedCriteria(option->getAsDouble(PREF_SEED_RATIO),
                                      requestGroup->getDownloadContext()));
        cri->setPieceStorage(pieceStorage);
        cri->setPeerStorage(peerStorage);

        unionCri->addSeedCriteria(cri);
      }
    }
    if(!unionCri->getSeedCriterion().empty()) {
      SeedCheckCommand* c =
        new SeedCheckCommand(e->newCUID(), requestGroup, e, unionCri);
      c->setPieceStorage(pieceStorage);
      c->setBtRuntime(btRuntime);
      commands.push_back(c);
    }
  }
  if(btReg->getTcpPort() == 0) {
    static int families[] = { AF_INET, AF_INET6 };
    size_t familiesLength = e->getOption()->getAsBool(PREF_DISABLE_IPV6)?1:2;
    for(size_t i = 0; i < familiesLength; ++i) {
      PeerListenCommand* command =
        new PeerListenCommand(e->newCUID(), e, families[i]);
      bool ret;
      uint16_t port;
      if(btReg->getTcpPort()) {
        SegList<int> sgl;
        int usedPort = btReg->getTcpPort();
        sgl.add(usedPort, usedPort+1);
        ret = command->bindPort(port, sgl);
      } else {
        SegList<int> sgl;
        util::parseIntSegments(sgl, e->getOption()->get(PREF_LISTEN_PORT));
        sgl.normalize();
        ret = command->bindPort(port, sgl);
      }
      if(ret) {
        btReg->setTcpPort(port);
        // Add command to DownloadEngine directly.
        e->addCommand(command);
      } else {
        delete command;
      }
    }
    if(btReg->getTcpPort() == 0) {
      throw DL_ABORT_EX(_("Errors occurred while binding port.\n"));
    }
  }
  btAnnounce->setTcpPort(btReg->getTcpPort());

  if(option->getAsBool(PREF_BT_ENABLE_LPD) &&
     btReg->getTcpPort() &&
     (metadataGetMode || !torrentAttrs->privateTorrent)) {
    if(!btReg->getLpdMessageReceiver()) {
      A2_LOG_INFO("Initializing LpdMessageReceiver.");
      SharedHandle<LpdMessageReceiver> receiver
        (new LpdMessageReceiver(LPD_MULTICAST_ADDR, LPD_MULTICAST_PORT));
      bool initialized = false;
      const std::string& lpdInterface =
        e->getOption()->get(PREF_BT_LPD_INTERFACE);
      if(lpdInterface.empty()) {
        if(receiver->init("")) {
          initialized = true;
        }
      } else {
        std::vector<std::pair<sockaddr_union, socklen_t> > ifAddrs;
        getInterfaceAddress(ifAddrs, lpdInterface, AF_INET, AI_NUMERICHOST);
        for(std::vector<std::pair<sockaddr_union, socklen_t> >::const_iterator
              i = ifAddrs.begin(), eoi = ifAddrs.end(); i != eoi; ++i) {
          char host[NI_MAXHOST];
          if(inetNtop(AF_INET, &(*i).first.in.sin_addr, host,
                      sizeof(host)) == 0 &&
             receiver->init(host)) {
            initialized = true;
            break;
          }
        }
      }
      if(initialized) {
        btReg->setLpdMessageReceiver(receiver);
        A2_LOG_INFO(fmt("LpdMessageReceiver initialized. multicastAddr=%s:%u,"
                        " localAddr=%s",
                        LPD_MULTICAST_ADDR, LPD_MULTICAST_PORT,
                        receiver->getLocalAddress().c_str()));
        LpdReceiveMessageCommand* cmd =
          new LpdReceiveMessageCommand(e->newCUID(), receiver, e);
        e->addCommand(cmd);
      } else {
        A2_LOG_INFO("LpdMessageReceiver not initialized.");
      }
    }
    if(btReg->getLpdMessageReceiver()) {
      const unsigned char* infoHash =
        bittorrent::getInfoHash(requestGroup->getDownloadContext());
      A2_LOG_INFO("Initializing LpdMessageDispatcher.");      
      SharedHandle<LpdMessageDispatcher> dispatcher
        (new LpdMessageDispatcher
         (std::string(&infoHash[0], &infoHash[INFO_HASH_LENGTH]),
          btReg->getTcpPort(),
          LPD_MULTICAST_ADDR, LPD_MULTICAST_PORT));
      if(dispatcher->init(btReg->getLpdMessageReceiver()->getLocalAddress(),
                          /*ttl*/1, /*loop*/0)) {
        A2_LOG_INFO("LpdMessageDispatcher initialized.");
        LpdDispatchMessageCommand* cmd =
          new LpdDispatchMessageCommand(e->newCUID(), dispatcher, e);
        cmd->setBtRuntime(btRuntime);
        e->addCommand(cmd);
      } else {
        A2_LOG_INFO("LpdMessageDispatcher not initialized.");
      }
    }
  }
  time_t btStopTimeout = option->getAsInt(PREF_BT_STOP_TIMEOUT);
  if(btStopTimeout > 0) {
    BtStopDownloadCommand* stopDownloadCommand =
      new BtStopDownloadCommand(e->newCUID(), requestGroup, e, btStopTimeout);
    stopDownloadCommand->setBtRuntime(btRuntime);
    stopDownloadCommand->setPieceStorage(pieceStorage);
    commands.push_back(stopDownloadCommand);
  }
  btRuntime->setReady(true);
}

} // namespace aria2
