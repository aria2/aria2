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
#include "ActivePeerConnectionCommand.h"
#include "PeerInitiateConnectionCommand.h"
#include "message.h"
#include "DownloadEngine.h"
#include "PeerStorage.h"
#include "PieceStorage.h"
#include "BtRuntime.h"
#include "Peer.h"
#include "Logger.h"
#include "LogFactory.h"
#include "prefs.h"
#include "Option.h"
#include "BtConstants.h"
#include "SocketCore.h"
#include "BtAnnounce.h"
#include "RequestGroup.h"
#include "DownloadContext.h"
#include "bittorrent_helper.h"
#include "wallclock.h"
#include "util.h"
#include "fmt.h"

namespace aria2 {

ActivePeerConnectionCommand::ActivePeerConnectionCommand
(cuid_t cuid,
 RequestGroup* requestGroup,
 DownloadEngine* e,
 time_t interval)
  : Command(cuid),
    requestGroup_(requestGroup),
    interval_(interval),
    e_(e),
    numNewConnection_(5)
{
  requestGroup_->increaseNumCommand();
}

ActivePeerConnectionCommand::~ActivePeerConnectionCommand()
{
  requestGroup_->decreaseNumCommand();
}

bool ActivePeerConnectionCommand::execute() {
  if(btRuntime_->isHalt()) {
    return true;
  }
  if(checkPoint_.difference(global::wallclock()) >= interval_) {
    checkPoint_ = global::wallclock();
    TransferStat tstat = requestGroup_->calculateStat();
    const int maxDownloadLimit = requestGroup_->getMaxDownloadSpeedLimit();
    const int maxUploadLimit = requestGroup_->getMaxUploadSpeedLimit();
    int thresholdSpeed;
    if(!bittorrent::getTorrentAttrs
       (requestGroup_->getDownloadContext())->metadata.empty()) {
      thresholdSpeed =
        requestGroup_->getOption()->getAsInt(PREF_BT_REQUEST_PEER_SPEED_LIMIT);
    } else {
      thresholdSpeed = 0;
    }
    if(maxDownloadLimit > 0) {
      thresholdSpeed = std::min(maxDownloadLimit, thresholdSpeed);
    }
    if(// for seeder state
       (pieceStorage_->downloadFinished() && btRuntime_->lessThanMaxPeers() &&
        (maxUploadLimit == 0 || tstat.getUploadSpeed() < maxUploadLimit*0.8)) ||
       // for leecher state
       (!pieceStorage_->downloadFinished() &&
        (tstat.getDownloadSpeed() < thresholdSpeed ||
         btRuntime_->lessThanMinPeers()))) {

      int numConnection = 0;
      if(pieceStorage_->downloadFinished()) {
        if(btRuntime_->getMaxPeers() > btRuntime_->getConnections()) {
          numConnection =
            std::min(numNewConnection_,
                     btRuntime_->getMaxPeers()-btRuntime_->getConnections());
        }
      } else {
        numConnection = numNewConnection_;
      }

      for(int numAdd = numConnection;
          numAdd > 0 && peerStorage_->isPeerAvailable(); --numAdd) {
        SharedHandle<Peer> peer = peerStorage_->getUnusedPeer();
        connectToPeer(peer);
      }
      if(btRuntime_->getConnections() == 0 &&
         !pieceStorage_->downloadFinished()) {
        btAnnounce_->overrideMinInterval(BtAnnounce::DEFAULT_ANNOUNCE_INTERVAL);
      }
    }
  }
  e_->addCommand(this);
  return false;
}

void ActivePeerConnectionCommand::connectToPeer(const SharedHandle<Peer>& peer)
{
  if(!peer) {
    return;
  }
  peer->usedBy(e_->newCUID());
  PeerInitiateConnectionCommand* command =
    new PeerInitiateConnectionCommand(peer->usedBy(), requestGroup_, peer, e_,
                                      btRuntime_);
  command->setPeerStorage(peerStorage_);
  command->setPieceStorage(pieceStorage_);
  e_->addCommand(command);
  A2_LOG_INFO(fmt(MSG_CONNECTING_TO_PEER,
                  getCuid(),
                  peer->getIPAddress().c_str()));
}

void ActivePeerConnectionCommand::setBtRuntime
(const SharedHandle<BtRuntime>& btRuntime)
{
  btRuntime_ = btRuntime;
}

void ActivePeerConnectionCommand::setPieceStorage
(const SharedHandle<PieceStorage>& pieceStorage)
{
  pieceStorage_ = pieceStorage;
}

void ActivePeerConnectionCommand::setPeerStorage
(const SharedHandle<PeerStorage>& peerStorage)
{
  peerStorage_ = peerStorage;
}

void ActivePeerConnectionCommand::setBtAnnounce
(const SharedHandle<BtAnnounce>& btAnnounce)
{
  btAnnounce_ = btAnnounce;
}

} // namespace aria2
