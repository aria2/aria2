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
#include "AuthConfigFactory.h"

#include <algorithm>

#include "Option.h"
#include "AuthConfig.h"
#include "Netrc.h"
#include "DefaultAuthResolver.h"
#include "NetrcAuthResolver.h"
#include "prefs.h"
#include "Request.h"
#include "util.h"

namespace aria2 {

namespace {
const std::string AUTH_DEFAULT_USER("anonymous");
const std::string AUTH_DEFAULT_PASSWD("ARIA2USER@");
} // namespace

AuthConfigFactory::AuthConfigFactory() {}

AuthConfigFactory::~AuthConfigFactory() = default;

std::unique_ptr<AuthConfig>
AuthConfigFactory::createAuthConfig(const std::shared_ptr<Request>& request,
                                    const Option* op)
{
  if (request->getProtocol() == "http" || request->getProtocol() == "https") {
    if (op->getAsBool(PREF_HTTP_AUTH_CHALLENGE)) {
      if (!request->getUsername().empty()) {
        updateBasicCred(make_unique<BasicCred>(
            request->getUsername(), request->getPassword(), request->getHost(),
            request->getPort(), request->getDir(), true));
        return AuthConfig::create(request->getUsername(),
                                  request->getPassword());
      }
      auto i = findBasicCred(request->getHost(), request->getPort(),
                             request->getDir());
      if (i == std::end(basicCreds_)) {
        return nullptr;
      }
      else {
        return AuthConfig::create((*i)->user_, (*i)->password_);
      }
    }
    else {
      if (!request->getUsername().empty()) {
        return AuthConfig::create(request->getUsername(),
                                  request->getPassword());
      }
      else {
        return createHttpAuthResolver(op)->resolveAuthConfig(
            request->getHost());
      }
    }
  }
  else if (request->getProtocol() == "ftp" ||
           request->getProtocol() == "sftp") {
    if (!request->getUsername().empty()) {
      if (request->hasPassword()) {
        return AuthConfig::create(request->getUsername(),
                                  request->getPassword());
      }
      else {
        if (!op->getAsBool(PREF_NO_NETRC)) {
          // First, check we have password corresponding to host and
          // username
          NetrcAuthResolver authResolver;
          authResolver.setNetrc(netrc_.get());

          auto ac = authResolver.resolveAuthConfig(request->getHost());
          if (ac && ac->getUser() == request->getUsername()) {
            return ac;
          }
        }
        // We don't have password for host and username. Return
        // password specified by --ftp-passwd
        return AuthConfig::create(request->getUsername(),
                                  op->get(PREF_FTP_PASSWD));
      }
    }
    else {
      return createFtpAuthResolver(op)->resolveAuthConfig(request->getHost());
    }
  }
  else {
    return nullptr;
  }
}

std::unique_ptr<AuthResolver>
AuthConfigFactory::createHttpAuthResolver(const Option* op) const
{
  std::unique_ptr<AbstractAuthResolver> resolver;
  if (op->getAsBool(PREF_NO_NETRC)) {
    resolver = make_unique<DefaultAuthResolver>();
  }
  else {
    auto authResolver = make_unique<NetrcAuthResolver>();
    authResolver->setNetrc(netrc_.get());
    authResolver->ignoreDefault();
    resolver = std::move(authResolver);
  }
  resolver->setUserDefinedCred(op->get(PREF_HTTP_USER),
                               op->get(PREF_HTTP_PASSWD));
  return std::move(resolver);
}

std::unique_ptr<AuthResolver>
AuthConfigFactory::createFtpAuthResolver(const Option* op) const
{
  std::unique_ptr<AbstractAuthResolver> resolver;
  if (op->getAsBool(PREF_NO_NETRC)) {
    resolver = make_unique<DefaultAuthResolver>();
  }
  else {
    auto authResolver = make_unique<NetrcAuthResolver>();
    authResolver->setNetrc(netrc_.get());
    resolver = std::move(authResolver);
  }
  resolver->setUserDefinedCred(op->get(PREF_FTP_USER),
                               op->get(PREF_FTP_PASSWD));
  resolver->setDefaultCred(AUTH_DEFAULT_USER, AUTH_DEFAULT_PASSWD);
  return std::move(resolver);
}

void AuthConfigFactory::setNetrc(std::unique_ptr<Netrc> netrc)
{
  netrc_ = std::move(netrc);
}

void AuthConfigFactory::updateBasicCred(std::unique_ptr<BasicCred> basicCred)
{
  auto i = basicCreds_.lower_bound(basicCred);
  if (i != std::end(basicCreds_) && *i == basicCred) {
    *(*i) = std::move(*basicCred);
  }
  else {
    basicCreds_.insert(i, std::move(basicCred));
  }
}

bool AuthConfigFactory::activateBasicCred(const std::string& host,
                                          uint16_t port,
                                          const std::string& path,
                                          const Option* op)
{
  auto i = findBasicCred(host, port, path);
  if (i == std::end(basicCreds_)) {
    auto authConfig = createHttpAuthResolver(op)->resolveAuthConfig(host);
    if (!authConfig) {
      return false;
    }
    else {
      basicCreds_.insert(make_unique<BasicCred>(authConfig->getUser(),
                                                authConfig->getPassword(), host,
                                                port, path, true));
      return true;
    }
  }
  else {
    (*i)->activate();
    return true;
  }
}

BasicCred::BasicCred(std::string user, std::string password, std::string host,
                     uint16_t port, std::string path, bool activated)
    : user_(std::move(user)),
      password_(std::move(password)),
      host_(std::move(host)),
      port_(port),
      path_(std::move(path)),
      activated_(activated)
{
  if (path_.empty() || path_[path_.size() - 1] != '/') {
    path_ += "/";
  }
}

void BasicCred::activate() { activated_ = true; }

bool BasicCred::isActivated() const { return activated_; }

bool BasicCred::operator==(const BasicCred& cred) const
{
  return host_ == cred.host_ && port_ == cred.port_ && path_ == cred.path_;
}

bool BasicCred::operator<(const BasicCred& cred) const
{
  return host_ < cred.host_ || (!(cred.host_ < host_) &&
                                (port_ < cred.port_ || (!(cred.port_ < port_) &&
                                                        path_ > cred.path_)));
}

AuthConfigFactory::BasicCredSet::iterator
AuthConfigFactory::findBasicCred(const std::string& host, uint16_t port,
                                 const std::string& path)
{
  auto bc = make_unique<BasicCred>("", "", host, port, path);
  auto i = basicCreds_.lower_bound(bc);
  for (;
       i != std::end(basicCreds_) && (*i)->host_ == host && (*i)->port_ == port;
       ++i) {
    if (util::startsWith(bc->path_, (*i)->path_)) {
      return i;
    }
  }
  return std::end(basicCreds_);
}

} // namespace aria2
