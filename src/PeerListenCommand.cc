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
#include "SocketCore.h"
#include "SimpleRandomizer.h"
#include "util.h"
#include "fmt.h"

namespace aria2 {

PeerListenCommand::PeerListenCommand(cuid_t cuid, DownloadEngine* e, int family)
    : Command(cuid), e_(e), family_(family)
{
}

PeerListenCommand::~PeerListenCommand() = default;

bool PeerListenCommand::bindPort(uint16_t& port, SegList<int>& sgl)
{
  socket_ = std::make_shared<SocketCore>();
  std::vector<uint16_t> ports;
  while (sgl.hasNext()) {
    ports.push_back(sgl.next());
  }
  std::shuffle(ports.begin(), ports.end(), *SimpleRandomizer::getInstance());
  const int ipv = (family_ == AF_INET) ? 4 : 6;
  for (std::vector<uint16_t>::const_iterator i = ports.begin(),
                                             eoi = ports.end();
       i != eoi; ++i) {
    port = *i;
    try {
      socket_->bind(nullptr, port, family_);
      socket_->beginListen();
      A2_LOG_NOTICE(
          fmt(_("IPv%d BitTorrent: listening on TCP port %u"), ipv, port));
      return true;
    }
    catch (RecoverableException& ex) {
      A2_LOG_ERROR_EX(
          fmt("IPv%d BitTorrent: failed to bind TCP port %u", ipv, port), ex);
      socket_->closeConnection();
    }
  }
  return false;
}

uint16_t PeerListenCommand::getPort() const
{
  if (!socket_) {
    return 0;
  }

  return socket_->getAddrInfo().port;
}

bool PeerListenCommand::execute()
{
  if (e_->isHaltRequested() || e_->getRequestGroupMan()->downloadFinished()) {
    return true;
  }
  for (int i = 0; i < 3 && socket_->isReadable(0); ++i) {
    std::shared_ptr<SocketCore> peerSocket;
    try {
      peerSocket = socket_->acceptConnection();
      peerSocket->applyIpDscp();
      auto endpoint = peerSocket->getPeerInfo();

      auto peer = std::make_shared<Peer>(endpoint.addr, endpoint.port, true);
      cuid_t cuid = e_->newCUID();
      e_->addCommand(
          make_unique<ReceiverMSEHandshakeCommand>(cuid, peer, e_, peerSocket));
      A2_LOG_DEBUG(fmt("Accepted the connection from %s:%u.",
                       peer->getIPAddress().c_str(), peer->getPort()));
      A2_LOG_DEBUG(fmt(
          "Added CUID#%" PRId64 " to receive BitTorrent/MSE handshake.", cuid));
    }
    catch (RecoverableException& ex) {
      A2_LOG_DEBUG_EX(fmt(MSG_ACCEPT_FAILURE, getCuid()), ex);
    }
  }
  e_->addCommand(std::unique_ptr<Command>(this));
  return false;
}

} // namespace aria2
