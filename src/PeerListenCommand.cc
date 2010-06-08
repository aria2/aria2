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
#include "PeerListenCommand.h"

#include <utility>
#include <algorithm>

#include "DownloadEngine.h"
#include "Peer.h"
#include "RequestGroupMan.h"
#include "RecoverableException.h"
#include "message.h"
#include "ReceiverMSEHandshakeCommand.h"
#include "Logger.h"
#include "Socket.h"
#include "SimpleRandomizer.h"
#include "FileEntry.h"
#include "util.h"
#include "ServerStatMan.h"
#include "FileAllocationEntry.h"
#include "CheckIntegrityEntry.h"

namespace aria2 {

unsigned int PeerListenCommand::__numInstance = 0;

PeerListenCommand* PeerListenCommand::__instance = 0;

PeerListenCommand::PeerListenCommand(cuid_t cuid, DownloadEngine* e):
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

  std::vector<int32_t> randPorts = seq.flush();
  std::random_shuffle(randPorts.begin(), randPorts.end(),
                      *SimpleRandomizer::getInstance().get());
  
  for(std::vector<int32_t>::const_iterator portItr = randPorts.begin(),
        eoi = randPorts.end(); portItr != eoi; ++portItr) {
    if(!(0 < (*portItr) && (*portItr) <= 65535)) {
      continue;
    }
    port = (*portItr);
    try {
      socket->bind(port);
      socket->beginListen();
      socket->setNonBlockingMode();
      logger->notice("BitTorrent: listening to port %d", port);
      return true;
    } catch(RecoverableException& ex) {
      logger->error(MSG_BIND_FAILURE, ex, util::itos(cuid).c_str(), port);
      socket->closeConnection();
    }
  }
  return false;
}

uint16_t PeerListenCommand::getPort() const
{
  if(socket.isNull()) {
    return 0;
  } else {
    std::pair<std::string, uint16_t> addr;
    socket->getAddrInfo(addr);
    return addr.second;
  }
}

bool PeerListenCommand::execute() {
  if(e->isHaltRequested() || e->getRequestGroupMan()->downloadFinished()) {
    return true;
  }
  for(int i = 0; i < 3 && socket->isReadable(0); ++i) {
    SocketHandle peerSocket;
    try {
      peerSocket.reset(socket->acceptConnection());
      std::pair<std::string, uint16_t> peerInfo;
      peerSocket->getPeerInfo(peerInfo);

      peerSocket->setNonBlockingMode();

      SharedHandle<Peer> peer(new Peer(peerInfo.first, peerInfo.second, true));
      cuid_t cuid = e->newCUID();
      Command* command =
        new ReceiverMSEHandshakeCommand(cuid, peer, e, peerSocket);
      e->addCommand(command);
      if(logger->debug()) {
        logger->debug("Accepted the connection from %s:%u.",
                      peer->ipaddr.c_str(),
                      peer->port);
        logger->debug("Added CUID#%s to receive BitTorrent/MSE handshake.",
                      util::itos(cuid).c_str());
      }
    } catch(RecoverableException& ex) {
      logger->debug(MSG_ACCEPT_FAILURE, ex, util::itos(cuid).c_str());
    }               
  }
  e->addCommand(this);
  return false;
}

PeerListenCommand* PeerListenCommand::getInstance(DownloadEngine* e)
{
  if(__numInstance == 0) {
    __instance = new PeerListenCommand(e->newCUID(), e);
  }
  return __instance;
}

} // namespace aria2
