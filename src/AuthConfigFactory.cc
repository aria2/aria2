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
#include "AuthConfigFactory.h"
#include "Option.h"
#include "AuthConfig.h"
#include "Netrc.h"
#include "DefaultAuthResolver.h"
#include "NetrcAuthResolver.h"
#include "prefs.h"
#include "Request.h"

namespace aria2 {

AuthConfigFactory::AuthConfigFactory(const Option* option):
  _option(option), _netrc(0) {}

AuthConfigFactory::~AuthConfigFactory() {}

AuthConfigHandle AuthConfigFactory::createAuthConfig(const RequestHandle& request) const
{
  if(request->getProtocol() == "http" || request->getProtocol() == "https") {
    return createHttpAuthResolver()->resolveAuthConfig(request->getHost());
  } else if(request->getProtocol() == "ftp") {
    if(!request->getUsername().empty()) {
      return createAuthConfig(request->getUsername(), request->getPassword());
    } else {
      return createFtpAuthResolver()->resolveAuthConfig(request->getHost());
    }
  } else {
    return new AuthConfig();
  }
}

AuthConfigHandle AuthConfigFactory::createAuthConfigForHttpProxy(const RequestHandle& request) const
{
  return createHttpProxyAuthResolver()->resolveAuthConfig(request->getHost());
}

AuthConfigHandle AuthConfigFactory::createAuthConfig(const std::string& user, const std::string& password) const
{
  if(user.length() > 0) {
    return new AuthConfig(user, password);
  } else {
    return 0;
  }
}

AuthResolverHandle AuthConfigFactory::createHttpAuthResolver() const
{
  AbstractAuthResolverHandle resolver = 0;
  if(true || _option->getAsBool(PREF_NO_NETRC)) {
    resolver = new DefaultAuthResolver();
  } else {
    NetrcAuthResolverHandle authResolver = new NetrcAuthResolver();
    authResolver->setNetrc(_netrc);
    resolver = authResolver;
  }
  resolver->setUserDefinedAuthConfig(createAuthConfig(_option->get(PREF_HTTP_USER), _option->get(PREF_HTTP_PASSWD)));
  return resolver;
}

AuthResolverHandle AuthConfigFactory::createFtpAuthResolver() const
{
  AbstractAuthResolverHandle resolver = 0;
  if(_option->getAsBool(PREF_NO_NETRC)) {
    resolver = new DefaultAuthResolver();
  } else {
    NetrcAuthResolverHandle authResolver = new NetrcAuthResolver();
    authResolver->setNetrc(_netrc);
    resolver = authResolver;
  }
  resolver->setUserDefinedAuthConfig(createAuthConfig(_option->get(PREF_FTP_USER), _option->get(PREF_FTP_PASSWD)));
  resolver->setDefaultAuthConfig(new AuthConfig("anonymous", "ARIA2USER@"));
  return resolver;
}

AuthResolverHandle AuthConfigFactory::createHttpProxyAuthResolver() const
{
  AbstractAuthResolverHandle resolver = 0;
  if(true || _option->getAsBool(PREF_NO_NETRC)) {
    resolver = new DefaultAuthResolver();
  } else {
    NetrcAuthResolverHandle authResolver = new NetrcAuthResolver();
    authResolver->setNetrc(_netrc);
    resolver = authResolver;
  }
  resolver->setUserDefinedAuthConfig(createAuthConfig(_option->get(PREF_HTTP_PROXY_USER), _option->get(PREF_HTTP_PROXY_PASSWD)));
  return resolver;
}

void AuthConfigFactory::setNetrc(const NetrcHandle& netrc)
{
  _netrc = netrc;
}

} // namespace aria2
