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
#include "DHTConnectionSocksProxyImpl.h"

#include <netinet/in.h>
#include <cstring>

#include "SocketCore.h"
#include "a2functional.h"
#include "SocksProxySocket.h"

namespace aria2 {

DHTConnectionSocksProxyImpl::DHTConnectionSocksProxyImpl(int family)
    : DHTConnectionImpl(family), family_(family)
{
}

DHTConnectionSocksProxyImpl::~DHTConnectionSocksProxyImpl() = default;

bool DHTConnectionSocksProxyImpl::startProxy(const std::string& host,
                                             uint16_t port,
                                             const std::string& user,
                                             const std::string& passwd,
                                             const std::string& listenAddr,
                                             uint16_t listenPort)
{
  socket_ = make_unique<SocksProxySocket>(family_);
  socket_->establish(host, port);

  // Authentication negotiation
  bool noAuth = user.empty() || passwd.empty();
  if (noAuth) {
    int authMethod =
        socket_->negotiateAuth(std::vector<uint8_t>{SOCKS_AUTH_NO_AUTH});
    if (authMethod < 0) {
      return false;
    }
  }
  else {
    int authMethod = socket_->negotiateAuth(
        std::vector<uint8_t>{SOCKS_AUTH_NO_AUTH, SOCKS_AUTH_USERPASS});
    if (authMethod < 0) {
      return false;
    }

    // Username/Password authentication
    if (authMethod == SOCKS_AUTH_USERPASS) {
      int status = socket_->authByUserpass(user, passwd);
      if (status != 0) {
        return false;
      }
    }
  }

  // UDP associate
  size_t i =
      socket_->startUdpProxy(listenAddr, listenPort, &bndAddr_, &bndPort_);
  if (i < 0) {
    return false;
  }
  return true;
}

ssize_t DHTConnectionSocksProxyImpl::receiveMessage(unsigned char* data,
                                                    size_t len,
                                                    std::string& host,
                                                    uint16_t& port)
{
  Endpoint remoteEndpoint;
  size_t resLen = len + (family_ == AF_INET ? 10 : 22);
  std::string buf;
  buf.resize(resLen);
  ssize_t length = getSocket()->readDataFrom(&buf[0], resLen, remoteEndpoint);
  if (length == 0) {
    return length;
  }

  // unencapsulate SOCKS5 UDP header if has
  if (length > (family_ == AF_INET ? 10 : 22) &&
      buf.substr(0, 3) == std::string("\x00\x00\x00", 3) &&
      buf[3] == (family_ == AF_INET ? '\x01' : '\x04')) {
    if (family_ == AF_INET) {
      char addrBuf[20];
      inetNtop(AF_INET, &buf[4], addrBuf, 20);
      host = std::string(addrBuf);
      port = ntohs(*(reinterpret_cast<uint16_t*>(&buf[8])));
      memcpy(data, &buf[10], length - 10);
      return length - 10;
    }
    else {
      char addrBuf[50];
      inetNtop(AF_INET6, &buf[4], addrBuf, 50);
      host = std::string(addrBuf);
      port = ntohs(*(reinterpret_cast<uint16_t*>(&buf[20])));
      memcpy(data, &buf[22], length - 22);
      return length - 22;
    }
  }
  else {
    host = remoteEndpoint.addr;
    port = remoteEndpoint.port;
    return length;
  }
}

ssize_t DHTConnectionSocksProxyImpl::sendMessage(const unsigned char* data,
                                                 size_t len,
                                                 const std::string& host,
                                                 uint16_t port)
{
  std::string buf;
  if (family_ == AF_INET) {
    buf.resize(10);
    buf[3] = '\x01';
    // host is got from receiveMessage(). And as socket is binded according to
    // family, host should be the same family as family_. Omit family checking.
    net::getBinAddr(&buf[4], host);
    *(reinterpret_cast<uint16_t*>(&buf[8])) = htons(port);
    buf.append(reinterpret_cast<const char*>(data), len);
  }
  else {
    buf.resize(22);
    buf[3] = '\x04';
    net::getBinAddr(&buf[4], host);
    *(reinterpret_cast<uint16_t*>(&buf[20])) = htons(port);
    buf.append(reinterpret_cast<const char*>(data), len);
  }
  return getSocket()->writeData(&buf[0], buf.length(), bndAddr_, bndPort_);
}

} // namespace aria2
