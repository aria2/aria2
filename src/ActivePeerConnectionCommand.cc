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
#include "ActivePeerConnectionCommand.h"
#include "PeerInitiateConnectionCommand.h"
#include "CUIDCounter.h"
#include "message.h"
#include "DownloadEngine.h"
#include "BtContext.h"
#include "PeerStorage.h"
#include "PieceStorage.h"
#include "BtRuntime.h"
#include "Peer.h"
#include "Logger.h"
#include "prefs.h"
#include "Option.h"
#include "BtConstants.h"
#include "SocketCore.h"

namespace aria2 {

ActivePeerConnectionCommand::ActivePeerConnectionCommand(int cuid,
							 RequestGroup* requestGroup,
							 DownloadEngine* e,
							 const BtContextHandle& btContext,
							 time_t interval)
  :Command(cuid),
   BtContextAwareCommand(btContext),
   RequestGroupAware(requestGroup),
   interval(interval),
   e(e),
   _thresholdSpeed(e->option->getAsInt(PREF_BT_REQUEST_PEER_SPEED_LIMIT)),
   _maxUploadSpeedLimit(e->option->getAsInt(PREF_MAX_UPLOAD_LIMIT)),
   _numNewConnection(5)
{
  unsigned int maxDownloadSpeed = e->option->getAsInt(PREF_MAX_DOWNLOAD_LIMIT);
  if(maxDownloadSpeed > 0) {
    _thresholdSpeed = std::min(maxDownloadSpeed, _thresholdSpeed);
  }
}

ActivePeerConnectionCommand::~ActivePeerConnectionCommand() {}

bool ActivePeerConnectionCommand::execute() {
  if(btRuntime->isHalt()) {
    return true;
  }
  if(checkPoint.elapsed(interval)) {
    checkPoint.reset();
    TransferStat tstat = peerStorage->calculateStat();
    if(// for seeder state
       (pieceStorage->downloadFinished() && btRuntime->lessThanMaxPeers() &&
	(_maxUploadSpeedLimit == 0 || tstat.getUploadSpeed() < _maxUploadSpeedLimit)) ||
       // for leecher state
       (tstat.getDownloadSpeed() < _thresholdSpeed ||
	btRuntime->lessThanMinPeers())) {
      for(size_t numAdd = _numNewConnection;
	  numAdd > 0 && peerStorage->isPeerAvailable(); --numAdd) {
	PeerHandle peer = peerStorage->getUnusedPeer();
	connectToPeer(peer);
      }
    }
  }
  e->commands.push_back(this);
  return false;
}

void ActivePeerConnectionCommand::connectToPeer(const PeerHandle& peer)
{
  if(peer.isNull()) {
    return;
  }
  peer->usedBy(CUIDCounterSingletonHolder::instance()->newID());
  PeerInitiateConnectionCommand* command =
    new PeerInitiateConnectionCommand(peer->usedBy(), _requestGroup, peer, e, btContext);
  e->commands.push_back(command);
  logger->info(MSG_CONNECTING_TO_PEER,
	       cuid, peer->ipaddr.c_str());
}

} // namespace aria2
