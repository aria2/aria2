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

const std::string AuthConfigFactory::ANONYMOUS("anonymous");

const std::string AuthConfigFactory::ARIA2USER_AT("ARIA2USER@");

AuthConfigFactory::AuthConfigFactory() {}

AuthConfigFactory::~AuthConfigFactory() {}

AuthConfigHandle
AuthConfigFactory::createAuthConfig
(const SharedHandle<Request>& request, const Option* op)
{
  if(request->getProtocol() == Request::PROTO_HTTP ||
     request->getProtocol() == Request::PROTO_HTTPS) {

    if(op->getAsBool(PREF_HTTP_AUTH_CHALLENGE)) {
      if(!request->getUsername().empty()) {
        SharedHandle<BasicCred> bc(new BasicCred(request->getUsername(),
                                                 request->getPassword(),
                                                 request->getHost(),
                                                 request->getPort(),
                                                 request->getDir(), true));
        updateBasicCred(bc);
        return createAuthConfig(request->getUsername(), request->getPassword());
      }
      BasicCredSet::iterator i =
        findBasicCred(request->getHost(), request->getPort(),
                      request->getDir());
      if(i == basicCreds_.end()) {
        return SharedHandle<AuthConfig>();
      } else {
        return createAuthConfig((*i)->user_, (*i)->password_);
      }
    } else {
      if(!request->getUsername().empty()) {
        return createAuthConfig(request->getUsername(), request->getPassword());
      } else {
        return
          createHttpAuthResolver(op)->resolveAuthConfig(request->getHost());
      }
    }
  } else if(request->getProtocol() == Request::PROTO_FTP) {
    if(!request->getUsername().empty()) {
      if(request->hasPassword()) {
        return createAuthConfig(request->getUsername(), request->getPassword());
      } else {
        if(!op->getAsBool(PREF_NO_NETRC)) {
          // First, check we have password corresponding to host and
          // username
          NetrcAuthResolver authResolver;
          authResolver.setNetrc(netrc_);

          SharedHandle<AuthConfig> ac =
            authResolver.resolveAuthConfig(request->getHost());
          if(ac && ac->getUser() == request->getUsername()) {
            return ac;
          }
        }
        // We don't have password for host and username. Return
        // password specified by --ftp-passwd
        return
          createAuthConfig(request->getUsername(), op->get(PREF_FTP_PASSWD));
      }
    } else {
      return
        createFtpAuthResolver(op)->resolveAuthConfig(request->getHost());
    }
  } else {
    return SharedHandle<AuthConfig>();
  }
}

AuthConfigHandle
AuthConfigFactory::createAuthConfig(const std::string& user, const std::string& password) const
{
  SharedHandle<AuthConfig> ac;
  if(!user.empty()) {
    ac.reset(new AuthConfig(user, password));
  }
  return ac;
}

AuthResolverHandle AuthConfigFactory::createHttpAuthResolver
(const Option* op) const
{
  AbstractAuthResolverHandle resolver;
  if(op->getAsBool(PREF_NO_NETRC)) {
    resolver.reset(new DefaultAuthResolver());
  } else {
    NetrcAuthResolverHandle authResolver(new NetrcAuthResolver());
    authResolver->setNetrc(netrc_);
    authResolver->ignoreDefault();
    resolver = authResolver;
  }
  resolver->setUserDefinedAuthConfig
    (createAuthConfig(op->get(PREF_HTTP_USER), op->get(PREF_HTTP_PASSWD)));
  return resolver;
}

AuthResolverHandle AuthConfigFactory::createFtpAuthResolver
(const Option* op) const
{
  AbstractAuthResolverHandle resolver;
  if(op->getAsBool(PREF_NO_NETRC)) {
    resolver.reset(new DefaultAuthResolver());
  } else {
    NetrcAuthResolverHandle authResolver(new NetrcAuthResolver());
    authResolver->setNetrc(netrc_);
    resolver = authResolver;
  }
  resolver->setUserDefinedAuthConfig
    (createAuthConfig(op->get(PREF_FTP_USER), op->get(PREF_FTP_PASSWD)));
  SharedHandle<AuthConfig> defaultAuthConfig
    (new AuthConfig(AuthConfigFactory::ANONYMOUS,
                    AuthConfigFactory::ARIA2USER_AT));
  resolver->setDefaultAuthConfig(defaultAuthConfig);
  return resolver;
}

void AuthConfigFactory::setNetrc(const SharedHandle<Netrc>& netrc)
{
  netrc_ = netrc;
}

void AuthConfigFactory::updateBasicCred
(const SharedHandle<BasicCred>& basicCred)
{
  BasicCredSet::iterator i = basicCreds_.lower_bound(basicCred);
  if(i != basicCreds_.end() && *(*i) == *basicCred) {
    *(*i) = *basicCred;
  } else {
    basicCreds_.insert(i, basicCred);
  }
}

bool AuthConfigFactory::activateBasicCred
(const std::string& host,
 uint16_t port,
 const std::string& path,
 const Option* op)
{
  BasicCredSet::iterator i = findBasicCred(host, port, path);
  if(i == basicCreds_.end()) {
    SharedHandle<AuthConfig> authConfig =
      createHttpAuthResolver(op)->resolveAuthConfig(host);
    if(!authConfig) {
      return false;
    } else {
      SharedHandle<BasicCred> bc
        (new BasicCred(authConfig->getUser(), authConfig->getPassword(),
                       host, port, path, true));
      basicCreds_.insert(bc);
      return true;
    }
  } else {
    (*i)->activate();
    return true;
  }
}

AuthConfigFactory::BasicCred::BasicCred
(const std::string& user, const std::string& password,
 const std::string& host, uint16_t port, const std::string& path,
 bool activated):
  user_(user), password_(password),
  host_(host), port_(port), path_(path), activated_(activated)
{
  if(path_.empty() || path_[path_.size()-1] != '/') {
    path_ += "/";
  }
}

void AuthConfigFactory::BasicCred::activate()
{
  activated_ = true;
}

bool AuthConfigFactory::BasicCred::isActivated() const
{
  return activated_;
}

bool AuthConfigFactory::BasicCred::operator==(const BasicCred& cred) const
{
  return host_ == cred.host_ && port_ == cred.port_ && path_ == cred.path_;
}

bool AuthConfigFactory::BasicCred::operator<(const BasicCred& cred) const
{
  return host_ < cred.host_ ||
    (!(cred.host_ < host_) && (port_ < cred.port_ ||
                               (!(cred.port_ < port_) && path_ > cred.path_)));
}

AuthConfigFactory::BasicCredSet::iterator
AuthConfigFactory::findBasicCred
(const std::string& host,
 uint16_t port,
 const std::string& path)
{
  SharedHandle<BasicCred> bc(new BasicCred("", "", host, port, path));
  BasicCredSet::iterator i = basicCreds_.lower_bound(bc);
  for(; i != basicCreds_.end() && (*i)->host_ == host && (*i)->port_ == port;
      ++i) {
    if(util::startsWith(bc->path_, (*i)->path_)) {
      return i;
    }
  }
  return basicCreds_.end();
}

} // namespace aria2
