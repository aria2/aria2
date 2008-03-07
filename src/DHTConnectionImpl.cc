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
#include "DHTConnectionImpl.h"
#include "LogFactory.h"
#include "Logger.h"
#include "RecoverableException.h"
#include "Util.h"
#include "Socket.h"
#include <utility>

namespace aria2 {

DHTConnectionImpl::DHTConnectionImpl():_socket(new SocketCore(SOCK_DGRAM)),
				       _logger(LogFactory::getInstance()) {}

DHTConnectionImpl::~DHTConnectionImpl() {}

uint16_t DHTConnectionImpl::bind(IntSequence& ports)
{
  while(ports.hasNext()) {
    uint16_t port = bind(ports.next());
    if(port > 0) {
      return port;
    }
  }
  return 0;
}

uint16_t DHTConnectionImpl::bind(uint16_t port)
{
  try {
    _socket->bind(port);
    std::pair<std::string, int32_t> svaddr;
    _socket->getAddrInfo(svaddr);
    _logger->info("Bind socket for DHT. port=%u", port);
    return svaddr.second;
  } catch(RecoverableException* e) {
    _logger->error("Failed to bind for DHT. port=%u", e, port);
    delete e;
  }
  return 0;
}

ssize_t DHTConnectionImpl::receiveMessage(unsigned char* data, size_t len, std::string& host, uint16_t& port)
{
  if(_socket->isReadable(0)) {
    std::pair<std::string, uint16_t> remoteHost;
    ssize_t length = _socket->readDataFrom(data, len, remoteHost);
    host = remoteHost.first;
    port = remoteHost.second;
    return length;
  } else {
    return -1;
  }
}

void DHTConnectionImpl::sendMessage(const unsigned char* data, size_t len, const std::string& host, uint16_t port)
{
  _socket->writeData(data, len, host, port);
}

SharedHandle<SocketCore> DHTConnectionImpl::getSocket() const
{
  return _socket;
}

} // namespace aria2
