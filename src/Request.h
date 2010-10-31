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
#include "PeerStat.h"
#include "a2functional.h"

namespace aria2 {

class Request {
private:
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
  std::string protocol_;
  std::string host_;
  uint16_t port_;
  std::string dir_;
  std::string file_;
  /* after ? mark(includes '?' itself) */
  std::string query_;
  unsigned int tryCount_;

  unsigned int redirectCount_;

  // whether or not the server supports persistent connection
  bool supportsPersistentConnection_;
  // enable keep-alive if possible.
  bool keepAliveHint_;
  // enable pipelining if possible.
  bool pipeliningHint_;
  // maximum number of pipelined requests
  unsigned int maxPipelinedRequest_;

  std::string method_;

  std::string username_;

  std::string password_;

  bool hasPassword_;

  bool ipv6LiteralAddress_;

  SharedHandle<PeerStat> peerStat_;

  bool removalRequested_;

  std::string connectedHostname_;

  std::string connectedAddr_;

  uint16_t connectedPort_;

  bool parseUri(const std::string& uri);
public:
  Request();

  // Sets uri to uri_ and parses URI.  Returns true if parsing goes
  // successful, otherwise returns false.
  bool setUri(const std::string& uri);
  // Parses URI.  uri_ field are not altered by this method.  Returns
  // true if parsing goes successful, otherwise returns false.
  bool redirectUri(const std::string& uri);
  bool resetUri();
  void resetTryCount() { tryCount_ = 0; }
  void addTryCount() { ++tryCount_; }
  unsigned int getTryCount() const { return tryCount_; }

  void resetRedirectCount();
  
  unsigned int getRedirectCount() const
  {
    return redirectCount_;
  }

  // Returns URI passed by setUri()
  const std::string& getUri() const { return uri_; }
  const std::string& getCurrentUri() const { return currentUri_; }
  const std::string& getPreviousUri() const { return previousUri_; }
  const std::string& getReferer() const { return referer_; }
  void setReferer(const std::string& uri);
  const std::string& getProtocol() const { return protocol_; }
  const std::string& getHost() const { return host_; }
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
  uint16_t getPort() const { return port_; }
  const std::string& getDir() const { return dir_; }
  const std::string& getFile() const { return file_;}
  const std::string& getQuery() const { return query_; }
  bool isIPv6LiteralAddress() const { return ipv6LiteralAddress_; }

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

  void setMaxPipelinedRequest(unsigned int num);

  unsigned int getMaxPipelinedRequest() const
  {
    return maxPipelinedRequest_;
  }

  void setMethod(const std::string& method) {
    method_ = method;
  }

  const std::string& getUsername() const
  {
    return username_;
  }

  const std::string& getPassword() const
  {
    return password_;
  }

  // Returns true if current URI has embedded password.
  bool hasPassword() const
  {
    return hasPassword_;
  }

  const std::string& getMethod() const {
    return method_;
  }

  const SharedHandle<PeerStat>& getPeerStat() const { return peerStat_; }

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
  (const std::string& hostname, const std::string& addr, uint16_t port)
  {
    connectedHostname_ = hostname;
    connectedAddr_ = addr;
    connectedPort_ = port;
  }

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

  static const std::string METHOD_GET;
  static const std::string METHOD_HEAD;

  static const std::string PROTO_HTTP;

  static const std::string PROTO_HTTPS;

  static const std::string PROTO_FTP;

  static const unsigned int MAX_REDIRECT = 20;

};

} // namespace aria2

#endif // D_REQUEST_H
