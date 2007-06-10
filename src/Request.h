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
#ifndef _D_REQUEST_H_
#define _D_REQUEST_H_
#include "common.h"
#include "CookieBox.h"
#include "AuthConfig.h"
#include "AuthResolver.h"

#define SAFE_CHARS "abcdefghijklmnopqrstuvwxyz"\
"ABCDEFGHIJKLMNOPQRSTUVWXYZ"\
"0123456789"\
":/?[]@"\
"!$&'()*+,;="\
"-._~"\
"%"\
"#"

#define METALINK_MARK "#!metalink3!"

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
  int tryCount;
  int trackerEvent;
  bool keepAlive;
  string method;

  AuthResolverHandle _httpAuthResolver;

  AuthResolverHandle _httpProxyAuthResolver;

  AuthResolverHandle _ftpAuthResolver;

  bool parseUrl(const string& url);
public:
  CookieBoxHandle cookieBox;
public:
  Request();
  virtual ~Request();

  // Parses URL and sets url, host, port, dir, file fields.
  // Returns true if parsing goes successful, otherwise returns false.
  bool setUrl(const string& url);
  // Parses URL and sets host, port, dir, file fields.
  // url field are not altered by this method.
  // Returns true if parsing goes successful, otherwise returns false.
  bool redirectUrl(const string& url);
  bool resetUrl();
  void resetTryCount() { tryCount = 0; }
  void addTryCount() { tryCount++; }
  int getTryCount() const { return tryCount; }
  //bool noMoreTry() const { return tryCount >= PREF_MAX_TRY; }

  string getUrl() const { return url; }
  string getCurrentUrl() const { return currentUrl; }
  string getPreviousUrl() const { return previousUrl; }
  string getReferer() const { return referer; }
  void setReferer(const string& url) { referer = previousUrl = url; }
  string getProtocol() const { return protocol; }
  string getHost() const { return host; }
  int getPort() const { return port; }
  string getDir() const { return dir; }
  string getFile() const { return file;}
  bool isKeepAlive() const { return keepAlive; }
  void setKeepAlive(bool keepAlive) { this->keepAlive = keepAlive; }
  void setTrackerEvent(int event) { trackerEvent = event; }
  int getTrackerEvent() const { return trackerEvent; }

  void setMethod(const string& method) {
    this->method = method;
  }

  void setHttpAuthResolver(const AuthResolverHandle& authResolver)
  {
    _httpAuthResolver = authResolver;
  }

  void setHttpProxyAuthResolver(const AuthResolverHandle& authResolver)
  {
    _httpProxyAuthResolver = authResolver;
  }

  void setFtpAuthResolver(const AuthResolverHandle& authResolver)
  {
    _ftpAuthResolver = authResolver;
  }

  AuthConfigHandle resolveHttpAuthConfig();

  AuthConfigHandle resolveFtpAuthConfig();

  AuthConfigHandle resolveHttpProxyAuthConfig();

  const string& getMethod() const {
    return method;
  }

  static const string METHOD_GET;
  static const string METHOD_HEAD;

  enum TRACKER_EVENT {
    AUTO,
    STARTED,
    STOPPED,
    COMPLETED,
    AFTER_COMPLETED
  };

};

typedef SharedHandle<Request> RequestHandle;
typedef deque<RequestHandle> Requests;
typedef WeakHandle<Request> RequestWeakHandle;

#endif // _D_REQUEST_H_
