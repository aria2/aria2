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
#ifndef D_HTTP_RESPONSE_H
#define D_HTTP_RESPONSE_H

#include "common.h"

#include <stdint.h>
#include <vector>
#include <memory>

#include "TimeA2.h"
#include "Command.h"

namespace aria2 {

class HttpRequest;
class HttpHeader;
class StreamFilter;
struct MetalinkHttpEntry;
class Option;
class Checksum;

class HttpResponse {
private:
  cuid_t cuid_;
  std::unique_ptr<HttpRequest> httpRequest_;
  std::unique_ptr<HttpHeader> httpHeader_;

public:
  HttpResponse();

  void validateResponse() const;

  /**
   * Returns filename.
   * If content-disposition header is privided in response header,
   * this function returns the filename from it.
   * If it is not there, returns the part of filename from the request URL.
   */
  std::string determineFilename(bool contentDispositionUTF8) const;

  void retrieveCookie();

  /**
   * Returns true if the response header indicates redirection.
   */
  bool isRedirect() const;

  void processRedirect();

  const std::string& getRedirectURI() const;

  bool isTransferEncodingSpecified() const;

  const std::string& getTransferEncoding() const;

  std::unique_ptr<StreamFilter> getTransferEncodingStreamFilter() const;

  bool isContentEncodingSpecified() const;

  const std::string& getContentEncoding() const;

  std::unique_ptr<StreamFilter> getContentEncodingStreamFilter() const;

  int64_t getContentLength() const;

  int64_t getEntityLength() const;

  // Returns type "/" subtype. The parameter is removed.
  std::string getContentType() const;

  void setHttpHeader(std::unique_ptr<HttpHeader> httpHeader);

  const std::unique_ptr<HttpHeader>& getHttpHeader() const;

  int getStatusCode() const;

  void setHttpRequest(std::unique_ptr<HttpRequest> httpRequest);

  const std::unique_ptr<HttpRequest>& getHttpRequest() const
  {
    return httpRequest_;
  }

  void setCuid(cuid_t cuid) { cuid_ = cuid; }

  Time getLastModifiedTime() const;

  bool supportsPersistentConnection() const;

  void getMetalinKHttpEntries(std::vector<MetalinkHttpEntry>& result,
                              const std::shared_ptr<Option>& option) const;

  // Returns all digests specified in Digest header field.  Sort
  // strong algorithm first. Strength is defined in MessageDigest. If
  // several same digest algorithms are available, but they have
  // different value, they are all ignored.
  void getDigest(std::vector<Checksum>& result) const;
};

} // namespace aria2

#endif // D_HTTP_RESPONSE_H
