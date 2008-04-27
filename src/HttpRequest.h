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
#ifndef _D_HTTP_REQUEST_H_
#define _D_HTTP_REQUEST_H_

#include "common.h"
#include "SharedHandle.h"
#include <string>
#include <deque>

namespace aria2 {

class Request;
class Segment;
class Range;
class Option;

class HttpRequest {
private:
  
  static std::string USER_AGENT;

  SharedHandle<Request> request;

  SharedHandle<Segment> segment;

  uint64_t entityLength;

  bool authEnabled;

  bool proxyEnabled;

  bool proxyAuthEnabled;

  std::string userAgent;

  std::deque<std::string> _headers;

  std::string getHostText(const std::string& host, uint16_t port) const;

  std::string getProxyAuthString() const;

public:
  HttpRequest();

  SharedHandle<Segment> getSegment() const;

  void setSegment(const SharedHandle<Segment>& segment);

  void setRequest(const SharedHandle<Request>& request);

  /**
   * entityLength is used in isRangeSatisfied() method.
   */
  void setEntityLength(uint64_t entityLength)
  {
    this->entityLength = entityLength;
  }

  uint64_t getEntityLength() const
  {
    return entityLength;
  }

  std::string getHost() const;

  uint16_t getPort() const;

  std::string getMethod() const;

  std::string getProtocol() const;

  std::string getCurrentURI() const;
  
  std::string getDir() const;

  std::string getFile() const;

  std::string getQuery() const;

  std::string getPreviousURI() const;

  SharedHandle<Range> getRange() const;

  /**
   * Inspects whether the specified response range is satisfiable
   * with request range.
   */
  bool isRangeSatisfied(const SharedHandle<Range>& range) const;

  SharedHandle<Request> getRequest() const;

  off_t getStartByte() const;

  off_t getEndByte() const;

  /**
   * Returns string representation of http request.
   * It usually starts with "GET ..." and ends with "\r\n".
   */
  std::string createRequest() const;

  /**
   * Returns string representation of http tunnel request.
   * It usually starts with "CONNECT ..." and ends with "\r\n".
   */
  std::string createProxyRequest() const;

  /**
   * Configures this object with option.
   * Following values are evaluated:
   * PREF_HTTP_AUTH_ENABLED, PREF_HTTP_PROXY_ENABLED,
   * PREF_HTTP_PROXY_METHOD, PREF_HTTP_PROXY_AUTH_ENABLED,
   * PREF_HTTP_USER, PREF_HTTP_PASSWD,
   * PREF_HTTP_PROXY_USER, PREF_HTTP_PROXY_PASSWD
   * The evaluation results are stored in instance variables.
   */
  void configure(const Option* option);

  void setProxyEnabled(bool proxyEnabled)
  {
    this->proxyEnabled = proxyEnabled;
  }

  void setProxyAuthEnabled(bool proxyAuthEnabled)
  {
    this->proxyAuthEnabled = proxyAuthEnabled;
  }

  void setAuthEnabled(bool authEnabled)
  {
    this->authEnabled = authEnabled;
  }

  void setUserAgent(const std::string& userAgent)
  {
    this->userAgent = userAgent;
  }
  
  // accepts multiline headers, deliminated by LF
  void addHeader(const std::string& headers);
};

typedef SharedHandle<HttpRequest> HttpRequestHandle;
typedef std::deque<HttpRequestHandle> HttpRequests;

} // namespace aria2

#endif // _D_HTTP_REQUEST_H_
