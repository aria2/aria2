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

#include <algorithm>

#include "Option.h"
#include "AuthConfig.h"
#include "Netrc.h"
#include "DefaultAuthResolver.h"
#include "NetrcAuthResolver.h"
#include "prefs.h"
#include "Request.h"
#include "Util.h"

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
	// TODO setting "/" as path. Should we use request->getDir() instead?
	updateBasicCred(BasicCred(request->getUsername(), request->getPassword(),
				  request->getHost(), "/", true));
	return createAuthConfig(request->getUsername(), request->getPassword());
      }
      std::deque<BasicCred>::const_iterator i =
	findBasicCred(request->getHost(), request->getDir());
      if(i == _basicCreds.end()) {
	return SharedHandle<AuthConfig>();
      } else {
	return createAuthConfig((*i)._user, (*i)._password);
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
      return createAuthConfig(request->getUsername(), request->getPassword());
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
    authResolver->setNetrc(_netrc);
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
    authResolver->setNetrc(_netrc);
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

void AuthConfigFactory::setNetrc(const NetrcHandle& netrc)
{
  _netrc = netrc;
}

void AuthConfigFactory::updateBasicCred(const BasicCred& basicCred)
{
  std::deque<BasicCred>::iterator i =
    std::lower_bound(_basicCreds.begin(), _basicCreds.end(), basicCred);

  if(i != _basicCreds.end() && (*i) == basicCred) {
    (*i) = basicCred;
  } else {
    _basicCreds.insert(i, basicCred);
  }
}

bool AuthConfigFactory::activateBasicCred
(const std::string& host, const std::string& path, const Option* op)
{

  std::deque<BasicCred>::iterator i = findBasicCred(host, path);
  if(i == _basicCreds.end()) {
    SharedHandle<AuthConfig> authConfig =
      createHttpAuthResolver(op)->resolveAuthConfig(host);
    if(authConfig.isNull()) {
      return false;
    } else {
      BasicCred bc(authConfig->getUser(), authConfig->getPassword(),
		   host, path, true);
      i = std::lower_bound(_basicCreds.begin(), _basicCreds.end(), bc);
      _basicCreds.insert(i, bc);
      return true;
    }
  } else {
    (*i).activate();
    return true;
  }
}

AuthConfigFactory::BasicCred::BasicCred
(const std::string& user, const std::string& password,
 const std::string& host, const std::string& path,
 bool activated):
  _user(user), _password(password),
  _host(host), _path(path), _activated(activated)
{
  if(!Util::endsWith(_path, "/")) {
    _path += "/";
  }
}

void AuthConfigFactory::BasicCred::activate()
{
  _activated = true;
}

bool AuthConfigFactory::BasicCred::isActivated() const
{
  return _activated;
}

bool AuthConfigFactory::BasicCred::operator==(const BasicCred& cred) const
{
  return _host == cred._host && _path == cred._path;
}

bool AuthConfigFactory::BasicCred::operator<(const BasicCred& cred) const
{
  int c = _host.compare(cred._host);
  if(c == 0) {
    return _path > cred._path;
  } else {
    return c < 0;
  }
}

std::deque<AuthConfigFactory::BasicCred>::iterator
AuthConfigFactory::findBasicCred(const std::string& host,
				 const std::string& path)
{
  BasicCred bc("", "", host, path);
  std::deque<BasicCred>::iterator i =
    std::lower_bound(_basicCreds.begin(), _basicCreds.end(), bc);
  for(; i != _basicCreds.end() && (*i)._host == host; ++i) {
    if(Util::startsWith(bc._path, (*i)._path)) {
      return i;
    }
  }
  return _basicCreds.end();
}

} // namespace aria2
