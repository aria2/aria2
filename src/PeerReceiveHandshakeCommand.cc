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
#include "PeerReceiveHandshakeCommand.h"

#include <cstring>

#include "PeerConnection.h"
#include "DownloadEngine.h"
#include "BtHandshakeMessage.h"
#include "util.h"
#include "DownloadContext.h"
#include "DlAbortEx.h"
#include "PeerInteractionCommand.h"
#include "Peer.h"
#include "BtRegistry.h"
#include "PeerStorage.h"
#include "PieceStorage.h"
#include "BtRuntime.h"
#include "BtConstants.h"
#include "message.h"
#include "SocketCore.h"
#include "Logger.h"
#include "LogFactory.h"
#include "prefs.h"
#include "Option.h"
#include "RequestGroupMan.h"
#include "fmt.h"
#include "RequestGroup.h"

namespace aria2 {

PeerReceiveHandshakeCommand::PeerReceiveHandshakeCommand(
    cuid_t cuid, const std::shared_ptr<Peer>& peer, DownloadEngine* e,
    const std::shared_ptr<SocketCore>& s,
    std::unique_ptr<PeerConnection> peerConnection)
    : PeerAbstractCommand{cuid, peer, e, s},
      peerConnection_{std::move(peerConnection)}
{
  if (peerConnection_) {
    if (peerConnection_->getBufferLength() > 0) {
      setStatus(Command::STATUS_ONESHOT_REALTIME);
      getDownloadEngine()->setNoWait(true);
    }
  }
  else {
    peerConnection_ = make_unique<PeerConnection>(cuid, getPeer(), getSocket());
  }
}

PeerReceiveHandshakeCommand::~PeerReceiveHandshakeCommand() = default;

bool PeerReceiveHandshakeCommand::exitBeforeExecute()
{
  return getDownloadEngine()->isHaltRequested() ||
         getDownloadEngine()->getRequestGroupMan()->downloadFinished();
}

bool PeerReceiveHandshakeCommand::executeInternal()
{
  // Handle tracker's NAT-checking feature
  if (peerConnection_->getBufferLength() < 48) {
    size_t dataLength = 0;
    // Ignore return value. The received data is kept in
    // PeerConnection object because of peek = true.
    peerConnection_->receiveHandshake(nullptr, dataLength, true);
  }
  if (peerConnection_->getBufferLength() >= 48) {
    const unsigned char* data = peerConnection_->getBuffer();
    // check info_hash
    std::string infoHash(&data[28], &data[28 + INFO_HASH_LENGTH]);

    std::shared_ptr<DownloadContext> downloadContext =
        getDownloadEngine()->getBtRegistry()->getDownloadContext(infoHash);
    if (!downloadContext) {
      throw DL_ABORT_EX(
          fmt("Unknown info hash %s", util::toHex(infoHash).c_str()));
    }
    auto btObject = getDownloadEngine()->getBtRegistry()->get(
        downloadContext->getOwnerRequestGroup()->getGID());
    const std::shared_ptr<BtRuntime>& btRuntime = btObject->btRuntime;
    const std::shared_ptr<PieceStorage>& pieceStorage = btObject->pieceStorage;
    const std::shared_ptr<PeerStorage>& peerStorage = btObject->peerStorage;
    if (!btRuntime->ready()) {
      throw DL_ABORT_EX(
          fmt("Unknown info hash %s", util::toHex(infoHash).c_str()));
    }
    if (btRuntime->isHalt()) {
      A2_LOG_DEBUG("Info hash found but the download is over."
                   " Dropping connection.");
      return true;
    }
    NetStat& stat = downloadContext->getNetStat();
    const int maxDownloadLimit =
        downloadContext->getOwnerRequestGroup()->getMaxDownloadSpeedLimit();
    int thresholdSpeed =
        downloadContext->getOwnerRequestGroup()->getOption()->getAsInt(
            PREF_BT_REQUEST_PEER_SPEED_LIMIT);
    if (maxDownloadLimit > 0) {
      thresholdSpeed = std::min(maxDownloadLimit, thresholdSpeed);
    }

    if ((!pieceStorage->downloadFinished() &&
         stat.calculateDownloadSpeed() < thresholdSpeed) ||
        btRuntime->lessThanMaxPeers()) {
      // TODO addPeer and checkoutPeer must be "atomic", in a sense
      // that the added peer must be checked out.
      if (peerStorage->addAndCheckoutPeer(getPeer(), getCuid())) {
        getDownloadEngine()->addCommand(make_unique<PeerInteractionCommand>(
            getCuid(), downloadContext->getOwnerRequestGroup(), getPeer(),
            getDownloadEngine(), btRuntime, pieceStorage, peerStorage,
            getSocket(), PeerInteractionCommand::RECEIVER_WAIT_HANDSHAKE,
            std::move(peerConnection_)));
        A2_LOG_DEBUG(
            fmt(MSG_INCOMING_PEER_CONNECTION, getCuid(), getPeer()->usedBy()));
      }
    }
    return true;
  }
  else {
    addCommandSelf();
    return false;
  }
}

} // namespace aria2
