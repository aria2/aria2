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
#include "BtContext.h"
#include "DlAbortEx.h"
#include "PeerInteractionCommand.h"
#include "Peer.h"
#include "BtRegistry.h"
#include "PeerStorage.h"
#include "PieceStorage.h"
#include "BtRuntime.h"
#include "BtConstants.h"
#include "message.h"
#include "Socket.h"
#include "Logger.h"

namespace aria2 {

PeerReceiveHandshakeCommand::PeerReceiveHandshakeCommand(int32_t cuid,
							 const PeerHandle& peer,
							 DownloadEngine* e,
							 const SocketHandle& s):
  PeerAbstractCommand(cuid, peer, e, s),
  _peerConnection(new PeerConnection(cuid, s, e->option)),
  _lowestSpeedLimit(20*1024)
{}

PeerReceiveHandshakeCommand::~PeerReceiveHandshakeCommand() {}

bool PeerReceiveHandshakeCommand::exitBeforeExecute()
{
  return e->isHaltRequested();
}

bool PeerReceiveHandshakeCommand::executeInternal()
{
  unsigned char data[BtHandshakeMessage::MESSAGE_LENGTH];
  int32_t dataLength = BtHandshakeMessage::MESSAGE_LENGTH;
  // ignore return value. The received data is kept in PeerConnection object
  // because of peek = true.
  _peerConnection->receiveHandshake(data, dataLength, true);
  // To handle tracker's NAT-checking feature
  if(dataLength >= 48) {
    // check info_hash
    std::string infoHash = Util::toHex(&data[28], INFO_HASH_LENGTH);
    BtContextHandle btContext = BtRegistry::getBtContext(infoHash);
    if(btContext.isNull() || !BT_RUNTIME(btContext)->ready()) {
      throw new DlAbortEx("Unknown info hash %s", infoHash.c_str());
    }
    TransferStat tstat = PEER_STORAGE(btContext)->calculateStat();
    if(!PIECE_STORAGE(btContext)->downloadFinished() && tstat.getDownloadSpeed() < _lowestSpeedLimit ||
       BT_RUNTIME(btContext)->getConnections() < MAX_PEERS) {
      if(PEER_STORAGE(btContext)->addPeer(peer)) {

	peer->usedBy(cuid);
	
	PeerInteractionCommand* command =
	  new PeerInteractionCommand(cuid,
				     btContext->getOwnerRequestGroup(),
				     peer,
				     e,
				     btContext,
				     socket,
				     PeerInteractionCommand::RECEIVER_WAIT_HANDSHAKE,
				     _peerConnection);
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
