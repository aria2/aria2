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
#include "Socket.h"
#include "SimpleRandomizer.h"
#include "fmt.h"

namespace aria2 {

DHTConnectionImpl::DHTConnectionImpl(int family)
  : socket_(new SocketCore(SOCK_DGRAM)),
    family_(family)
{}

DHTConnectionImpl::~DHTConnectionImpl() {}

bool DHTConnectionImpl::bind
(uint16_t& port, const std::string& addr, SegList<int>& sgl)
{
  std::vector<uint16_t> ports;
  while(sgl.hasNext()) {
    ports.push_back(sgl.next());
  }
  std::random_shuffle(ports.begin(), ports.end(),
                      *SimpleRandomizer::getInstance().get());
  for(std::vector<uint16_t>::const_iterator i = ports.begin(),
        eoi = ports.end(); i != eoi; ++i) {
    port = *i;
    if(bind(port, addr)) {
      return true;
    }
  }
  return false;
}

bool DHTConnectionImpl::bind(uint16_t& port, const std::string& addr)
{
  const int ipv = (family_ == AF_INET) ? 4 : 6;
  try {
    if(addr.empty()) {
      socket_->bind(A2STR::NIL, port, family_);
    } else {
      socket_->bind(addr, port, family_);
    }
    socket_->setNonBlockingMode();
    std::pair<std::string, uint16_t> svaddr;
    socket_->getAddrInfo(svaddr);
    port = svaddr.second;
    A2_LOG_NOTICE(fmt(_("IPv%d DHT: listening to port %u"), ipv, port));
    return true;
  } catch(RecoverableException& e) {
    A2_LOG_ERROR_EX(fmt("IPv%d DHT: failed to bind port %u", ipv, port), e);
  }
  return false;
}

ssize_t DHTConnectionImpl::receiveMessage(unsigned char* data, size_t len,
                                          std::string& host, uint16_t& port)
{
  std::pair<std::string, uint16_t> remoteHost;
  ssize_t length = socket_->readDataFrom(data, len, remoteHost);
  if(length == 0) {
    return length;
  } else {
    host = remoteHost.first;
    port = remoteHost.second;
    return length;
  }
}

ssize_t DHTConnectionImpl::sendMessage(const unsigned char* data, size_t len,
                                       const std::string& host, uint16_t port)
{
  return socket_->writeData(data, len, host, port);
}

} // namespace aria2
