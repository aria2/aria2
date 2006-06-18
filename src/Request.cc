/* <!-- copyright */
/*
 * aria2 - a simple utility for downloading files faster
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
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */
/* copyright --> */
#include "Request.h"
#include "Util.h"

Request::Request():port(0), tryCount(0), isTorrent(false) {
  defaultPorts["http"] = 80;
#ifdef ENABLE_SSL
  // for SSL
  defaultPorts["https"] = 443;
#endif // ENABLE_SSL
  defaultPorts["ftp"] = 21;
  seg.sp = 0;
  seg.ep = 0;
  seg.ds = 0;
  seg.finish = false;
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
  currentUrl = url;
  string tempUrl = url;
  string query;
  host = "";
  port = 0;
  dir = "";
  file = "";
  if(url.find_first_not_of(SAFE_CHARS) != string::npos) {
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
  if((defPort = defaultPorts[protocol]) == 0) {
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
