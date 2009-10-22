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
#include "PeerReceiveHandshakeCommand.h"
#include "PeerConnection.h"
#include "DownloadEngine.h"
#include "BtHandshakeMessage.h"
#include "Util.h"
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

namespace aria2 {

PeerReceiveHandshakeCommand::PeerReceiveHandshakeCommand
(int32_t cuid,
 const PeerHandle& peer,
 DownloadEngine* e,
 const SocketHandle& s,
 const SharedHandle<PeerConnection>& peerConnection)
  :
  PeerAbstractCommand(cuid, peer, e, s),
  _peerConnection(peerConnection)
{
  if(_peerConnection.isNull()) {
    _peerConnection.reset(new PeerConnection(cuid, socket));
  }
}

PeerReceiveHandshakeCommand::~PeerReceiveHandshakeCommand() {}

bool PeerReceiveHandshakeCommand::exitBeforeExecute()
{
  return e->isHaltRequested() || e->_requestGroupMan->downloadFinished();
}

bool PeerReceiveHandshakeCommand::executeInternal()
{
  unsigned char data[BtHandshakeMessage::MESSAGE_LENGTH];
  size_t dataLength = BtHandshakeMessage::MESSAGE_LENGTH;
  // ignore return value. The received data is kept in PeerConnection object
  // because of peek = true.
  _peerConnection->receiveHandshake(data, dataLength, true);
  // To handle tracker's NAT-checking feature
  if(dataLength >= 48) {
    // check info_hash
    std::string infoHash = std::string(&data[28], &data[28+INFO_HASH_LENGTH]);

    BtObject btObject = e->getBtRegistry()->get(infoHash);
    SharedHandle<DownloadContext> downloadContext = btObject._downloadContext;
    SharedHandle<BtRuntime> btRuntime = btObject._btRuntime;
    SharedHandle<PieceStorage> pieceStorage = btObject._pieceStorage;
    SharedHandle<PeerStorage> peerStorage = btObject._peerStorage;

    if(downloadContext.isNull() || !btRuntime->ready()) {
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
      if(peerStorage->addPeer(peer)) {

	peer->usedBy(cuid);
	
	PeerInteractionCommand* command =
	  new PeerInteractionCommand
	  (cuid,
	   downloadContext->getOwnerRequestGroup(),
	   peer,
	   e,
	   btRuntime,
	   pieceStorage,
	   socket,
	   PeerInteractionCommand::RECEIVER_WAIT_HANDSHAKE,
	   _peerConnection);
	command->setPeerStorage(peerStorage);
	e->commands.push_back(command);
	logger->debug(MSG_INCOMING_PEER_CONNECTION, cuid, peer->usedBy());
      }
    }
    return true;
  } else {
    e->commands.push_back(this);
    return false;
  }
}

} // namespace aria2
