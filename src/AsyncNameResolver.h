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
#ifndef D_ASYNC_NAME_RESOLVER_H
#define D_ASYNC_NAME_RESOLVER_H

#include "common.h"

#include <string>
#include <vector>

#include <ares.h>

#include "SharedHandle.h"
#include "a2netcompat.h"

namespace aria2 {

class AsyncNameResolver {
  friend void callback
  (void* arg, int status, int timeouts, struct hostent* host);
public:
  enum STATUS {
    STATUS_READY,
    STATUS_QUERYING,
    STATUS_SUCCESS,
    STATUS_ERROR,
  };
private:
  STATUS status_;
  int family_;
  ares_channel channel_;

  std::vector<std::string> resolvedAddresses_;
  std::string error_;
  std::string hostname_;
public:
  AsyncNameResolver
  (int family
#ifdef HAVE_ARES_ADDR_NODE
   , ares_addr_node* servers
#endif // HAVE_ARES_ADDR_NODE
   );

  ~AsyncNameResolver();

  void resolve(const std::string& name);

  const std::vector<std::string>& getResolvedAddresses() const
  {
    return resolvedAddresses_;
  }

  const std::string& getError() const
  {
    return error_;
  }

  STATUS getStatus() const
  {
    return status_;
  }

  int getFds(fd_set* rfdsPtr, fd_set* wfdsPtr) const;

  void process(fd_set* rfdsPtr, fd_set* wfdsPtr);

#ifdef HAVE_LIBCARES

  int getsock(sock_t* sockets) const;

  void process(ares_socket_t readfd, ares_socket_t writefd);

#endif // HAVE_LIBCARES

  bool operator==(const AsyncNameResolver& resolver) const;

  void setAddr(const std::string& addrString);

  void reset();

  const std::string& getHostname() const
  {
    return hostname_;
  }

};

#ifdef HAVE_ARES_ADDR_NODE

ares_addr_node* parseAsyncDNSServers(const std::string& serversOpt);

#endif // HAVE_ARES_ADDR_NODE


} // namespace aria2

#endif // D_ASYNC_NAME_RESOLVER_H
