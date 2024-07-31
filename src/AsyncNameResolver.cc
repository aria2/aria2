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
#include "AsyncNameResolver.h"

#include <cstring>

#include "A2STR.h"
#include "LogFactory.h"
#include "SocketCore.h"
#include "util.h"
#include "EventPoll.h"

namespace aria2 {

void callback(void* arg, int status, int timeouts, ares_addrinfo* result)
{
  AsyncNameResolver* resolverPtr = reinterpret_cast<AsyncNameResolver*>(arg);
  if (status != ARES_SUCCESS) {
    resolverPtr->error_ = ares_strerror(status);
    resolverPtr->status_ = AsyncNameResolver::STATUS_ERROR;
    return;
  }
  for (auto ap = result->nodes; ap; ap = ap->ai_next) {
    char addrstring[NI_MAXHOST];
    auto rv = getnameinfo(ap->ai_addr, ap->ai_addrlen, addrstring,
                          sizeof(addrstring), nullptr, 0, NI_NUMERICHOST);
    if (rv == 0) {
      resolverPtr->resolvedAddresses_.push_back(addrstring);
    }
  }
  ares_freeaddrinfo(result);
  if (resolverPtr->resolvedAddresses_.empty()) {
    resolverPtr->error_ = "no address returned or address conversion failed";
    resolverPtr->status_ = AsyncNameResolver::STATUS_ERROR;
  }
  else {
    resolverPtr->status_ = AsyncNameResolver::STATUS_SUCCESS;
  }
}

namespace {
void sock_state_cb(void* arg, ares_socket_t fd, int read, int write)
{
  auto resolver = static_cast<AsyncNameResolver*>(arg);

  resolver->handle_sock_state(fd, read, write);
}
} // namespace

void AsyncNameResolver::handle_sock_state(ares_socket_t fd, int read, int write)
{
  int events = 0;

  if (read) {
    events |= EventPoll::EVENT_READ;
  }

  if (write) {
    events |= EventPoll::EVENT_WRITE;
  }

  auto it = std::find_if(
      std::begin(socks_), std::end(socks_),
      [fd](const AsyncNameResolverSocketEntry& ent) { return ent.fd == fd; });
  if (it == std::end(socks_)) {
    if (!events) {
      return;
    }

    socks_.emplace_back(AsyncNameResolverSocketEntry{fd, events});

    return;
  }

  if (!events) {
    socks_.erase(it);
    return;
  }

  (*it).events = events;
}

AsyncNameResolver::AsyncNameResolver(int family, const std::string& servers)
    : status_(STATUS_READY), family_(family)
{
  ares_options opts{};
  opts.sock_state_cb = sock_state_cb;
  opts.sock_state_cb_data = this;

  // TODO evaluate return value
  ares_init_options(&channel_, &opts, ARES_OPT_SOCK_STATE_CB);

  if (!servers.empty()) {
    if (ares_set_servers_csv(channel_, servers.c_str()) != ARES_SUCCESS) {
      A2_LOG_DEBUG("ares_set_servers_csv failed");
    }
  }
}

AsyncNameResolver::~AsyncNameResolver() { ares_destroy(channel_); }

void AsyncNameResolver::resolve(const std::string& name)
{
  hostname_ = name;
  status_ = STATUS_QUERYING;

  ares_addrinfo_hints hints{};
  hints.ai_family = family_;

  ares_getaddrinfo(channel_, name.c_str(), nullptr, &hints, callback, this);
}

ares_socket_t AsyncNameResolver::getFds(fd_set* rfdsPtr, fd_set* wfdsPtr) const
{
  ares_socket_t nfds = 0;

  for (const auto& ent : socks_) {
    if (ent.events & EventPoll::EVENT_READ) {
      FD_SET(ent.fd, rfdsPtr);
      nfds = std::max(nfds, ent.fd + 1);
    }

    if (ent.events & EventPoll::EVENT_WRITE) {
      FD_SET(ent.fd, wfdsPtr);
      nfds = std::max(nfds, ent.fd + 1);
    }
  }

  return nfds;
}

void AsyncNameResolver::process(fd_set* rfdsPtr, fd_set* wfdsPtr)
{
  for (const auto& ent : socks_) {
    ares_socket_t readfd = ARES_SOCKET_BAD;
    ares_socket_t writefd = ARES_SOCKET_BAD;

    if (FD_ISSET(ent.fd, rfdsPtr) && (ent.events & EventPoll::EVENT_READ)) {
      readfd = ent.fd;
    }

    if (FD_ISSET(ent.fd, wfdsPtr) && (ent.events & EventPoll::EVENT_WRITE)) {
      writefd = ent.fd;
    }

    if (readfd != ARES_SOCKET_BAD || writefd != ARES_SOCKET_BAD) {
      process(readfd, writefd);
    }
  }
}

#ifdef HAVE_LIBCARES

const std::vector<AsyncNameResolverSocketEntry>&
AsyncNameResolver::getsock() const
{
  return socks_;
}

void AsyncNameResolver::process(ares_socket_t readfd, ares_socket_t writefd)
{
  ares_process_fd(channel_, readfd, writefd);
}

#endif // HAVE_LIBCARES

bool AsyncNameResolver::operator==(const AsyncNameResolver& resolver) const
{
  return this == &resolver;
}

} // namespace aria2
