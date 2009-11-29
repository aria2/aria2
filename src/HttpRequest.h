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

#include <cassert>
#include <string>
#include <deque>

#include "SharedHandle.h"
#include "Request.h"
#include "FileEntry.h"

namespace aria2 {

class Segment;
class Range;
class Option;
class CookieStorage;
class AuthConfigFactory;
class AuthConfig;

class HttpRequest {
private:
  
  static const std::string USER_AGENT;

  SharedHandle<Request> request;

  SharedHandle<FileEntry> _fileEntry;

  SharedHandle<Segment> segment;

  bool _contentEncodingEnabled;

  std::string userAgent;

  std::deque<std::string> _headers;

  std::deque<std::string> _acceptTypes;

  SharedHandle<CookieStorage> _cookieStorage;

  SharedHandle<AuthConfigFactory> _authConfigFactory;

  const Option* _option;

  SharedHandle<AuthConfig> _authConfig;

  SharedHandle<Request> _proxyRequest;

  std::string getProxyAuthString() const;
public:
  HttpRequest();

  const SharedHandle<Segment>& getSegment() const
  {
    return segment;
  }

  void setSegment(const SharedHandle<Segment>& segment);

  void setRequest(const SharedHandle<Request>& request);

  uint64_t getEntityLength() const
  {
    assert(!_fileEntry.isNull());
    return _fileEntry->getLength();
  }

  const std::string& getHost() const
  {
    return request->getHost();
  }

  uint16_t getPort() const
  {
    return request->getPort();
  }

  const std::string& getMethod() const
  {
    return request->getMethod();
  }

  const std::string& getProtocol() const
  {
    return request->getProtocol();
  }

  const std::string& getCurrentURI() const
  {
    return request->getCurrentUrl();
  }
  
  const std::string& getDir() const
  {
    return request->getDir();
  }

  const std::string& getFile() const
  {
    return request->getFile();
  }

  const std::string& getQuery() const
  {
    return request->getQuery();
  }

  const std::string& getPreviousURI() const
  {
    return request->getPreviousUrl();
  }

  std::string getURIHost() const
  {
    return request->getURIHost();
  }

  SharedHandle<Range> getRange() const;

  /**
   * Inspects whether the specified response range is satisfiable
   * with request range.
   */
  bool isRangeSatisfied(const SharedHandle<Range>& range) const;

  const SharedHandle<Request>& getRequest() const
  {
    return request;
  }

  off_t getStartByte() const;

  off_t getEndByte() const;

  /**
   * Returns string representation of http request.  It usually starts
   * with "GET ..." and ends with "\r\n".  The AuthConfig for this
   * request is resolved using _authConfigFactory and stored in
   * _authConfig.  getAuthConfig() returns AuthConfig used in the last
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

  void setUserAgent(const std::string& userAgent)
  {
    this->userAgent = userAgent;
  }
  
  // accepts multiline headers, delimited by LF
  void addHeader(const std::string& headers);

  void addAcceptType(const std::string& type);

  template<typename InputIterator>
  void addAcceptType(InputIterator first, InputIterator last)
  {
    _acceptTypes.insert(_acceptTypes.end(), first, last);
  }

  void setCookieStorage(const SharedHandle<CookieStorage>& cookieStorage);

  const SharedHandle<CookieStorage>& getCookieStorage() const
  {
    return _cookieStorage;
  }

  void setAuthConfigFactory
  (const SharedHandle<AuthConfigFactory>& factory, const Option* option);

  /*
   * To use proxy, pass proxy string to Request::setUrl() and set it this
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

  void setFileEntry(const SharedHandle<FileEntry>& fileEntry)
  {
    _fileEntry = fileEntry;
  }

  const SharedHandle<FileEntry>& getFileEntry() const
  {
    return _fileEntry;
  }
};

typedef SharedHandle<HttpRequest> HttpRequestHandle;
typedef std::deque<HttpRequestHandle> HttpRequests;

} // namespace aria2

#endif // _D_HTTP_REQUEST_H_
