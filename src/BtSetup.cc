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

namespace aria2 {

BtSetup::BtSetup():_logger(LogFactory::getInstance()) {}

Commands BtSetup::setup(RequestGroup* requestGroup,
			DownloadEngine* e,
			const Option* option)
{
  Commands commands;
  BtContextHandle btContext = requestGroup->getDownloadContext();
  if(btContext.isNull()) {
    return commands;
  }
  // commands
  commands.push_back(new TrackerWatcherCommand(CUIDCounterSingletonHolder::instance()->newID(),
					       requestGroup,
					       e,
					       btContext));
  commands.push_back(new PeerChokeCommand(CUIDCounterSingletonHolder::instance()->newID(),
					  requestGroup,
					  e,
					  btContext,
					  10));

  commands.push_back(new ActivePeerConnectionCommand(CUIDCounterSingletonHolder::instance()->newID(),
						     requestGroup, e, btContext, 10));

  if(!btContext->isPrivate() && DHTSetup::initialized()) {
    DHTRegistry::_peerAnnounceStorage->addPeerAnnounce(btContext);
    DHTGetPeersCommand* command = new DHTGetPeersCommand(CUIDCounterSingletonHolder::instance()->newID(),
							 requestGroup,
							 e,
							 btContext);
    command->setTaskQueue(DHTRegistry::_taskQueue);
    command->setTaskFactory(DHTRegistry::_taskFactory);
    commands.push_back(command);
  }
  SharedHandle<UnionSeedCriteria> unionCri = new UnionSeedCriteria();
  if(option->defined(PREF_SEED_TIME)) {
    unionCri->addSeedCriteria(new TimeSeedCriteria(option->getAsInt(PREF_SEED_TIME)*60));
  }
  {
    double ratio = option->getAsDouble(PREF_SEED_RATIO);
    if(ratio > 0.0) {
      unionCri->addSeedCriteria(new ShareRatioSeedCriteria(option->getAsDouble(PREF_SEED_RATIO), btContext));
    }
  }
  if(unionCri->getSeedCriterion().size() > 0) {
    commands.push_back(new SeedCheckCommand(CUIDCounterSingletonHolder::instance()->newID(),
					    requestGroup,
					    e,
					    btContext,
					    unionCri));
  }

  if(PeerListenCommand::getNumInstance() == 0) {
    PeerListenCommand* listenCommand = PeerListenCommand::getInstance(e);
    IntSequence seq = Util::parseIntRange(option->get(PREF_LISTEN_PORT));
    uint16_t port;
    if(listenCommand->bindPort(port, seq)) {
      BT_RUNTIME(btContext)->setListenPort(port);
      commands.push_back(listenCommand);
    } else {
      _logger->error(_("Errors occurred while binding port.\n"));
      delete listenCommand;
    }
  }

  BT_RUNTIME(btContext)->setReady(true);
  return commands;
}

} // namespace aria2
