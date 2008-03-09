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
#ifndef _D_NAME_RESOLVER_H_
#define _D_NAME_RESOLVER_H_

#include "common.h"
#include "SharedHandle.h"
#include "a2netcompat.h"
#include <string>

#ifdef ENABLE_ASYNC_DNS

#ifdef __cplusplus
extern "C" {
#endif
#include <ares.h>
#ifdef __cplusplus
} /* end of extern "C" */
#endif

#endif // ENABLE_ASYNC_DNS

namespace aria2 {

#ifdef ENABLE_ASYNC_DNS

#ifdef HAVE_LIBCARES1_5
void callback(void* arg, int status, int timeouts, struct hostent* host);
#else
void callback(void* arg, int status, struct hostent* host);
#endif // HAVE_LIBCARES1_5

class NameResolver {
#ifdef HAVE_LIBCARES1_5
  friend void callback(void* arg, int status, int timeouts, struct hostent* host);
#else
  friend void callback(void* arg, int status, struct hostent* host);
#endif // HAVE_LIBCARES1_5

public:
  enum STATUS {
    STATUS_READY,
    STATUS_QUERYING,
    STATUS_SUCCESS,
    STATUS_ERROR,
  };
private:
  STATUS status;
  ares_channel channel;
  struct in_addr addr;
  std::string error;
public:
  NameResolver():
    status(STATUS_READY)
  {
    // TODO evaluate return value
    ares_init(&channel);
  }

  ~NameResolver() {
    ares_destroy(channel);
  }

  void resolve(const std::string& name);

  std::string getAddrString() const;

  const struct in_addr& getAddr() const {
    return addr;
  }

  const std::string& getError() const {
    return error;
  }

  STATUS getStatus() const {
    return status;
  }

  int getFds(fd_set* rfdsPtr, fd_set* wfdsPtr) const {
    return ares_fds(channel, rfdsPtr, wfdsPtr);
  }

  void process(fd_set* rfdsPtr, fd_set* wfdsPtr) {
    ares_process(channel, rfdsPtr, wfdsPtr);
  }

  bool operator==(const NameResolver& resolver) {
    return this == &resolver;
  }

  void setAddr(const std::string& addrString);

  void reset();
};

#else // ENABLE_ASYNC_DNS

class NameResolver {
private:
  struct in_addr _addr;
public:
  void resolve(const std::string& hostname);

  std::string getAddrString() const;
  
  void setAddr(const std::string& addrString);

  void reset();
};

#endif // ENABLE_ASYNC_DNS

typedef SharedHandle<NameResolver> NameResolverHandle;

} // namespace aria2

#endif // _D_NAME_RESOLVER_H_
