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
#ifndef D_DHT_CONNECTION_IMPL_H
#define D_DHT_CONNECTION_IMPL_H

#include "DHTConnection.h"

#include <memory>

#include "SegList.h"

namespace aria2 {

class SocketCore;

class DHTConnectionImpl : public DHTConnection {
private:
  std::shared_ptr<SocketCore> socket_;

  int family_;

public:
  DHTConnectionImpl(int family);

  virtual ~DHTConnectionImpl();

  /**
   * Binds port. All number in sgl are tried. All numbers in sgl must
   * be in range [1, 65535], inclusive. If successful, the bound port
   * is assigned to port and returns true.  Otherwise return false and
   * port is undefined in this case.  If non-empty string addr is
   * given, the socket is associated to the address.
   */
  bool bind(uint16_t& port, const std::string& addr, SegList<int>& sgl);

  /**
   * Binds port. The port number specified by port is used to bind.
   * If successful, the bound port is assigned to port and returns
   * true.  Otherwise return false and port is undefined in this case.
   * If non-empty string addr is given, the socket is associated to
   * the address.
   *
   * If you want to bind arbitrary port, give 0 as port and if successful,
   * the bound port is assigned to port.
   */
  bool bind(uint16_t& port, const std::string& addr);

  virtual ssize_t receiveMessage(unsigned char* data, size_t len,
                                 std::string& host,
                                 uint16_t& port) CXX11_OVERRIDE;

  virtual ssize_t sendMessage(const unsigned char* data, size_t len,
                              const std::string& host,
                              uint16_t port) CXX11_OVERRIDE;

  const std::shared_ptr<SocketCore>& getSocket() const { return socket_; }
};

} // namespace aria2

#endif // D_DHT_CONNECTION_IMPL_H
