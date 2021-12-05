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
#ifndef D_DHT_CONNECTION_SOCKS_PROXY_IMPL_H
#define D_DHT_CONNECTION_SOCKS_PROXY_IMPL_H

#include "DHTConnectionImpl.h"

#include <memory>

namespace aria2 {

class SocketCore;

class DHTConnectionSocksProxyImpl : public DHTConnectionImpl {
private:
  std::unique_ptr<SocketCore> proxySocket_;
  int family_;
  std::string bndAddr_;
  uint16_t bndPort_;

public:
  DHTConnectionSocksProxyImpl(int family);

  virtual ~DHTConnectionSocksProxyImpl();

  // Connect to SOCKS5 server to start UDP proxy.
  // Leave user and passwd empty to indicate no authentication.
  // Leave listen host and port empty / 0 to indicate no receiving from proxy.
  bool startProxy(const std::string& host, uint16_t port,
                  const std::string& user, const std::string& passwd,
                  const std::string& listenHost, uint16_t listenPort);

  virtual ssize_t receiveMessage(unsigned char* data, size_t len,
                                 std::string& host,
                                 uint16_t& port) CXX11_OVERRIDE;

  virtual ssize_t sendMessage(const unsigned char* data, size_t len,
                              const std::string& host,
                              uint16_t port) CXX11_OVERRIDE;
};

} // namespace aria2

#endif // D_DHT_CONNECTION_SOCKS_PROXY_IMPL_H
