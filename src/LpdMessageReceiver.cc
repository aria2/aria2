/* <!-- copyright */
/*
 * aria2 - The high speed download utility
 *
 * Copyright (C) 2010 Tatsuhiro Tsujikawa
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

#include "LpdMessageReceiver.h"
#include "SocketCore.h"
#include "Logger.h"
#include "LogFactory.h"
#include "HttpHeaderProcessor.h"
#include "HttpHeader.h"
#include "util.h"
#include "LpdMessage.h"
#include "RecoverableException.h"
#include "Peer.h"
#include "fmt.h"

namespace aria2 {

LpdMessageReceiver::LpdMessageReceiver(const std::string& multicastAddress,
                                       uint16_t multicastPort)
    : multicastAddress_(multicastAddress), multicastPort_(multicastPort)
{
}

LpdMessageReceiver::~LpdMessageReceiver() = default;

bool LpdMessageReceiver::init(const std::string& localAddr)
{
  try {
    socket_ = std::make_shared<SocketCore>(SOCK_DGRAM);
#ifdef __MINGW32__
    // Binding multicast address fails under Windows.
    socket_->bindWithFamily(multicastPort_, AF_INET);
#else  // !__MINGW32__
    socket_->bind(multicastAddress_.c_str(), multicastPort_, AF_INET);
#endif // !__MINGW32__
    A2_LOG_DEBUG(fmt("Joining multicast group. %s:%u, localAddr=%s",
                     multicastAddress_.c_str(), multicastPort_,
                     localAddr.c_str()));
    socket_->joinMulticastGroup(multicastAddress_, multicastPort_, localAddr);
    socket_->setNonBlockingMode();
    localAddress_ = localAddr;
    A2_LOG_INFO(fmt("Listening multicast group (%s:%u) packet",
                    multicastAddress_.c_str(), multicastPort_));
    return true;
  }
  catch (RecoverableException& e) {
    A2_LOG_ERROR_EX("Failed to initialize LPD message receiver.", e);
  }
  return false;
}

std::unique_ptr<LpdMessage> LpdMessageReceiver::receiveMessage()
{
  while (1) {
    unsigned char buf[200];
    Endpoint remoteEndpoint;
    ssize_t length;
    try {
      length = socket_->readDataFrom(buf, sizeof(buf), remoteEndpoint);
      if (length == 0) {
        return nullptr;
      }
    }
    catch (RecoverableException& e) {
      A2_LOG_INFO_EX("Failed to receive LPD message.", e);
      return nullptr;
    }
    HttpHeaderProcessor proc(HttpHeaderProcessor::SERVER_PARSER);
    try {
      if (!proc.parse(buf, length)) {
        // UDP packet must contain whole HTTP header block.
        continue;
      }
    }
    catch (RecoverableException& e) {
      A2_LOG_INFO_EX("Failed to parse LPD message.", e);
      continue;
    }
    auto header = proc.getResult();
    const std::string& infoHashString = header->find(HttpHeader::INFOHASH);
    uint32_t port = 0;
    if (!util::parseUIntNoThrow(port, header->find(HttpHeader::PORT)) ||
        port > UINT16_MAX || port == 0) {
      A2_LOG_INFO(fmt("Bad LPD port=%u", port));
      continue;
    }
    A2_LOG_INFO(fmt("LPD message received infohash=%s, port=%u from %s",
                    infoHashString.c_str(), port, remoteEndpoint.addr.c_str()));
    std::string infoHash;
    if (infoHashString.size() != 40 ||
        (infoHash = util::fromHex(infoHashString.begin(), infoHashString.end()))
            .empty()) {
      A2_LOG_INFO(fmt("LPD bad request. infohash=%s", infoHashString.c_str()));
      continue;
    }
    auto peer = std::make_shared<Peer>(remoteEndpoint.addr, port, false);
    if (util::inPrivateAddress(remoteEndpoint.addr)) {
      peer->setLocalPeer(true);
    }
    return make_unique<LpdMessage>(peer, infoHash);
  }
}

} // namespace aria2
