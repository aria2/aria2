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

namespace aria2 {

void callback(void* arg, int status, int timeouts, struct hostent* host)
{
  AsyncNameResolver* resolverPtr = reinterpret_cast<AsyncNameResolver*>(arg);
  if (status != ARES_SUCCESS) {
    resolverPtr->error_ = ares_strerror(status);
    resolverPtr->status_ = AsyncNameResolver::STATUS_ERROR;
    return;
  }
  for (char** ap = host->h_addr_list; *ap; ++ap) {
    char addrstring[NI_MAXHOST];
    if (inetNtop(host->h_addrtype, *ap, addrstring, sizeof(addrstring)) == 0) {
      resolverPtr->resolvedAddresses_.push_back(addrstring);
    }
  }
  if (resolverPtr->resolvedAddresses_.empty()) {
    resolverPtr->error_ = "no address returned or address conversion failed";
    resolverPtr->status_ = AsyncNameResolver::STATUS_ERROR;
  }
  else {
    resolverPtr->status_ = AsyncNameResolver::STATUS_SUCCESS;
  }
}

AsyncNameResolver::AsyncNameResolver(int family
#ifdef HAVE_ARES_ADDR_NODE
                                     ,
                                     ares_addr_node* servers
#endif // HAVE_ARES_ADDR_NODE
                                     )
    : status_(STATUS_READY), family_(family)
{
  // TODO evaluate return value
  ares_init(&channel_);
#if defined(HAVE_ARES_SET_SERVERS) && defined(HAVE_ARES_ADDR_NODE)
  if (servers) {
    // ares_set_servers has been added since c-ares 1.7.1
    if (ares_set_servers(channel_, servers) != ARES_SUCCESS) {
      A2_LOG_DEBUG("ares_set_servers failed");
    }
  }
#endif // HAVE_ARES_SET_SERVERS && HAVE_ARES_ADDR_NODE
}

AsyncNameResolver::~AsyncNameResolver() { ares_destroy(channel_); }

void AsyncNameResolver::resolve(const std::string& name)
{
  hostname_ = name;
  status_ = STATUS_QUERYING;
  ares_gethostbyname(channel_, name.c_str(), family_, callback, this);
}

int AsyncNameResolver::getFds(fd_set* rfdsPtr, fd_set* wfdsPtr) const
{
  return ares_fds(channel_, rfdsPtr, wfdsPtr);
}

void AsyncNameResolver::process(fd_set* rfdsPtr, fd_set* wfdsPtr)
{
  ares_process(channel_, rfdsPtr, wfdsPtr);
}

#ifdef HAVE_LIBCARES

int AsyncNameResolver::getsock(sock_t* sockets) const
{
  return ares_getsock(channel_, reinterpret_cast<ares_socket_t*>(sockets),
                      ARES_GETSOCK_MAXNUM);
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

void AsyncNameResolver::reset()
{
  hostname_ = A2STR::NIL;
  resolvedAddresses_.clear();
  status_ = STATUS_READY;
  ares_destroy(channel_);
  // TODO evaluate return value
  ares_init(&channel_);
}

#ifdef HAVE_ARES_ADDR_NODE

ares_addr_node* parseAsyncDNSServers(const std::string& serversOpt)
{
  std::vector<std::string> servers;
  util::split(std::begin(serversOpt), std::end(serversOpt),
              std::back_inserter(servers), ',', true /* doStrip */);
  ares_addr_node root;
  root.next = nullptr;
  ares_addr_node* tail = &root;
  for (const auto& s : servers) {
    auto node = make_unique<ares_addr_node>();

    size_t len = net::getBinAddr(&node->addr, s.c_str());
    if (len != 0) {
      node->next = nullptr;
      node->family = (len == 4 ? AF_INET : AF_INET6);
      tail->next = node.release();
      tail = tail->next;
    }
  }

  return root.next;
}

#endif // HAVE_ARES_ADDR_NODE

} // namespace aria2
