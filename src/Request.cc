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
#include "CookieBoxFactory.h"

const string Request::METHOD_GET = "get";

const string Request::METHOD_HEAD = "head";

Request::Request():port(0), tryCount(0), keepAlive(false), method(METHOD_GET),
		   cookieBox(CookieBoxFactorySingletonHolder::instance()->createNewInstance()) {}

Request::~Request() {}

bool Request::setUrl(const string& url) {
  this->url = url;
  return parseUrl(url);
}

bool Request::resetUrl() {
  previousUrl = referer;
  return setUrl(url);
}

bool Request::redirectUrl(const string& url) {
  previousUrl = "";
  keepAlive = false;
  return parseUrl(url);
}

bool Request::parseUrl(const string& url) {
  string tempUrl;
  string::size_type sharpIndex = url.find("#");
  if(sharpIndex != string::npos) {
    tempUrl = urlencode(url.substr(0, sharpIndex));
  } else {
    tempUrl = urlencode(url);
  }
  currentUrl = tempUrl;
  string query;
  host = "";
  port = 0;
  dir = "";
  file = "";
  _username = "";
  _password = "";
  // find query part
  string::size_type startQueryIndex = tempUrl.find("?");
  if(startQueryIndex != string::npos) {
    query = tempUrl.substr(startQueryIndex);
    tempUrl.erase(startQueryIndex);
  }
  // find protocol
  string::size_type hp = tempUrl.find("://");
  if(hp == string::npos) return false;
  protocol = tempUrl.substr(0, hp);
  int32_t defPort;
  if((defPort = FeatureConfig::getInstance()->getDefaultPort(protocol)) == 0) {
    return false;
  }
  hp += 3;
  // find host part
  if(tempUrl.size() <= hp) return false;
  string::size_type hep = tempUrl.find("/", hp);
  if(hep == string::npos) {
    hep = tempUrl.size();
  }
  string hostPart = tempUrl.substr(hp, hep-hp);
  //   find username and password in host part if they exist
  string::size_type atmarkp =  hostPart.find_last_of("@");
  if(atmarkp != string::npos) {
    string authPart = hostPart.substr(0, atmarkp);
    pair<string, string> userPass = Util::split(authPart, ":");
    _username = Util::urldecode(userPass.first);
    _password = Util::urldecode(userPass.second);
    hostPart.erase(0, atmarkp+1);
  }
  pair<string, string> hostAndPort;
  Util::split(hostAndPort, hostPart, ':');
  host = hostAndPort.first;
  if(hostAndPort.second != "") {
    port = strtol(hostAndPort.second.c_str(), NULL, 10);
    if(!(0 < port && port <= 65535)) {
      return false;
    }
  } else {
    // If port is not specified, then we set it to default port of its protocol..
    port = defPort;
  }
  // find directory and file part
  string::size_type direp = tempUrl.find_last_of("/");
  if(direp == string::npos || direp <= hep) {
    dir = "/";
    direp = hep;
  } else {
    string rawDir = tempUrl.substr(hep, direp-hep);
    string::size_type p = rawDir.find_first_not_of("/");
    if(p != string::npos) {
      rawDir.erase(0, p-1);
    }
    p = rawDir.find_last_not_of("/");
    if(p != string::npos) {
      rawDir.erase(p+1);
    }
    dir = rawDir;
  }
  if(tempUrl.size() > direp+1) {
    file = tempUrl.substr(direp+1);
  }
  file += query;
  return true;
}

bool Request::isHexNumber(const char c) const
{
  return '0' <= c && c <= '9' || 'A' <= c && c <= 'F' || 'a' <= c && c <= 'f';
}

string Request::urlencode(const string& src) const
{
  int32_t lastIndex = src.size()-1;
  string result = src+"  ";
  for(int32_t index = lastIndex; index >= 0; --index) {
    const char c = result[index];
    // '/' is not urlencoded because src is expected to be a path.
    if(Util::shouldUrlencode(c)) {
      if(c == '%') {
	if(!isHexNumber(result[index+1]) || !isHexNumber(result[index+2])) {
	  result.replace(index, 1, "%25");
	}
      } else {
	char temp[4];
	sprintf(temp, "%%%02x", c);
	temp[3] = '\0';
	result.replace(index, 1, temp);
      }
    }
  }
  return result.substr(0, result.size()-2);
}
