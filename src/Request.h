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
#include "SharedHandle.h"
#include <string>
#include <deque>

#define SAFE_CHARS "abcdefghijklmnopqrstuvwxyz"\
"ABCDEFGHIJKLMNOPQRSTUVWXYZ"\
"0123456789"\
":/?[]@"\
"!$&'()*+,;="\
"-._~"\
"%"\
"#"

namespace aria2 {

class CookieBox;

class Request {
private:
  std::string url;
  std::string currentUrl;
  /**
   * URL previously requested to the server. This is used as Referer
   */
  std::string previousUrl;
  /**
   * URL used as Referer in the initial request
   */
  std::string referer;
  std::string protocol;
  std::string host;
  uint16_t port;
  std::string dir;
  std::string file;
  /* after ? mark(includes '?' itself) */
  std::string _query;
  unsigned int tryCount;

  // whether or not the server supports persistent connection
  bool _supportsPersistentConnection;
  // enable keep-alive if possible.
  bool _keepAliveHint;
  // enable pipelining if possible.
  bool _pipeliningHint;

  std::string method;

  std::string _username;

  std::string _password;

  bool parseUrl(const std::string& url);

  bool isHexNumber(const char c) const;

  void urlencode(std::string& result, const std::string& src) const;

public:
  SharedHandle<CookieBox> cookieBox;
public:
  Request();
  virtual ~Request();

  // Parses URL and sets url, host, port, dir, file fields.
  // Returns true if parsing goes successful, otherwise returns false.
  bool setUrl(const std::string& url);
  // Parses URL and sets host, port, dir, file fields.
  // url field are not altered by this method.
  // Returns true if parsing goes successful, otherwise returns false.
  bool redirectUrl(const std::string& url);
  bool resetUrl();
  void resetTryCount() { tryCount = 0; }
  void addTryCount() { tryCount++; }
  unsigned int getTryCount() const { return tryCount; }
  //bool noMoreTry() const { return tryCount >= PREF_MAX_TRY; }

  const std::string& getUrl() const { return url; }
  const std::string& getCurrentUrl() const { return currentUrl; }
  const std::string& getPreviousUrl() const { return previousUrl; }
  const std::string& getReferer() const { return referer; }
  void setReferer(const std::string& url) { referer = previousUrl = url; }
  const std::string& getProtocol() const { return protocol; }
  const std::string& getHost() const { return host; }
  uint16_t getPort() const { return port; }
  const std::string& getDir() const { return dir; }
  const std::string& getFile() const { return file;}
  const std::string& getQuery() const { return _query; }

  void supportsPersistentConnection(bool f)
  {
    _supportsPersistentConnection = f;
  }

  bool supportsPersistentConnection()
  {
    return _supportsPersistentConnection;
  }

  bool isKeepAliveEnabled() const
  {
    return _supportsPersistentConnection && _keepAliveHint;
  }

  void setKeepAliveHint(bool keepAliveHint)
  {
    _keepAliveHint = keepAliveHint;
  }

  bool isPipeliningEnabled()
  {
    return _supportsPersistentConnection && _pipeliningHint;
  }

  void setPipeliningHint(bool pipeliningHint)
  {
    _pipeliningHint = pipeliningHint;
  }

  void setMethod(const std::string& method) {
    this->method = method;
  }

  const std::string& getUsername() const
  {
    return _username;
  }

  const std::string& getPassword() const
  {
    return _password;
  }

  const std::string& getMethod() const {
    return method;
  }

  static const std::string METHOD_GET;
  static const std::string METHOD_HEAD;

  static const std::string PROTO_HTTP;

  static const std::string PROTO_HTTPS;

  static const std::string PROTO_FTP;

};

typedef SharedHandle<Request> RequestHandle;
typedef WeakHandle<Request> RequestWeakHandle;
typedef std::deque<RequestHandle> Requests;

} // namespace aria2

#endif // _D_REQUEST_H_
