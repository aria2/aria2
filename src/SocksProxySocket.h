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
#ifndef D_SOCKS_PROXY_SOCKET_H
#define D_SOCKS_PROXY_SOCKET_H

#include <memory>
#include <vector>
#include <string>

namespace aria2 {

class SocketCore;

enum SocksProxyAuthMethod {
  // No authentication required
  SOCKS_AUTH_NO_AUTH = 0,
  // Username/Password authentication
  SOCKS_AUTH_USERPASS = 2,
};

// Other than transferring proxy traffic,
// the class helps to start proxy connections.
class SocksProxySocket {
private:
  int family_;
  // The socket is not shared because as it is used as some kind of controller,
  // when it is closed, all related proxy connections are closed.
  std::unique_ptr<SocketCore> socket_;

  enum SocksProxyCmd {
    SOCKS_CMD_CONNECT = 1,
    SOCKS_CMD_BIND = 2,
    SOCKS_CMD_UDP_ASSOCIATE = 3,
  };

  // When allowEmtpy is true, allow dst* to be empty.
  void sendCmd(SocksProxyCmd cmd, const std::string& dstAddr, uint16_t dstPort,
               bool allowEmpty);
  // Returns status replied from proxy server. 0 is OK. > 0 with error messages.
  // < 0 for invalid replies from server.
  // bndFamily may be AF_INET + AF_INET6 to indicate domain format.
  int receiveReply(int& bndFamily, std::string& bndAddr, uint16_t& bndPort);

public:
  SocksProxySocket(int family);

  virtual ~SocksProxySocket();

  // Establish connection to SOCKS port.
  void establish(const std::string& host, uint16_t port);

  // Customize the TCP socket for SOCKS protocol.
  // Socket should be blocking to ensure latter steps finished in order.
  void establish(std::unique_ptr<SocketCore> socket);

  // Negotiate authentication that returns selected auth method in 0-255.
  // When no auth method is selected, return -1.
  // Returned auth method SHOULD be in expected but it is not checked.
  int negotiateAuth(std::vector<SocksProxyAuthMethod> expected);

  // Username/Password authentication.
  // user and pass should not be empty.
  // Returns status replied from proxy server. 0 is OK.
  int authByUserpass(const std::string& user, const std::string& passwd);

  // Helper to negotiate and auth by username/password authentication, or skip
  // it if not required.
  // Leave either user or pass empty to force no authentication.
  bool authByUserpassOrNone(const std::string& user, const std::string& passwd);

  // Create an UDP association to start UDP proxy.
  // Leave listen host and port empty / 0 to indicate no receiving from proxy.
  // Returns status replied from proxy server. 0 is OK.
  int startUdpAssociate(const std::string& listenAddr, uint16_t listenPort,
                        std::string& bndAddr, uint16_t& bndPort);
};

} // namespace aria2

#endif // D_SOCKS_PROXY_SOCKET_H
