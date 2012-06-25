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
#ifndef D_HTTP_REQUEST_H
#define D_HTTP_REQUEST_H

#include "common.h"

#include <cassert>
#include <string>
#include <vector>

#include "SharedHandle.h"
#include "FileEntry.h"

namespace aria2 {

class Request;
class Segment;
class Range;
class Option;
class CookieStorage;
class AuthConfigFactory;
class AuthConfig;

class HttpRequest {
private:
  
  static const std::string USER_AGENT;

  SharedHandle<Request> request_;

  SharedHandle<FileEntry> fileEntry_;

  SharedHandle<Segment> segment_;

  bool contentEncodingEnabled_;

  std::string userAgent_;

  std::vector<std::string> headers_;

  std::vector<std::string> acceptTypes_;

  SharedHandle<CookieStorage> cookieStorage_;

  SharedHandle<AuthConfigFactory> authConfigFactory_;

  const Option* option_;

  SharedHandle<AuthConfig> authConfig_;

  SharedHandle<Request> proxyRequest_;

  bool noCache_;

  bool acceptGzip_;

  int64_t endOffsetOverride_;

  std::string ifModSinceHeader_;

  std::pair<std::string, std::string> getProxyAuthString() const;
public:
  HttpRequest();
  ~HttpRequest();

  const SharedHandle<Segment>& getSegment() const
  {
    return segment_;
  }

  void setSegment(const SharedHandle<Segment>& segment);

  void setRequest(const SharedHandle<Request>& request);

  int64_t getEntityLength() const;

  const std::string& getHost() const;

  uint16_t getPort() const;

  const std::string& getMethod() const;

  const std::string& getProtocol() const;

  const std::string& getCurrentURI() const;
  
  const std::string& getDir() const;

  const std::string& getFile() const;

  const std::string& getQuery() const;

  const std::string& getPreviousURI() const;

  std::string getURIHost() const;

  SharedHandle<Range> getRange() const;

  /**
   * Inspects whether the specified response range is satisfiable
   * with request range.
   */
  bool isRangeSatisfied(const SharedHandle<Range>& range) const;

  const SharedHandle<Request>& getRequest() const
  {
    return request_;
  }

  int64_t getStartByte() const;

  int64_t getEndByte() const;

  /**
   * Returns string representation of http request.  It usually starts
   * with "GET ..." and ends with "\r\n".  The AuthConfig for this
   * request is resolved using authConfigFactory_ and stored in
   * authConfig_.  getAuthConfig() returns AuthConfig used in the last
   * invocation of createRequest().
   */
  std::string createRequest();

  /**
   * Returns string representation of http tunnel request.
   * It usually starts with "CONNECT ..." and ends with "\r\n".
   */
  std::string createProxyRequest() const;

  void enableContentEncoding();

  void disableContentEncoding();

  void setUserAgent(const std::string& userAgent);
  
  // accepts multiline headers, delimited by LF
  void addHeader(const std::string& headers);

  void clearHeader();

  void addAcceptType(const std::string& type);

  template<typename InputIterator>
  void addAcceptType(InputIterator first, InputIterator last)
  {
    acceptTypes_.insert(acceptTypes_.end(), first, last);
  }

  void setCookieStorage(const SharedHandle<CookieStorage>& cookieStorage);

  const SharedHandle<CookieStorage>& getCookieStorage() const
  {
    return cookieStorage_;
  }

  void setAuthConfigFactory
  (const SharedHandle<AuthConfigFactory>& factory, const Option* option);

  /*
   * To use proxy, pass proxy string to Request::setUri() and set it this
   * object.
   */
  void setProxyRequest(const SharedHandle<Request>& proxyRequest);
  
  /*
   * Returns true if non-Null proxy request is set by setProxyRequest().
   * Otherwise, returns false.
   */
  bool isProxyRequestSet() const;

  // Returns true if authentication was used in the last
  // createRequest().
  bool authenticationUsed() const;

  // Returns AuthConfig used in the last invocation of
  // createRequest().
  const SharedHandle<AuthConfig>& getAuthConfig() const;

  void setFileEntry(const SharedHandle<FileEntry>& fileEntry);

  const SharedHandle<FileEntry>& getFileEntry() const
  {
    return fileEntry_;
  }

  void enableNoCache()
  {
    noCache_ = true;
  }

  void disableNoCache()
  {
    noCache_ = false;
  }

  void enableAcceptGZip()
  {
    acceptGzip_ = true;
  }

  void disableAcceptGZip()
  {
    acceptGzip_ = false;
  }

  bool acceptGZip() const
  {
    return acceptGzip_;
  }

  void setEndOffsetOverride(int64_t offset)
  {
    endOffsetOverride_ = offset;
  }

  void setIfModifiedSinceHeader(const std::string& hd);

  const std::string& getIfModifiedSinceHeader() const
  {
    return ifModSinceHeader_;
  }

  // Returns true if request is conditional:more specifically, the
  // request is considered to be conditional if the client sent
  // "If-Modified-Since" or "If-None-Match" request-header field.
  bool conditionalRequest() const;
};

} // namespace aria2

#endif // D_HTTP_REQUEST_H
