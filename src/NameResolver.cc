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
#include "NameResolver.h"
#include <cstring>

namespace aria2 {

#ifdef ENABLE_ASYNC_DNS

#ifdef HAVE_LIBCARES1_5
void callback(void* arg, int status, int timeouts, struct hostent* host)
#else
void callback(void* arg, int status, struct hostent* host)
#endif // HAVE_LIBCARES1_5
{
  NameResolver* resolverPtr = (NameResolver*)arg;
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
    resolverPtr->status = NameResolver::STATUS_ERROR;
    return;
  }
  memcpy(&resolverPtr->addr, *host->h_addr_list, sizeof(struct in_addr));
  resolverPtr->status = NameResolver::STATUS_SUCCESS;
}

void NameResolver::resolve(const std::string& name)
{
  status = STATUS_QUERYING;
  ares_gethostbyname(channel, name.c_str(), AF_INET, callback, this);
}

std::string NameResolver::getAddrString() const
{
  return inet_ntoa(addr);
}

void NameResolver::setAddr(const std::string& addrString)
{
  inet_aton(addrString.c_str(), &addr);
}
 
#else // ENABLE_ASYNC_DNS

#include "DlAbortEx.h"
#include "message.h"

void NameResolver::resolve(const std::string& hostname)
{
  memset(&_addr, 0, sizeof(in_addr));
  struct addrinfo ai;
  memset((char*)&ai, 0, sizeof(ai));
  ai.ai_flags = 0;
  ai.ai_family = PF_INET;
  ai.ai_socktype = SOCK_STREAM;
  ai.ai_protocol = 0; 
  struct addrinfo* res;
  int ec;
  if((ec = getaddrinfo(hostname.c_str(), 0, &ai, &res)) != 0) {
    throw new DlAbortEx(EX_RESOLVE_HOSTNAME,
			hostname.c_str(), gai_strerror(ec));
  }
  _addr = ((struct sockaddr_in*)res->ai_addr)->sin_addr;
  freeaddrinfo(res);
}

std::string NameResolver::getAddrString() const
{
  return inet_ntoa(_addr);
}

void NameResolver::setAddr(const std::string& addrString)
{
  inet_aton(addrString.c_str(), &_addr);
}

#endif // ENABLE_ASYNC_DNS

} // namespace aria2
