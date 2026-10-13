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
#ifndef D_SOCKS5_HANDSHAKE_H
#define D_SOCKS5_HANDSHAKE_H

#include "common.h"

#include <string>
#include <vector>

namespace aria2 {

class SocketCore;

// Non-blocking client side of the SOCKS5 handshake (RFC 1928, with
// the username/password sub-negotiation of RFC 1929) that asks the
// proxy to CONNECT to the given target.  It reads exactly the bytes
// it needs, so no data sent by the target after the tunnel is
// established is consumed.
class Socks5Handshake {
public:
  enum Status {
    // The handshake needs the socket to become readable.
    WANT_READ,
    // The handshake needs the socket to become writable.
    WANT_WRITE,
    // The tunnel has been established.
    DONE
  };

  // If targetAddr is a numeric address, it is sent as ATYP IPv4/IPv6
  // (the target was resolved locally, "socks5" scheme).  Otherwise
  // targetHost is sent as ATYP DOMAINNAME and the proxy resolves it
  // ("socks5h" scheme).  A numeric targetHost is always sent as an
  // address.  If user is empty, the username/password authentication
  // is not offered.
  Socks5Handshake(const std::string& user, const std::string& passwd,
                  const std::string& targetHost, const std::string& targetAddr,
                  uint16_t targetPort);

  // Makes as much progress as possible without blocking.
  Status advance(SocketCore& socket);

private:
  enum State {
    GREETING_SEND,
    GREETING_RECV,
    AUTH_SEND,
    AUTH_RECV,
    CONNECT_SEND,
    CONNECT_RECV_HEAD,
    CONNECT_RECV_DOMAIN_LEN,
    CONNECT_RECV_TAIL,
    FINISHED
  };

  bool flush(SocketCore& socket);
  // Reads until recvBuf_ holds recvNeeded_ bytes.
  bool fill(SocketCore& socket);
  void startRecv(State state, size_t n);
  void startSend(State state);
  void buildConnect();
  void processGreetingResponse();
  void processAuthResponse();
  void processConnectHead();

  std::string user_;
  std::string passwd_;
  std::string targetHost_;
  std::string targetAddr_;
  uint16_t targetPort_;
  State state_;
  std::vector<unsigned char> sendBuf_;
  size_t sendOffset_;
  std::vector<unsigned char> recvBuf_;
  size_t recvNeeded_;
};

} // namespace aria2

#endif // D_SOCKS5_HANDSHAKE_H
