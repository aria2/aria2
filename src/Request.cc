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

Request::Request():port(0), tryCount(0) {
  defaultPorts["http"] = 80;
#ifdef HAVE_LIBSSL
  // for SSL
  defaultPorts["https"] = 443;
#endif // HAVE_LIBSSL
  defaultPorts["ftp"] = 21;
  seg.sp = 0;
  seg.ep = 0;
  seg.ds = 0;
  seg.finish = false;
  cookieBox = new CookieBox();
}

Request::~Request() {}

bool Request::setUrl(string url) {
  this->url = url;
  return parseUrl(url);
}

bool Request::resetUrl() {
  previousUrl = referer;
  return setUrl(url);
}

bool Request::redirectUrl(string url) {
  previousUrl = currentUrl;
  return parseUrl(url);
}

bool Request::parseUrl(string url) {
  currentUrl = url;
  host = "";
  port = 0;
  dir = "";
  file = "";
  if(url.find_first_not_of(SAFE_CHARS) != string::npos) {
    return false;
  }
  string::size_type hp = url.find("://");
  if(hp == string::npos) return false;
  protocol = url.substr(0, hp);
  int defPort;
  if((defPort = defaultPorts[protocol]) == 0) {
    return false;
  }
  hp += 3;
  if(url.size() <= hp) return false;
  string::size_type hep = url.find("/", hp);
  if(hep == string::npos) {
    hep = url.size();
  }
  pair<string, string> hostAndPort;
  Util::split(hostAndPort, url.substr(hp, hep-hp), ':');
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
  string::size_type direp = url.find_last_of("/");
  if(direp == string::npos || direp <= hep) {
    dir = "/";
    direp = hep;
  } else {
    dir = url.substr(hep, direp-hep);
  }
  if(url.size() > direp+1) {
    file = url.substr(direp+1);
  }
  return true;
}
