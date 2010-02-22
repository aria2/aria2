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
#include "LpdMessageDispatcher.h"
#include "SocketCore.h"
#include "A2STR.h"
#include "util.h"
#include "Logger.h"
#include "LogFactory.h"
#include "BtConstants.h"
#include "RecoverableException.h"

namespace aria2 {

LpdMessageDispatcher::LpdMessageDispatcher
(const std::string& infoHash, uint16_t port,
 const std::string& multicastAddress, uint16_t multicastPort,
 time_t interval):
  _infoHash(infoHash),
  _port(port),
  _multicastAddress(multicastAddress),
  _multicastPort(multicastPort),
  _timer(0),
  _interval(interval),
  _request(bittorrent::createLpdRequest(_multicastAddress, _multicastPort,
                                        _infoHash, _port)),
  _logger(LogFactory::getInstance()) {}

bool LpdMessageDispatcher::init(const std::string& localAddr,
                                unsigned char ttl, unsigned char loop)
{
  try {
    _socket.reset(new SocketCore(SOCK_DGRAM));
    _socket->create(AF_INET);
    if(_logger->debug()) {
      _logger->debug("Setting multicast outgoing interface=%s",
                     localAddr.c_str());
    }
    _socket->setMulticastInterface(localAddr);
    if(_logger->debug()) {
      _logger->debug("Setting multicast ttl=%u",static_cast<unsigned int>(ttl));
    }
    _socket->setMulticastTtl(ttl);
    if(_logger->debug()) {
      _logger->debug("Setting multicast loop=%u",
                     static_cast<unsigned int>(loop));
    }
    _socket->setMulticastLoop(loop);
    return true;
  } catch(RecoverableException& e) {
    _logger->error("Failed to initialize LpdMessageDispatcher.", e);
  }
  return false;
}

bool LpdMessageDispatcher::sendMessage()
{
  return
    _socket->writeData(_request.c_str(), _request.size(),
                       _multicastAddress, _multicastPort)
    == (ssize_t)_request.size();
}

bool LpdMessageDispatcher::isAnnounceReady() const
{
  return _timer.elapsed(_interval);
}

void LpdMessageDispatcher::resetAnnounceTimer()
{
  _timer.reset();
}

namespace bittorrent {

std::string createLpdRequest
(const std::string& multicastAddress, uint16_t multicastPort,
 const std::string& infoHash, uint16_t port)
{
  std::string req = "BT-SEARCH * HTTP/1.1\r\n";
  strappend(req, "Host: ", multicastAddress, A2STR::COLON_C,
            util::uitos(multicastPort), A2STR::CRLF);
  strappend(req, "Port: ", util::uitos(port), A2STR::CRLF);
  strappend(req, "Infohash: ", util::toHex(infoHash), A2STR::CRLF);
  req += "\r\n\r\n";
  return req;
}

} // namespace bittorrent

} // namespac aria2
