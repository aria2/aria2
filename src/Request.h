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
#ifndef _D_REQUEST_H_
#define _D_REQUEST_H_
#include <string>
#include <map>
#include <Segment.h>
#include "CookieBox.h"
#include "common.h"

using namespace std;

#define MAX_TRY_COUNT 5

#define SAFE_CHARS "abcdefghijklmnopqrstuvwxyz"\
"ABCDEFGHIJKLMNOPQRSTUVWXYZ"\
"0123456789"\
":/?[]@"\
"!$&'()*+,;="\
"-._~"\
"%"

class Request {
private:
  string url;
  string currentUrl;
  /**
   * URL previously requested to the server. This is used as Referer
   */
  string previousUrl;
  /**
   * URL used as Referer in the initial request
   */
  string referer;
  string protocol;
  string host;
  int port;
  string dir;
  string file;
  map<string, int> defaultPorts;
  int tryCount;
  bool parseUrl(string url);
public:
  Segment seg;
  CookieBox* cookieBox;
public:
  Request();
  virtual ~Request();

  // Parses URL and sets url, host, port, dir, file fields.
  // Returns true if parsing goes successful, otherwise returns false.
  bool setUrl(string url);
  // Parses URL and sets host, port, dir, file fields.
  // url field are not altered by this method.
  // Returns true if parsing goes successful, otherwise returns false.
  bool redirectUrl(string url);
  bool resetUrl();
  void resetTryCount() { tryCount = 0; }
  void addTryCount() { tryCount++; }
  int getTryCount() const { return tryCount; }
  bool noMoreTry() const { return tryCount >= MAX_TRY_COUNT; }

  string getUrl() const { return url; }
  string getCurrentUrl() const { return currentUrl; }
  string getPreviousUrl() const { return previousUrl; }
  string getReferer() const { return referer; }
  void setReferer(string url) { referer = previousUrl = url; }
  string getProtocol() const { return protocol; }
  string getHost() const { return host; }
  int getPort() const { return port; }
  string getDir() const { return dir; }
  string getFile() const { return file;}
};

#endif // _D_REQUEST_H_
