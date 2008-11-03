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
#include "BtSetup.h"
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
#include "CUIDCounter.h"
#include "prefs.h"
#include "LogFactory.h"
#include "Logger.h"
#include "Util.h"
#include "IntSequence.h"
#include "DHTGetPeersCommand.h"
#include "DHTPeerAnnounceStorage.h"
#include "DHTSetup.h"
#include "DHTRegistry.h"
#include "BtProgressInfoFile.h"
#include "BtAnnounce.h"

namespace aria2 {

BtSetup::BtSetup():_logger(LogFactory::getInstance()) {}

void BtSetup::setup(std::deque<Command*>& commands,
		    RequestGroup* requestGroup,
		    DownloadEngine* e,
		    const Option* option)
{
  BtContextHandle btContext(dynamic_pointer_cast<BtContext>(requestGroup->getDownloadContext()));
  if(btContext.isNull()) {
    return;
  }
  SharedHandle<BtRegistry> btRegistry = e->getBtRegistry();
  SharedHandle<PieceStorage> pieceStorage =
    btRegistry->getPieceStorage(btContext->getInfoHashAsString());
  SharedHandle<PeerStorage> peerStorage =
    btRegistry->getPeerStorage(btContext->getInfoHashAsString());
  SharedHandle<BtRuntime> btRuntime =
    btRegistry->getBtRuntime(btContext->getInfoHashAsString());
  SharedHandle<BtAnnounce> btAnnounce =
    btRegistry->getBtAnnounce(btContext->getInfoHashAsString());

  // commands
  {
    TrackerWatcherCommand* c =
      new TrackerWatcherCommand(CUIDCounterSingletonHolder::instance()->newID(),
				requestGroup,
				e,
				btContext);
    c->setPeerStorage(peerStorage);
    c->setPieceStorage(pieceStorage);
    c->setBtRuntime(btRuntime);
    c->setBtAnnounce(btAnnounce);
    
    commands.push_back(c);
  }
  {
    PeerChokeCommand* c =
      new PeerChokeCommand(CUIDCounterSingletonHolder::instance()->newID(),
			   e,
			   btContext);
    c->setPeerStorage(peerStorage);
    c->setBtRuntime(btRuntime);

    commands.push_back(c);
  }
  {
    ActivePeerConnectionCommand* c =
      new ActivePeerConnectionCommand(CUIDCounterSingletonHolder::instance()->newID(),
				      requestGroup, e, btContext, 10);
    c->setBtRuntime(btRuntime);
    c->setPieceStorage(pieceStorage);
    c->setPeerStorage(peerStorage);
    c->setBtAnnounce(btAnnounce);
	    
    commands.push_back(c);
  }

  if(!btContext->isPrivate() && DHTSetup::initialized()) {
    DHTRegistry::_peerAnnounceStorage->addPeerAnnounce(btContext->getInfoHash(),
						       peerStorage);
    DHTGetPeersCommand* command =
      new DHTGetPeersCommand(CUIDCounterSingletonHolder::instance()->newID(),
			     requestGroup,
			     e,
			     btContext);
    command->setTaskQueue(DHTRegistry::_taskQueue);
    command->setTaskFactory(DHTRegistry::_taskFactory);
    command->setBtRuntime(btRuntime);
    command->setPeerStorage(peerStorage);
    commands.push_back(command);
  }
  SharedHandle<UnionSeedCriteria> unionCri(new UnionSeedCriteria());
  if(option->defined(PREF_SEED_TIME)) {
    SharedHandle<SeedCriteria> cri(new TimeSeedCriteria(option->getAsInt(PREF_SEED_TIME)*60));
    unionCri->addSeedCriteria(cri);
  }
  {
    double ratio = option->getAsDouble(PREF_SEED_RATIO);
    if(ratio > 0.0) {
      SharedHandle<ShareRatioSeedCriteria> cri
	(new ShareRatioSeedCriteria(option->getAsDouble(PREF_SEED_RATIO),
				    btContext));
      cri->setPieceStorage(pieceStorage);
      cri->setPeerStorage(peerStorage);
      cri->setBtRuntime(btRuntime);

      unionCri->addSeedCriteria(cri);
    }
  }
  if(unionCri->getSeedCriterion().size() > 0) {
    SeedCheckCommand* c =
      new SeedCheckCommand(CUIDCounterSingletonHolder::instance()->newID(),
			   requestGroup,
			   e,
			   btContext,
			   unionCri);
    c->setPieceStorage(pieceStorage);
    c->setBtRuntime(btRuntime);
    commands.push_back(c);
  }

  if(PeerListenCommand::getNumInstance() == 0) {
    PeerListenCommand* listenCommand = PeerListenCommand::getInstance(e);
    IntSequence seq = Util::parseIntRange(option->get(PREF_LISTEN_PORT));
    uint16_t port;
    if(listenCommand->bindPort(port, seq)) {
      btRuntime->setListenPort(port);
      commands.push_back(listenCommand);
    } else {
      _logger->error(_("Errors occurred while binding port.\n"));
      delete listenCommand;
    }
  }

  btRuntime->setReady(true);
}

} // namespace aria2
