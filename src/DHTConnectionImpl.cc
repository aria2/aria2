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
#include "DHTConnectionImpl.h"

#include <utility>
#include <algorithm>

#include "LogFactory.h"
#include "Logger.h"
#include "RecoverableException.h"
#include "util.h"
#include "SocketCore.h"
#include "SimpleRandomizer.h"
#include "fmt.h"

namespace aria2 {

DHTConnectionImpl::DHTConnectionImpl(int family)
    : socket_(std::make_shared<SocketCore>(SOCK_DGRAM)), family_(family)
{
}

DHTConnectionImpl::~DHTConnectionImpl() = default;

bool DHTConnectionImpl::bind(uint16_t& port, const std::string& addr,
                             SegList<int>& sgl)
{
  std::vector<uint16_t> ports;
  while (sgl.hasNext()) {
    ports.push_back(sgl.next());
  }
  std::shuffle(ports.begin(), ports.end(), *SimpleRandomizer::getInstance());
  for (const auto& p : ports) {
    port = p;
    if (bind(port, addr)) {
      return true;
    }
  }
  return false;
}

bool DHTConnectionImpl::bind(uint16_t& port, const std::string& addr)
{
  const int ipv = (family_ == AF_INET) ? 4 : 6;
  try {
    socket_->bind(addr.c_str(), port, family_);
    socket_->setNonBlockingMode();
    auto endpoint = socket_->getAddrInfo();
    port = endpoint.port;
    A2_LOG_NOTICE(fmt(_("IPv%d DHT: listening on UDP port %u"), ipv, port));
    return true;
  }
  catch (RecoverableException& e) {
    A2_LOG_ERROR_EX(fmt("IPv%d DHT: failed to bind UDP port %u", ipv, port), e);
  }
  return false;
}

ssize_t DHTConnectionImpl::receiveMessage(unsigned char* data, size_t len,
                                          std::string& host, uint16_t& port)
{
  Endpoint remoteEndpoint;
  ssize_t length = socket_->readDataFrom(data, len, remoteEndpoint);
  if (length == 0) {
    return length;
  }

  host = remoteEndpoint.addr;
  port = remoteEndpoint.port;
  return length;
}

ssize_t DHTConnectionImpl::sendMessage(const unsigned char* data, size_t len,
                                       const std::string& host, uint16_t port)
{
  return socket_->writeData(data, len, host, port);
}

} // namespace aria2
