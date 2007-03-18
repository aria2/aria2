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
#include "Request.h"
#include "Util.h"
#include "FeatureConfig.h"
#include "Netrc.h"

const string Request::METHOD_GET = "get";

const string Request::METHOD_HEAD = "head";

Request::Request():port(0), tryCount(0), keepAlive(true), method(METHOD_GET),
		   _userDefinedAuthConfig(0),
		   isTorrent(false)
{
  cookieBox = new CookieBox();
}

Request::~Request() {
  delete cookieBox;
}

bool Request::setUrl(const string& url) {
  this->url = url;
  return parseUrl(url);
}

bool Request::resetUrl() {
  previousUrl = referer;
  return setUrl(url);
}

bool Request::redirectUrl(const string& url) {
  previousUrl = currentUrl;
  return parseUrl(url);
}

bool Request::parseUrl(const string& url) {
  string tempUrl;
  string::size_type sharpIndex = url.find("#");
  if(sharpIndex != string::npos) {
    if(FeatureConfig::getInstance()->isSupported("metalink") &&
       url.find(METALINK_MARK) == sharpIndex) {
      tempUrl = url.substr(sharpIndex+strlen(METALINK_MARK));
    } else {
      tempUrl = url.substr(0, sharpIndex);
    }
  } else {
    tempUrl = url;
  }

  currentUrl = tempUrl;
  string query;
  host = "";
  port = 0;
  dir = "";
  file = "";
  if(tempUrl.find_first_not_of(SAFE_CHARS) != string::npos) {
    return false;
  }
  string::size_type startQueryIndex = tempUrl.find("?");
  if(startQueryIndex != string::npos) {
    query = tempUrl.substr(startQueryIndex);
    tempUrl.erase(startQueryIndex);
  }
  string::size_type hp = tempUrl.find("://");
  if(hp == string::npos) return false;
  protocol = tempUrl.substr(0, hp);
  int defPort;
  if((defPort = FeatureConfig::getInstance()->getDefaultPort(protocol)) == 0) {
    return false;
  }
  hp += 3;
  if(tempUrl.size() <= hp) return false;
  string::size_type hep = tempUrl.find("/", hp);
  if(hep == string::npos) {
    hep = tempUrl.size();
  }
  pair<string, string> hostAndPort;
  Util::split(hostAndPort, tempUrl.substr(hp, hep-hp), ':');
  host = hostAndPort.first;
  if(hostAndPort.second != "") {
    port = (int)strtol(hostAndPort.second.c_str(), NULL, 10);
    if(!(0 < port && port <= 65535)) {
      return false;
    }
  } else {
    // If port is not specified, then we set it to default port of its protocol..
    port = defPort;
  }
  string::size_type direp = tempUrl.find_last_of("/");
  if(direp == string::npos || direp <= hep) {
    dir = "/";
    direp = hep;
  } else {
    dir = tempUrl.substr(hep, direp-hep);
  }
  if(tempUrl.size() > direp+1) {
    file = tempUrl.substr(direp+1);
  }
  file += query;
  return true;
}

AuthConfigItemHandle Request::findNetrcAuthenticator() const
{
  if(!NetrcSingletonHolder::instance().isNull()) {
    AuthenticatorHandle auth = NetrcSingletonHolder::instance()->findAuthenticator(getHost());
    if(auth.isNull()) {
      return 0;
    } else {
      return new AuthConfigItem(auth->getLogin(), auth->getPassword());
    }
  } else {
    return 0;
  }
}

AuthConfigItemHandle Request::resolveHttpAuthConfigItem() const
{
  if(!_userDefinedAuthConfig.isNull() &&
     !_userDefinedAuthConfig->getHttpAuthConfigItem().isNull()) {
    return _userDefinedAuthConfig->getHttpAuthConfigItem();
  } else {
    return findNetrcAuthenticator();
  }
}

AuthConfigItemHandle Request::resolveFtpAuthConfigItem() const
{
  if(!_userDefinedAuthConfig.isNull() &&
     !_userDefinedAuthConfig->getFtpAuthConfigItem().isNull()) {
    return _userDefinedAuthConfig->getFtpAuthConfigItem();
  } else {
    AuthConfigItemHandle authConfig = findNetrcAuthenticator();
    if(authConfig.isNull()) {
      return new AuthConfigItem("anonymous", "ARIA2USER@");
    } else {
      return authConfig;
    }
  }
}

AuthConfigItemHandle Request::resolveHttpProxyAuthConfigItem() const
{
  if(!_userDefinedAuthConfig.isNull() &&
     !_userDefinedAuthConfig->getHttpProxyAuthConfigItem().isNull()) {
    return _userDefinedAuthConfig->getHttpProxyAuthConfigItem();
  } else {
    return findNetrcAuthenticator();
  }
}
