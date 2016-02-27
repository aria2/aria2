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

ActivePeerConnectionCommand::ActivePeerConnectionCommand(
    cuid_t cuid, RequestGroup* requestGroup, DownloadEngine* e,
    std::chrono::seconds interval)
    : Command(cuid),
      requestGroup_(requestGroup),
      interval_(std::move(interval)),
      e_(e),
      numNewConnection_(5)
{
  requestGroup_->increaseNumCommand();
}

ActivePeerConnectionCommand::~ActivePeerConnectionCommand()
{
  requestGroup_->decreaseNumCommand();
}

bool ActivePeerConnectionCommand::execute()
{
  if (btRuntime_->isHalt()) {
    return true;
  }
  if (checkPoint_.difference(global::wallclock()) >= interval_) {
    checkPoint_ = global::wallclock();
    NetStat& stat = requestGroup_->getDownloadContext()->getNetStat();
    const int maxDownloadLimit = requestGroup_->getMaxDownloadSpeedLimit();
    const int maxUploadLimit = requestGroup_->getMaxUploadSpeedLimit();
    int thresholdSpeed;
    if (!bittorrent::getTorrentAttrs(requestGroup_->getDownloadContext())
             ->metadata.empty()) {
      thresholdSpeed = requestGroup_->getOption()->getAsInt(
          PREF_BT_REQUEST_PEER_SPEED_LIMIT);
    }
    else {
      thresholdSpeed = 0;
    }
    if (maxDownloadLimit > 0) {
      thresholdSpeed = std::min(maxDownloadLimit, thresholdSpeed);
    }
    if ( // for seeder state
        (pieceStorage_->downloadFinished() && btRuntime_->lessThanMaxPeers() &&
         (maxUploadLimit == 0 ||
          stat.calculateUploadSpeed() < maxUploadLimit * 0.8)) ||
        // for leecher state
        (!pieceStorage_->downloadFinished() &&
         (stat.calculateDownloadSpeed() < thresholdSpeed ||
          btRuntime_->lessThanMinPeers()))) {

      int numConnection = 0;
      if (pieceStorage_->downloadFinished()) {
        if (btRuntime_->getMaxPeers() > btRuntime_->getConnections()) {
          numConnection =
              std::min(numNewConnection_, btRuntime_->getMaxPeers() -
                                              btRuntime_->getConnections());
        }
      }
      else {
        numConnection = numNewConnection_;
      }

      makeNewConnections(numConnection);

      if (btRuntime_->getConnections() == 0 &&
          !pieceStorage_->downloadFinished()) {
        btAnnounce_->overrideMinInterval(BtAnnounce::DEFAULT_ANNOUNCE_INTERVAL);
      }
    }
  }
  e_->addCommand(std::unique_ptr<Command>(this));
  return false;
}

void ActivePeerConnectionCommand::makeNewConnections(int num)
{
  for (; num && peerStorage_->isPeerAvailable(); --num) {
    cuid_t ncuid = e_->newCUID();
    std::shared_ptr<Peer> peer = peerStorage_->checkoutPeer(ncuid);
    // sanity check
    if (!peer) {
      break;
    }
    auto command = make_unique<PeerInitiateConnectionCommand>(
        ncuid, requestGroup_, peer, e_, btRuntime_);
    command->setPeerStorage(peerStorage_);
    command->setPieceStorage(pieceStorage_);
    e_->addCommand(std::move(command));
    A2_LOG_INFO(
        fmt(MSG_CONNECTING_TO_PEER, getCuid(), peer->getIPAddress().c_str()));
  }
}

void ActivePeerConnectionCommand::setBtRuntime(
    const std::shared_ptr<BtRuntime>& btRuntime)
{
  btRuntime_ = btRuntime;
}

void ActivePeerConnectionCommand::setPieceStorage(
    const std::shared_ptr<PieceStorage>& pieceStorage)
{
  pieceStorage_ = pieceStorage;
}

void ActivePeerConnectionCommand::setPeerStorage(
    const std::shared_ptr<PeerStorage>& peerStorage)
{
  peerStorage_ = peerStorage;
}

void ActivePeerConnectionCommand::setBtAnnounce(
    const std::shared_ptr<BtAnnounce>& btAnnounce)
{
  btAnnounce_ = btAnnounce;
}

} // namespace aria2
