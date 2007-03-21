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
#include "RequestFactory.h"
#include "prefs.h"
#include "NetrcAuthResolver.h"

RequestHandle RequestFactory::createRequest()
{
  RequestHandle request = new Request();
  request->setMethod(_method);
  request->setReferer(_referer);
  request->setHttpAuthResolver(createHttpAuthResolver());
  request->setHttpProxyAuthResolver(createHttpProxyAuthResolver());
  request->setFtpAuthResolver(createFtpAuthResolver());
  return request;
}

AuthConfigHandle RequestFactory::createAuthConfig(const string& user, const string& password) const
{
  if(user.length() > 0) {
    return new AuthConfig(user, password);
  } else {
    return 0;
  }
}

AuthResolverHandle RequestFactory::createHttpAuthResolver()
{
  NetrcAuthResolverHandle authResolver = 0;
  authResolver = new NetrcAuthResolver();
  authResolver->setNetrc(_netrc);
  authResolver->setUserDefinedAuthConfig(createAuthConfig(_option->get(PREF_HTTP_USER), _option->get(PREF_HTTP_PASSWD)));
  return authResolver;		     
}

AuthResolverHandle RequestFactory::createFtpAuthResolver()
{
  NetrcAuthResolverHandle authResolver = 0;
  authResolver = new NetrcAuthResolver();
  authResolver->setNetrc(_netrc);
  authResolver->setUserDefinedAuthConfig(createAuthConfig(_option->get(PREF_FTP_USER), _option->get(PREF_FTP_PASSWD)));
  authResolver->setDefaultAuthConfig(new AuthConfig("anonymous",
							    "ARIA2USER@"));
  return authResolver;
}

AuthResolverHandle RequestFactory::createHttpProxyAuthResolver()
{
  NetrcAuthResolverHandle authResolver = 0;
  authResolver = new NetrcAuthResolver();
  authResolver->setNetrc(_netrc);
  authResolver->setUserDefinedAuthConfig(createAuthConfig(_option->get(PREF_HTTP_PROXY_USER), _option->get(PREF_HTTP_PROXY_PASSWD)));
  return authResolver;
}
