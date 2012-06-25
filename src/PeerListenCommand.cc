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
#include "LogFactory.h"
#include "Socket.h"
#include "SimpleRandomizer.h"
#include "util.h"
#include "fmt.h"

namespace aria2 {

PeerListenCommand::PeerListenCommand
(cuid_t cuid,
 DownloadEngine* e,
 int family)
  : Command(cuid),
    e_(e),
    family_(family)
{}

PeerListenCommand::~PeerListenCommand() {}

bool PeerListenCommand::bindPort(uint16_t& port, SegList<int>& sgl)
{
  socket_.reset(new SocketCore());
  std::vector<uint16_t> ports;
  while(sgl.hasNext()) {
    ports.push_back(sgl.next());
  }
  std::random_shuffle(ports.begin(), ports.end(),
                      *SimpleRandomizer::getInstance().get());
  const int ipv = (family_ == AF_INET) ? 4 : 6;
  for(std::vector<uint16_t>::const_iterator i = ports.begin(),
        eoi = ports.end(); i != eoi; ++i) {
    port = *i;
    try {
      socket_->bind(A2STR::NIL, port, family_);
      socket_->beginListen();
      socket_->setNonBlockingMode();
      A2_LOG_NOTICE(fmt(_("IPv%d BitTorrent: listening to port %u"),
                        ipv, port));
      return true;
    } catch(RecoverableException& ex) {
      A2_LOG_ERROR_EX(fmt("IPv%d BitTorrent: failed to bind port %u",
                          ipv, port), ex);
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
      A2_LOG_DEBUG(fmt("Accepted the connection from %s:%u.",
                       peer->getIPAddress().c_str(),
                       peer->getPort()));
      A2_LOG_DEBUG(fmt("Added CUID#%" PRId64 " to receive BitTorrent/MSE handshake.",
                       cuid));
    } catch(RecoverableException& ex) {
      A2_LOG_DEBUG_EX(fmt(MSG_ACCEPT_FAILURE,
                          getCuid()),
                      ex);
    }
  }
  e_->addCommand(this);
  return false;
}

} // namespace aria2
