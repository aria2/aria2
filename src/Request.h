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
#ifndef D_REQUEST_H
#define D_REQUEST_H
#include "common.h"

#include <string>

#include "SharedHandle.h"
#include "TimerA2.h"
#include "uri.h"

namespace aria2 {

class PeerStat;

class Request {
private:
  uri::UriStruct us_;
  std::string uri_;
  std::string currentUri_;
  /**
   * URI previously requested to the server. This is used as Referer
   */
  std::string previousUri_;
  /**
   * URI used as Referer in the initial request
   */
  std::string referer_;
  std::string method_;
  std::string connectedHostname_;
  std::string connectedAddr_;

  int tryCount_;
  int redirectCount_;
  // whether or not the server supports persistent connection
  bool supportsPersistentConnection_;
  // enable keep-alive if possible.
  bool keepAliveHint_;
  // enable pipelining if possible.
  bool pipeliningHint_;
  // maximum number of pipelined requests
  int maxPipelinedRequest_;
  SharedHandle<PeerStat> peerStat_;
  bool removalRequested_;
  uint16_t connectedPort_;
  Timer wakeTime_;

  bool parseUri(const std::string& uri);
public:
  Request();
  ~Request();
  // Sets uri to uri_ and parses URI.  Returns true if parsing goes
  // successful, otherwise returns false.
  bool setUri(const std::string& uri);
  // Parses URI.  uri_ field are not altered by this method.  Returns
  // true if parsing goes successful, otherwise returns false.
  bool redirectUri(const std::string& uri);
  bool resetUri();
  void resetTryCount() { tryCount_ = 0; }
  void addTryCount() { ++tryCount_; }
  int getTryCount() const { return tryCount_; }
  void resetRedirectCount();
  int getRedirectCount() const { return redirectCount_; }
  // Returns URI passed by setUri()
  const std::string& getUri() const { return uri_; }
  const std::string& getCurrentUri() const { return currentUri_; }
  const std::string& getPreviousUri() const { return previousUri_; }
  const std::string& getReferer() const { return referer_; }
  void setReferer(const std::string& uri);
  const std::string& getProtocol() const { return us_.protocol; }
  const std::string& getHost() const { return us_.host; }
  // Same as getHost(), but for IPv6 literal addresses, enclose them
  // with square brackets and return.
  std::string getURIHost() const;
  uint16_t getPort() const { return us_.port; }
  const std::string& getDir() const { return us_.dir; }
  const std::string& getFile() const { return us_.file;}
  const std::string& getQuery() const { return us_.query; }
  bool isIPv6LiteralAddress() const { return us_.ipv6LiteralAddress; }

  void supportsPersistentConnection(bool f)
  {
    supportsPersistentConnection_ = f;
  }

  bool supportsPersistentConnection()
  {
    return supportsPersistentConnection_;
  }

  bool isKeepAliveEnabled() const
  {
    return supportsPersistentConnection_ && keepAliveHint_;
  }

  void setKeepAliveHint(bool keepAliveHint)
  {
    keepAliveHint_ = keepAliveHint;
  }

  bool isPipeliningEnabled()
  {
    return supportsPersistentConnection_ && pipeliningHint_;
  }

  void setPipeliningHint(bool pipeliningHint)
  {
    pipeliningHint_ = pipeliningHint;
  }

  bool isPipeliningHint() const
  {
    return pipeliningHint_;
  }

  void setMaxPipelinedRequest(int num);

  int getMaxPipelinedRequest() const
  {
    return maxPipelinedRequest_;
  }

  void setMethod(const std::string& method);

  const std::string& getUsername() const
  {
    return us_.username;
  }

  const std::string& getPassword() const
  {
    return us_.password;
  }

  // Returns true if current URI has embedded password.
  bool hasPassword() const
  {
    return us_.hasPassword;
  }

  const std::string& getMethod() const
  {
    return method_;
  }

  const SharedHandle<PeerStat>& getPeerStat() const
  {
    return peerStat_;
  }

  const SharedHandle<PeerStat>& initPeerStat();

  void requestRemoval()
  {
    removalRequested_ = true;
  }

  bool removalRequested() const
  {
    return removalRequested_;
  }

  void setConnectedAddrInfo
  (const std::string& hostname, const std::string& addr, uint16_t port);

  const std::string& getConnectedHostname() const
  {
    return connectedHostname_;
  }

  const std::string& getConnectedAddr() const
  {
    return connectedAddr_;
  }

  uint16_t getConnectedPort() const
  {
    return connectedPort_;
  }

  void setWakeTime(Timer timer)
  {
    wakeTime_ = timer;
  }

  const Timer& getWakeTime()
  {
    return wakeTime_;
  }

  static const std::string METHOD_GET;
  static const std::string METHOD_HEAD;

  static const std::string PROTO_HTTP;

  static const std::string PROTO_HTTPS;

  static const std::string PROTO_FTP;

  static const int MAX_REDIRECT = 20;

};

} // namespace aria2

#endif // D_REQUEST_H
