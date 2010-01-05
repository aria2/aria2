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
#include <deque>
#include <algorithm>

#include "LogFactory.h"
#include "Logger.h"
#include "RecoverableException.h"
#include "util.h"
#include "Socket.h"
#include "SimpleRandomizer.h"

namespace aria2 {

DHTConnectionImpl::DHTConnectionImpl():_socket(new SocketCore(SOCK_DGRAM)),
                                       _logger(LogFactory::getInstance()) {}

DHTConnectionImpl::~DHTConnectionImpl() {}

bool DHTConnectionImpl::bind(uint16_t& port, IntSequence& ports)
{
  std::deque<int32_t> randPorts = ports.flush();
  std::random_shuffle(randPorts.begin(), randPorts.end(),
                      *SimpleRandomizer::getInstance().get());
  
  for(std::deque<int32_t>::const_iterator portItr = randPorts.begin();
      portItr != randPorts.end(); ++portItr) {
    if(!(0 < (*portItr) && (*portItr) <= 65535)) {
      continue;
    }
    port = (*portItr);
    if(bind(port)) {
      return true;
    }
  }
  return false;
}

bool DHTConnectionImpl::bind(uint16_t& port)
{
  try {
    _socket->bind(port);
    _socket->setNonBlockingMode();
    std::pair<std::string, uint16_t> svaddr;
    _socket->getAddrInfo(svaddr);
    port = svaddr.second;
    _logger->notice("DHT: listening to port %d", port);
    return true;
  } catch(RecoverableException& e) {
    _logger->error("Failed to bind for DHT. port=%u", e, port);
  }
  return false;
}

ssize_t DHTConnectionImpl::receiveMessage(unsigned char* data, size_t len,
                                          std::string& host, uint16_t& port)
{
  std::pair<std::string, uint16_t> remoteHost;
  ssize_t length = _socket->readDataFrom(data, len, remoteHost);
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
  return _socket->writeData(data, len, host, port);
}

} // namespace aria2
