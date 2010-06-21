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
#include "BtAnnounce.h"
#include "BtProgressInfoFile.h"
#include "BtConstants.h"
#include "message.h"
#include "Socket.h"
#include "Logger.h"
#include "prefs.h"
#include "Option.h"
#include "RequestGroupMan.h"
#include "StringFormat.h"
#include "RequestGroup.h"
#include "ServerStatMan.h"
#include "FileAllocationEntry.h"
#include "CheckIntegrityEntry.h"

namespace aria2 {

PeerReceiveHandshakeCommand::PeerReceiveHandshakeCommand
(cuid_t cuid,
 const SharedHandle<Peer>& peer,
 DownloadEngine* e,
 const SocketHandle& s,
 const SharedHandle<PeerConnection>& peerConnection)
  :
  PeerAbstractCommand(cuid, peer, e, s),
  peerConnection_(peerConnection)
{
  if(peerConnection_.isNull()) {
    peerConnection_.reset(new PeerConnection(cuid, getSocket()));
  }
}

PeerReceiveHandshakeCommand::~PeerReceiveHandshakeCommand() {}

bool PeerReceiveHandshakeCommand::exitBeforeExecute()
{
  return getDownloadEngine()->isHaltRequested() ||
    getDownloadEngine()->getRequestGroupMan()->downloadFinished();
}

bool PeerReceiveHandshakeCommand::executeInternal()
{
  unsigned char data[BtHandshakeMessage::MESSAGE_LENGTH];
  size_t dataLength = BtHandshakeMessage::MESSAGE_LENGTH;
  // ignore return value. The received data is kept in PeerConnection object
  // because of peek = true.
  peerConnection_->receiveHandshake(data, dataLength, true);
  // To handle tracker's NAT-checking feature
  if(dataLength >= 48) {
    // check info_hash
    std::string infoHash = std::string(&data[28], &data[28+INFO_HASH_LENGTH]);

    SharedHandle<DownloadContext> downloadContext =
      getDownloadEngine()->getBtRegistry()->getDownloadContext(infoHash);
    if(downloadContext.isNull()) {
      throw DL_ABORT_EX
        (StringFormat("Unknown info hash %s",
                      util::toHex(infoHash).c_str()).str());
    }
    BtObject btObject = getDownloadEngine()->getBtRegistry()->get
      (downloadContext->getOwnerRequestGroup()->getGID());
    SharedHandle<BtRuntime> btRuntime = btObject.btRuntime_;
    SharedHandle<PieceStorage> pieceStorage = btObject.pieceStorage_;
    SharedHandle<PeerStorage> peerStorage = btObject.peerStorage_;
    if(!btRuntime->ready()) {
      throw DL_ABORT_EX
        (StringFormat("Unknown info hash %s",
                      util::toHex(infoHash).c_str()).str());
    }

    TransferStat tstat =
      downloadContext->getOwnerRequestGroup()->calculateStat();
    const unsigned int maxDownloadLimit =
      downloadContext->getOwnerRequestGroup()->getMaxDownloadSpeedLimit();
    unsigned int thresholdSpeed =
      downloadContext->getOwnerRequestGroup()->
      getOption()->getAsInt(PREF_BT_REQUEST_PEER_SPEED_LIMIT);
    if(maxDownloadLimit > 0) {
      thresholdSpeed = std::min(maxDownloadLimit, thresholdSpeed);
    }

    if((!pieceStorage->downloadFinished() &&
        tstat.getDownloadSpeed() < thresholdSpeed) ||
       btRuntime->lessThanMaxPeers()) {
      if(peerStorage->addPeer(getPeer())) {
        getPeer()->usedBy(getCuid());
        PeerInteractionCommand* command =
          new PeerInteractionCommand
          (getCuid(),
           downloadContext->getOwnerRequestGroup(),
           getPeer(),
           getDownloadEngine(),
           btRuntime,
           pieceStorage,
           peerStorage,
           getSocket(),
           PeerInteractionCommand::RECEIVER_WAIT_HANDSHAKE,
           peerConnection_);
        getDownloadEngine()->addCommand(command);
        if(getLogger()->debug()) {
          getLogger()->debug(MSG_INCOMING_PEER_CONNECTION,
                             util::itos(getCuid()).c_str(),
                             util::itos(getPeer()->usedBy()).c_str());
        }
      }
    }
    return true;
  } else {
    getDownloadEngine()->addCommand(this);
    return false;
  }
}

} // namespace aria2
