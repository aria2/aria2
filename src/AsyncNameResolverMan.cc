/* <!-- copyright */
/*
 * aria2 - The high speed download utility
 *
 * Copyright (C) 2013 Tatsuhiro Tsujikawa
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
#include "AsyncNameResolverMan.h"

#include <cassert>

#include "AsyncNameResolver.h"
#include "DownloadEngine.h"
#include "Command.h"
#include "message.h"
#include "fmt.h"
#include "LogFactory.h"
#include "Option.h"
#include "SocketCore.h"
#include "prefs.h"

namespace aria2 {

AsyncNameResolverMan::AsyncNameResolverMan()
    : numResolver_(0), resolverCheck_(0), ipv4_(true), ipv6_(true)
{
}

AsyncNameResolverMan::~AsyncNameResolverMan() { assert(!resolverCheck_); }

bool AsyncNameResolverMan::started() const
{
  for (size_t i = 0; i < numResolver_; ++i) {
    if (asyncNameResolver_[i]) {
      return true;
    }
  }
  return false;
}

void AsyncNameResolverMan::startAsync(const std::string& hostname,
                                      DownloadEngine* e, Command* command)
{
  numResolver_ = 0;
  // Set IPv6 resolver first, so that we can push IPv6 address in
  // front of IPv6 address in getResolvedAddress().
  if (ipv6_) {
    startAsyncFamily(hostname, AF_INET6, e, command);
    ++numResolver_;
  }
  if (ipv4_) {
    startAsyncFamily(hostname, AF_INET, e, command);
    ++numResolver_;
  }
  A2_LOG_INFO(
      fmt(MSG_RESOLVING_HOSTNAME, command->getCuid(), hostname.c_str()));
}

void AsyncNameResolverMan::startAsyncFamily(const std::string& hostname,
                                            int family, DownloadEngine* e,
                                            Command* command)
{
  asyncNameResolver_[numResolver_] =
      std::make_shared<AsyncNameResolver>(family
#ifdef HAVE_ARES_ADDR_NODE
                                          ,
                                          e->getAsyncDNSServers()
#endif // HAVE_ARES_ADDR_NODE
      );
  asyncNameResolver_[numResolver_]->resolve(hostname);
  setNameResolverCheck(numResolver_, e, command);
}

void AsyncNameResolverMan::getResolvedAddress(
    std::vector<std::string>& res) const
{
  for (size_t i = 0; i < numResolver_; ++i) {
    if (asyncNameResolver_[i]->getStatus() ==
        AsyncNameResolver::STATUS_SUCCESS) {
      auto& addrs = asyncNameResolver_[i]->getResolvedAddresses();
      res.insert(std::end(res), std::begin(addrs), std::end(addrs));
    }
  }
  return;
}

void AsyncNameResolverMan::setNameResolverCheck(DownloadEngine* e,
                                                Command* command)
{
  for (size_t i = 0; i < numResolver_; ++i) {
    setNameResolverCheck(i, e, command);
  }
}

void AsyncNameResolverMan::setNameResolverCheck(size_t index, DownloadEngine* e,
                                                Command* command)
{
  if (asyncNameResolver_[index]) {
    assert((resolverCheck_ & (1 << index)) == 0);
    resolverCheck_ |= 1 << index;
    e->addNameResolverCheck(asyncNameResolver_[index], command);
  }
}

void AsyncNameResolverMan::disableNameResolverCheck(DownloadEngine* e,
                                                    Command* command)
{
  for (size_t i = 0; i < numResolver_; ++i) {
    disableNameResolverCheck(i, e, command);
  }
}

void AsyncNameResolverMan::disableNameResolverCheck(size_t index,
                                                    DownloadEngine* e,
                                                    Command* command)
{
  if (asyncNameResolver_[index] && (resolverCheck_ & (1 << index))) {
    resolverCheck_ &= ~(1 << index);
    e->deleteNameResolverCheck(asyncNameResolver_[index], command);
  }
}

int AsyncNameResolverMan::getStatus() const
{
  size_t success = 0;
  size_t error = 0;
  bool ipv4Success = false;
  for (size_t i = 0; i < numResolver_; ++i) {
    switch (asyncNameResolver_[i]->getStatus()) {
    case AsyncNameResolver::STATUS_SUCCESS:
      ++success;
      if (asyncNameResolver_[i]->getFamily() == AF_INET) {
        ipv4Success = true;
      }
      break;
    case AsyncNameResolver::STATUS_ERROR:
      ++error;
      break;
    default:
      break;
    }
  }
  // If we got a IPv4 lookup response, we don't wait for a IPv6 lookup
  // response. This is because DNS servers may drop AAAA queries and we
  // have to wait for a long time before timeout. We don't do the
  // inverse, because, based on today's deployment of DNS servers,
  // almost all of them can respond to A queries just fine.
  if ((success && ipv4Success) || success == numResolver_) {
    return 1;
  }
  else if (error == numResolver_) {
    return -1;
  }
  else {
    return 0;
  }
}

const std::string& AsyncNameResolverMan::getLastError() const
{
  for (size_t i = 0; i < numResolver_; ++i) {
    if (asyncNameResolver_[i]->getStatus() == AsyncNameResolver::STATUS_ERROR) {
      // TODO This is not last error chronologically.
      return asyncNameResolver_[i]->getError();
    }
  }
  return A2STR::NIL;
}

void AsyncNameResolverMan::reset(DownloadEngine* e, Command* command)
{
  disableNameResolverCheck(e, command);
  assert(resolverCheck_ == 0);
  for (size_t i = 0; i < numResolver_; ++i) {
    asyncNameResolver_[i].reset();
  }
  numResolver_ = 0;
}

void configureAsyncNameResolverMan(AsyncNameResolverMan* asyncNameResolverMan,
                                   Option* option)
{
  // Currently, aria2 checks configured addresses at the startup. But
  // there are chances that interfaces are not setup at that
  // moment. For example, if aria2 is used as daemon, it may start
  // before network interfaces up. To workaround this, we check
  // addresses again if both addresses are not configured at the
  // startup.
  if (!net::getIPv4AddrConfigured() && !net::getIPv6AddrConfigured()) {
    net::checkAddrconfig();
  }
  if (!net::getIPv4AddrConfigured()) {
    asyncNameResolverMan->setIPv4(false);
  }
  if (!net::getIPv6AddrConfigured() || option->getAsBool(PREF_DISABLE_IPV6)) {
    asyncNameResolverMan->setIPv6(false);
  }
}

} // namespace aria2
