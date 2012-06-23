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
#ifndef D_HTTP_HEADER_H
#define D_HTTP_HEADER_H

#include "common.h"

#include <map>
#include <vector>
#include <string>

#include "SharedHandle.h"

namespace aria2 {

class Range;

class HttpHeader {
private:
  std::multimap<std::string, std::string> table_;

  // HTTP status code, e.g. 200
  int statusCode_;

  // The reason-phrase for the response
  std::string reasonPhrase_;

  // HTTP version, e.g. HTTP/1.1
  std::string version_;

  // HTTP Method, e.g. GET, POST, etc
  std::string method_;

  // Request Path
  std::string requestPath_;
public:
  HttpHeader();
  ~HttpHeader();

  // For all methods, use lowercased header field name.
  void put(const std::string& name, const std::string& value);
  bool defined(const std::string& name) const;
  const std::string& find(const std::string& name) const;
  std::vector<std::string> findAll(const std::string& name) const;
  std::pair<std::multimap<std::string, std::string>::const_iterator,
            std::multimap<std::string, std::string>::const_iterator>
  equalRange(const std::string& name) const;
  int32_t findAsInt(const std::string& name) const;
  int64_t findAsLLInt(const std::string& name) const;

  SharedHandle<Range> getRange() const;

  int getStatusCode() const;

  void setStatusCode(int code);

  const std::string& getReasonPhrase() const;

  void setReasonPhrase(const std::string& reasonPhrase);

  const std::string& getVersion() const;

  void setVersion(const std::string& version);

  template<typename InputIterator>
  void setVersion(InputIterator first, InputIterator last)
  {
    version_.assign(first, last);
  }

  const std::string& getMethod() const;

  void setMethod(const std::string& method);

  template<typename InputIterator>
  void setMethod(InputIterator first, InputIterator last)
  {
    method_.assign(first, last);
  }

  const std::string& getRequestPath() const;

  void setRequestPath(const std::string& requestPath);

  template<typename InputIterator>
  void setRequestPath(InputIterator first, InputIterator last)
  {
    requestPath_.assign(first, last);
  }

  // Clears table_. responseStatus_ and version_ are unchanged.
  void clearField();

  // Returns true if heder field |name| contains |value|. This method
  // assumes the values of the header field is delimited by ','.
  bool fieldContains(const std::string& name, const std::string& value);

  static const std::string LOCATION;
  static const std::string TRANSFER_ENCODING;
  static const std::string CONTENT_ENCODING;
  static const std::string CONTENT_DISPOSITION;
  static const std::string SET_COOKIE;
  static const std::string CONTENT_TYPE;
  static const std::string RETRY_AFTER;
  static const std::string CONNECTION;
  static const std::string CONTENT_LENGTH;
  static const std::string CONTENT_RANGE;
  static const std::string LAST_MODIFIED;
  static const std::string ACCEPT_ENCODING;
  static const std::string LINK;
  static const std::string DIGEST;
  static const std::string AUTHORIZATION;
  static const std::string PROXY_CONNECTION;

  static const std::string HTTP_1_1;
  static const std::string CLOSE;
  static const std::string KEEP_ALIVE;
  static const std::string CHUNKED;
  static const std::string GZIP;
  static const std::string DEFLATE;
};

typedef SharedHandle<HttpHeader> HttpHeaderHandle;

} // namespace std;

#endif // D_HTTP_HEADER_H
