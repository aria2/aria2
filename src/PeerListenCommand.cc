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
#include "DownloadEngine.h"
#include "Peer.h"
#include "RequestGroupMan.h"
#include "RecoverableException.h"
#include "CUIDCounter.h"
#include "message.h"
#include "ReceiverMSEHandshakeCommand.h"
#include "Logger.h"
#include "Socket.h"
#include <utility>

namespace aria2 {

unsigned int PeerListenCommand::__numInstance = 0;

PeerListenCommand* PeerListenCommand::__instance = 0;

PeerListenCommand::PeerListenCommand(int32_t cuid, DownloadEngine* e):
  Command(cuid),
  e(e),
  _lowestSpeedLimit(20*1024)
{
  ++__numInstance;
}

PeerListenCommand::~PeerListenCommand()
{
  --__numInstance;
}

bool PeerListenCommand::bindPort(uint16_t& port, IntSequence& seq)
{
  socket.reset(new SocketCore());
  while(seq.hasNext()) {
    int sport = seq.next();
    if(!(0 < sport && sport <= UINT16_MAX)) {
      continue;
    }
    port = sport;
    try {
      socket->bind(port);
      socket->beginListen();
      socket->setNonBlockingMode();
      logger->info(MSG_LISTENING_PORT, cuid, port);
      return true;
    } catch(RecoverableException* ex) {
      logger->error(MSG_BIND_FAILURE, ex, cuid, port);
      socket->closeConnection();
      delete ex;
    }
  }
  return false;
}

bool PeerListenCommand::execute() {
  if(e->isHaltRequested() || e->_requestGroupMan->downloadFinished()) {
    return true;
  }
  for(int i = 0; i < 3 && socket->isReadable(0); i++) {
    SocketHandle peerSocket;
    try {
      peerSocket.reset(socket->acceptConnection());
      std::pair<std::string, uint16_t> peerInfo;
      peerSocket->getPeerInfo(peerInfo);
      std::pair<std::string, uint16_t> localInfo;
      peerSocket->getAddrInfo(localInfo);

      if(peerInfo.first == localInfo.first) {
	continue;
      }
      // Since peerSocket may be in non-blocking mode, make it blocking mode
      // here.
      peerSocket->setBlockingMode();

      PeerHandle peer(new Peer(peerInfo.first, 0));
      int32_t cuid = CUIDCounterSingletonHolder::instance()->newID();
      Command* command =
	new ReceiverMSEHandshakeCommand(cuid, peer, e, peerSocket);
      e->commands.push_back(command);
      logger->debug("Accepted the connection from %s:%u.",
		    peer->ipaddr.c_str(),
		    peer->port);
      logger->debug("Added CUID#%d to receive BitTorrent/MSE handshake.", cuid);
    } catch(RecoverableException* ex) {
      logger->debug(MSG_ACCEPT_FAILURE, ex, cuid);
      delete ex;
    }		    
  }
  e->commands.push_back(this);
  return false;
}

PeerListenCommand* PeerListenCommand::getInstance(DownloadEngine* e)
{
  if(__numInstance == 0) {
    __instance = new PeerListenCommand(CUIDCounterSingletonHolder::instance()->newID(), e);
  }
  return __instance;
}

} // namespace aria2
