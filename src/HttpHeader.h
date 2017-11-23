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

namespace aria2 {

struct Range;

class HttpHeader {
private:
  std::multimap<int, std::string> table_;

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

  // The list of headers we are interested in. Only those header
  // values are stored in table_. When updating this list, also update
  // INTERESTING_HEADER_NAMES in HttpHeader.cc
  enum InterestingHeader {
    ACCEPT_ENCODING,
    ACCESS_CONTROL_REQUEST_HEADERS,
    ACCESS_CONTROL_REQUEST_METHOD,
    AUTHORIZATION,
    CONNECTION,
    CONTENT_DISPOSITION,
    CONTENT_ENCODING,
    CONTENT_LENGTH,
    CONTENT_RANGE,
    CONTENT_TYPE,
    DIGEST,
    INFOHASH, // Used for BitTorrent LPD
    LAST_MODIFIED,
    LINK,
    LOCATION,
    ORIGIN,
    PORT, // Used for BitTorrent LPD
    RETRY_AFTER,
    SEC_WEBSOCKET_KEY,
    SEC_WEBSOCKET_VERSION,
    SET_COOKIE,
    TRANSFER_ENCODING,
    UPGRADE,
    MAX_INTERESTING_HEADER
  };

  // For all methods, use lowercased header field name.
  void put(int hdKey, const std::string& value);
  bool defined(int hdKey) const;
  const std::string& find(int hdKey) const;
  std::vector<std::string> findAll(int hdKey) const;
  std::pair<std::multimap<int, std::string>::const_iterator,
            std::multimap<int, std::string>::const_iterator>
  equalRange(int hdKey) const;

  void remove(int hdKey);

  Range getRange() const;

  int getStatusCode() const;

  void setStatusCode(int code);

  const std::string& getReasonPhrase() const;

  void setReasonPhrase(const std::string& reasonPhrase);

  const std::string& getVersion() const;

  void setVersion(const std::string& version);

  template <typename InputIterator>
  void setVersion(InputIterator first, InputIterator last)
  {
    version_.assign(first, last);
  }

  const std::string& getMethod() const;

  void setMethod(const std::string& method);

  template <typename InputIterator>
  void setMethod(InputIterator first, InputIterator last)
  {
    method_.assign(first, last);
  }

  const std::string& getRequestPath() const;

  void setRequestPath(const std::string& requestPath);

  template <typename InputIterator>
  void setRequestPath(InputIterator first, InputIterator last)
  {
    requestPath_.assign(first, last);
  }

  // Clears table_. responseStatus_ and version_ are unchanged.
  void clearField();

  // Returns true if header field |name| contains |value|. This method
  // assumes the values of the header field is delimited by ','.
  bool fieldContains(int hdKey, const char* value);

  // Returns true if the headers indicate that the remote endpoint
  // keeps connection open.
  bool isKeepAlive() const;
};

int idInterestingHeader(const char* hdName);

} // namespace aria2

#endif // D_HTTP_HEADER_H
