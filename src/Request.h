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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
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

#include <string>

#include "SharedHandle.h"
#include "PeerStat.h"
#include "a2functional.h"

namespace aria2 {

class Request {
private:
  std::string _url;
  std::string _currentUrl;
  /**
   * URL previously requested to the server. This is used as Referer
   */
  std::string _previousUrl;
  /**
   * URL used as Referer in the initial request
   */
  std::string _referer;
  std::string _protocol;
  std::string _host;
  uint16_t _port;
  std::string _dir;
  std::string _file;
  /* after ? mark(includes '?' itself) */
  std::string _query;
  unsigned int _tryCount;

  unsigned int _redirectCount;

  // whether or not the server supports persistent connection
  bool _supportsPersistentConnection;
  // enable keep-alive if possible.
  bool _keepAliveHint;
  // enable pipelining if possible.
  bool _pipeliningHint;
  // maximum number of pipelined requests
  unsigned int _maxPipelinedRequest;

  std::string _method;

  std::string _username;

  std::string _password;

  bool _hasPassword;

  bool _ipv6LiteralAddress;

  SharedHandle<PeerStat> _peerStat;

  bool parseUrl(const std::string& url);
public:
  Request();

  // Sets url to _url and parses URL.  Returns true if parsing goes
  // successful, otherwise returns false.
  bool setUrl(const std::string& url);
  // Parses URL.  _url field are not altered by this method.  Returns
  // true if parsing goes successful, otherwise returns false.
  bool redirectUrl(const std::string& url);
  bool resetUrl();
  void resetTryCount() { _tryCount = 0; }
  void addTryCount() { ++_tryCount; }
  unsigned int getTryCount() const { return _tryCount; }

  void resetRedirectCount();
  
  unsigned int getRedirectCount() const
  {
    return _redirectCount;
  }

  // Returns URI passed by setUrl()
  const std::string& getUrl() const { return _url; }
  const std::string& getCurrentUrl() const { return _currentUrl; }
  const std::string& getPreviousUrl() const { return _previousUrl; }
  const std::string& getReferer() const { return _referer; }
  void setReferer(const std::string& url);
  const std::string& getProtocol() const { return _protocol; }
  const std::string& getHost() const { return _host; }
  // Same as getHost(), but for IPv6 literal addresses, enclose them
  // with square brackets and return.
  std::string getURIHost() const
  {
    if(isIPv6LiteralAddress()) {
      return strconcat("[", getHost(), "]");
    } else {
      return getHost();
    }
  }
  uint16_t getPort() const { return _port; }
  const std::string& getDir() const { return _dir; }
  const std::string& getFile() const { return _file;}
  const std::string& getQuery() const { return _query; }
  bool isIPv6LiteralAddress() const { return _ipv6LiteralAddress; }

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

  bool isPipeliningHint() const
  {
    return _pipeliningHint;
  }

  void setMaxPipelinedRequest(unsigned int num);

  unsigned int getMaxPipelinedRequest() const
  {
    return _maxPipelinedRequest;
  }

  void setMethod(const std::string& method) {
    _method = method;
  }

  const std::string& getUsername() const
  {
    return _username;
  }

  const std::string& getPassword() const
  {
    return _password;
  }

  // Returns true if current URI has embedded password.
  bool hasPassword() const
  {
    return _hasPassword;
  }

  const std::string& getMethod() const {
    return _method;
  }

  const SharedHandle<PeerStat>& getPeerStat() const { return _peerStat; }

  const SharedHandle<PeerStat>& initPeerStat();

  static const std::string METHOD_GET;
  static const std::string METHOD_HEAD;

  static const std::string PROTO_HTTP;

  static const std::string PROTO_HTTPS;

  static const std::string PROTO_FTP;

  static const unsigned int MAX_REDIRECT = 20;

};

} // namespace aria2

#endif // _D_REQUEST_H_
