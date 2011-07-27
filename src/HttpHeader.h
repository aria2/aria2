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
#include <iosfwd>

#include "SharedHandle.h"

namespace aria2 {

class Range;

class HttpHeader {
private:
  std::multimap<std::string, std::string> table_;

  // HTTP status code, e.g. 200
  int statusCode_;

  // HTTP version, e.g. HTTP/1.1
  std::string version_;

  // HTTP Method, e.g. GET, POST, etc
  std::string method_;

  // Request Path
  std::string requestPath_;
public:
  HttpHeader();
  ~HttpHeader();

  void put(const std::string& name, const std::string& value);
  bool defined(const std::string& name) const;
  const std::string& getFirst(const std::string& name) const;
  std::vector<std::string> get(const std::string& name) const;
  std::pair<std::multimap<std::string, std::string>::const_iterator,
            std::multimap<std::string, std::string>::const_iterator>
  getIterator(const std::string& name) const;
  unsigned int getFirstAsUInt(const std::string& name) const;
  uint64_t getFirstAsULLInt(const std::string& name) const;

  SharedHandle<Range> getRange() const;

  int getStatusCode() const
  {
    return statusCode_;
  }

  void setStatusCode(int code)
  {
    statusCode_ = code;
  }

  const std::string& getVersion() const
  {
    return version_;
  }

  void setVersion(const std::string& version);

  const std::string& getMethod() const
  {
    return method_;
  }

  void setMethod(const std::string& method);

  const std::string& getRequestPath() const
  {
    return requestPath_;
  }

  void setRequestPath(const std::string& requestPath);

  void fill(std::istream& in);

  // Clears table_. responseStatus_ and version_ are unchanged.
  void clearField();

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

  static const char HTTP_1_1[];

  static const char CLOSE[];

  static const char CHUNKED[];
  
  static const char GZIP[];

  static const char DEFLATE[];
};

typedef SharedHandle<HttpHeader> HttpHeaderHandle;

} // namespace std;

#endif // D_HTTP_HEADER_H
