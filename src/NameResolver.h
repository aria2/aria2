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
#include <netdb.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#ifdef __cplusplus
extern "C" {
#endif
#include <ares.h>
#ifdef __cplusplus
} /* end of extern "C" */
#endif

void callback(void* arg, int status, struct hostent* host);

class NameResolver {
  friend void callback(void* arg, int status, struct hostent* host);

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
  string error;
public:
  NameResolver():
    status(STATUS_READY)
  {
    ares_init(&channel);
  }

  ~NameResolver() {
    ares_destroy(channel);
  }

  void resolve(const string& name) {
    status = STATUS_QUERYING;
    ares_gethostbyname(channel, name.c_str(), AF_INET, callback, this);
  }

  string getAddrString() const {
    return inet_ntoa(addr);
  }

  const struct in_addr& getAddr() const {
    return addr;
  }

  const string& getError() const {
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
};

typedef SharedHandle<NameResolver> NameResolverHandle;

#endif // _D_NAME_RESOLVER_H_
