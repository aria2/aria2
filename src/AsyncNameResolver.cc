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
#include "AsyncNameResolver.h"
#include "Util.h"
#include "A2STR.h"
#include <cstring>

namespace aria2 {

#ifdef HAVE_LIBCARES1_5
void callback(void* arg, int status, int timeouts, struct hostent* host)
#else
void callback(void* arg, int status, struct hostent* host)
#endif // HAVE_LIBCARES1_5
{
  AsyncNameResolver* resolverPtr = reinterpret_cast<AsyncNameResolver*>(arg);
#ifdef HAVE_LIBARES
  // This block is required since the assertion in ares_strerror fails
  // if status = ARES_EDESTRUCTION is passed to ares_strerror as 1st argument.
  // This does not happen in c-ares.
  if(status == ARES_EDESTRUCTION) {
    // we simply return in this case.
    return;
  }
#endif
  if(status != ARES_SUCCESS) {
#ifdef HAVE_LIBCARES
    resolverPtr->error = ares_strerror(status);
#else
    resolverPtr->error = ares_strerror(status, 0);
#endif // HAVE_LIBCARES
    resolverPtr->status = AsyncNameResolver::STATUS_ERROR;
    return;
  }
  for(char** ap = host->h_addr_list; *ap; ++ap) {
    resolverPtr->_resolvedAddresses.push_back
      (inet_ntoa(*reinterpret_cast<struct in_addr*>(*ap)));
  }
  resolverPtr->status = AsyncNameResolver::STATUS_SUCCESS;
}

AsyncNameResolver::AsyncNameResolver():
  status(STATUS_READY)
{
  // TODO evaluate return value
  ares_init(&channel);
}

AsyncNameResolver::~AsyncNameResolver()
{
  ares_destroy(channel);
}

void AsyncNameResolver::resolve(const std::string& name)
{
  _hostname = name;
  status = STATUS_QUERYING;
  ares_gethostbyname(channel, name.c_str(), AF_INET, callback, this);
}

const std::deque<std::string>& AsyncNameResolver::getResolvedAddresses() const
{
  return _resolvedAddresses;
}

const std::string& AsyncNameResolver::getError() const
{
  return error;
}

AsyncNameResolver::STATUS AsyncNameResolver::getStatus() const
{
  return status;
}

int AsyncNameResolver::getFds(fd_set* rfdsPtr, fd_set* wfdsPtr) const
{
  return ares_fds(channel, rfdsPtr, wfdsPtr);
}

void AsyncNameResolver::process(fd_set* rfdsPtr, fd_set* wfdsPtr)
{
  ares_process(channel, rfdsPtr, wfdsPtr);
}

#ifdef HAVE_LIBCARES

int AsyncNameResolver::getsock(sock_t* sockets) const
{
  return ares_getsock(channel, sockets, ARES_GETSOCK_MAXNUM);
}

void AsyncNameResolver::process(ares_socket_t readfd, ares_socket_t writefd)
{
  ares_process_fd(channel, readfd, writefd);
}

#endif // HAVE_LIBCARES

bool AsyncNameResolver::operator==(const AsyncNameResolver& resolver) const
{
  return this == &resolver;
}

void AsyncNameResolver::reset()
{
  _hostname = A2STR::NIL;
  _resolvedAddresses.clear();
  status = STATUS_READY;
  ares_destroy(channel);
  // TODO evaluate return value
  ares_init(&channel);
}

const std::string& AsyncNameResolver::getHostname() const
{
  return _hostname;
}

} // namespace aria2
