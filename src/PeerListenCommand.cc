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

unsigned int PeerListenCommand::numInstance_ = 0;

PeerListenCommand* PeerListenCommand::instance_ = 0;

PeerListenCommand* PeerListenCommand::instance6_ = 0;

PeerListenCommand::PeerListenCommand
(cuid_t cuid, DownloadEngine* e, int family):
  Command(cuid),
  e_(e),
  family_(family),
  lowestSpeedLimit_(20*1024)
{
  ++numInstance_;
}

PeerListenCommand::~PeerListenCommand()
{
  --numInstance_;
}

bool PeerListenCommand::bindPort(uint16_t& port, IntSequence& seq)
{
  socket_.reset(new SocketCore());

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
      socket_->bind(A2STR::NIL, port, family_);
      socket_->beginListen();
      socket_->setNonBlockingMode();
      getLogger()->notice("IPv%d BitTorrent: listening to port %d",
                          family_ == AF_INET?4:6, port);
      return true;
    } catch(RecoverableException& ex) {
      getLogger()->error(MSG_BIND_FAILURE, ex,
                         util::itos(getCuid()).c_str(), port);
      socket_->closeConnection();
    }
  }
  return false;
}

uint16_t PeerListenCommand::getPort() const
{
  if(!socket_) {
    return 0;
  } else {
    std::pair<std::string, uint16_t> addr;
    socket_->getAddrInfo(addr);
    return addr.second;
  }
}

bool PeerListenCommand::execute() {
  if(e_->isHaltRequested() || e_->getRequestGroupMan()->downloadFinished()) {
    return true;
  }
  for(int i = 0; i < 3 && socket_->isReadable(0); ++i) {
    SocketHandle peerSocket;
    try {
      peerSocket.reset(socket_->acceptConnection());
      std::pair<std::string, uint16_t> peerInfo;
      peerSocket->getPeerInfo(peerInfo);

      peerSocket->setNonBlockingMode();

      SharedHandle<Peer> peer(new Peer(peerInfo.first, peerInfo.second, true));
      cuid_t cuid = e_->newCUID();
      Command* command =
        new ReceiverMSEHandshakeCommand(cuid, peer, e_, peerSocket);
      e_->addCommand(command);
      if(getLogger()->debug()) {
        getLogger()->debug("Accepted the connection from %s:%u.",
                           peer->getIPAddress().c_str(),
                           peer->getPort());
        getLogger()->debug("Added CUID#%s to receive BitTorrent/MSE handshake.",
                           util::itos(cuid).c_str());
      }
    } catch(RecoverableException& ex) {
      getLogger()->debug(MSG_ACCEPT_FAILURE, ex, util::itos(getCuid()).c_str());
    }               
  }
  e_->addCommand(this);
  return false;
}

PeerListenCommand* PeerListenCommand::getInstance(DownloadEngine* e, int family)
{
  if(family == AF_INET) {
    if(!instance_) {
      instance_ = new PeerListenCommand(e->newCUID(), e, family);
    }
    return instance_;
  } else if(family == AF_INET6) {
    if(!instance6_) {
      instance6_ = new PeerListenCommand(e->newCUID(), e, family);
    }
    return instance6_;
  } else {
    return 0;
  }
}

} // namespace aria2
