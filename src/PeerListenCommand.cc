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
#include "PeerListenCommand.h"
#include "PeerInteractionCommand.h"
#include "RecoverableException.h"
#include "CUIDCounter.h"
#include "message.h"

PeerListenCommand::PeerListenCommand(int32_t cuid,
				     TorrentDownloadEngine* e,
				     const BtContextHandle& btContext)
  :BtContextAwareCommand(cuid, btContext),
   e(e),
   _lowestSpeedLimit(20*1024) {}

PeerListenCommand::~PeerListenCommand() {}

int32_t PeerListenCommand::bindPort(int32_t portRangeStart, int32_t portRangeEnd) {
  if(portRangeStart > portRangeEnd) {
    return -1;
  }
  for(int32_t port = portRangeStart; port <= portRangeEnd; port++) {
    try {
      socket->beginListen(port);
      logger->info(MSG_LISTENING_PORT,
		   cuid, port);
      return port;
    } catch(RecoverableException* ex) {
      logger->error(MSG_BIND_FAILURE,
		    ex, cuid, port);
      socket->closeConnection();
      delete ex;
    }
  }
  return -1;
}

bool PeerListenCommand::execute() {
  if(btRuntime->isHalt()) {
    return true;
  }
  for(int32_t i = 0; i < 3 && socket->isReadable(0); i++) {
    SocketHandle peerSocket;
    try {
      peerSocket = socket->acceptConnection();
      pair<string, int32_t> peerInfo;
      peerSocket->getPeerInfo(peerInfo);
      pair<string, int32_t> localInfo;
      peerSocket->getAddrInfo(localInfo);

      TransferStat tstat = peerStorage->calculateStat();
      if(peerInfo.first != localInfo.first &&
	 (!pieceStorage->downloadFinished() && tstat.getDownloadSpeed() < _lowestSpeedLimit ||
	  btRuntime->getConnections() < MAX_PEERS)) {
	PeerHandle peer = PeerHandle(new Peer(peerInfo.first, peerInfo.second,
					      btContext->getPieceLength(),
					      btContext->getTotalLength()));
	if(peerStorage->addIncomingPeer(peer)) {
	  peer->cuid = CUIDCounterSingletonHolder::instance()->newID();
	  PeerInteractionCommand* command =
	    new PeerInteractionCommand(peer->cuid, peer, e,
				       btContext,
				       peerSocket,
				       PeerInteractionCommand::RECEIVER_WAIT_HANDSHAKE);
	  e->commands.push_back(command);
	  logger->debug(MSG_INCOMING_PEER_CONNECTION, cuid, peer->cuid);
	}
      }
    } catch(RecoverableException* ex) {
      logger->debug(MSG_ACCEPT_FAILURE, ex, cuid);
      delete ex;
    }		    
  }
  e->commands.push_back(this);
  return false;
}
