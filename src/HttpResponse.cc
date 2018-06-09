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

#include "HttpResponse.h"
#include "Request.h"
#include "Segment.h"
#include "HttpRequest.h"
#include "HttpHeader.h"
#include "Range.h"
#include "LogFactory.h"
#include "Logger.h"
#include "util.h"
#include "message.h"
#include "DlAbortEx.h"
#include "DlRetryEx.h"
#include "fmt.h"
#include "A2STR.h"
#include "CookieStorage.h"
#include "AuthConfigFactory.h"
#include "AuthConfig.h"
#include "ChunkedDecodingStreamFilter.h"
#include "error_code.h"
#include "prefs.h"
#include "Option.h"
#include "Checksum.h"
#include "uri.h"
#include "MetalinkHttpEntry.h"
#include "base64.h"
#include "array_fun.h"
#include "MessageDigest.h"
#ifdef HAVE_ZLIB
#  include "GZipDecodingStreamFilter.h"
#endif // HAVE_ZLIB

namespace aria2 {

HttpResponse::HttpResponse() : cuid_{0} {}

void HttpResponse::validateResponse() const
{
  int statusCode = getStatusCode();
  switch (statusCode) {
  case 200: // OK
  case 206: // Partial Content
    if (!httpHeader_->defined(HttpHeader::TRANSFER_ENCODING)) {
      // compare the received range against the requested range
      auto responseRange = httpHeader_->getRange();
      if (!httpRequest_->isRangeSatisfied(responseRange)) {
        throw DL_ABORT_EX2(
            fmt(EX_INVALID_RANGE_HEADER, httpRequest_->getStartByte(),
                httpRequest_->getEndByte(), httpRequest_->getEntityLength(),
                responseRange.startByte, responseRange.endByte,
                responseRange.entityLength),
            error_code::CANNOT_RESUME);
      }
    }
    return;
  case 304: // Not Modified
    if (!httpRequest_->conditionalRequest()) {
      throw DL_ABORT_EX2("Got 304 without If-Modified-Since or If-None-Match",
                         error_code::HTTP_PROTOCOL_ERROR);
    }
    return;
  case 300: // Multiple Choices
  case 301: // Moved Permanently
  case 302: // Found
  case 303: // See Other
  case 307: // Temporary Redirect
  case 308: // Permanent Redirect
    if (!httpHeader_->defined(HttpHeader::LOCATION)) {
      throw DL_ABORT_EX2(fmt(EX_LOCATION_HEADER_REQUIRED, statusCode),
                         error_code::HTTP_PROTOCOL_ERROR);
    }
    return;
  }
  if (statusCode >= 400) {
    return;
  }
  throw DL_ABORT_EX2(fmt("Unexpected status %d", statusCode),
                     error_code::HTTP_PROTOCOL_ERROR);
}

std::string HttpResponse::determineFilename(bool contentDispositionUTF8) const
{
  std::string contentDisposition = util::getContentDispositionFilename(
      httpHeader_->find(HttpHeader::CONTENT_DISPOSITION),
      contentDispositionUTF8);
  if (contentDisposition.empty()) {
    auto file = httpRequest_->getFile();
    file = util::percentDecode(file.begin(), file.end());
    if (file.empty()) {
      return Request::DEFAULT_FILE;
    }
    return file;
  }

  A2_LOG_INFO(
      fmt(MSG_CONTENT_DISPOSITION_DETECTED, cuid_, contentDisposition.c_str()));
  return contentDisposition;
}

void HttpResponse::retrieveCookie()
{
  Time now;
  auto r = httpHeader_->equalRange(HttpHeader::SET_COOKIE);
  for (; r.first != r.second; ++r.first) {
    httpRequest_->getCookieStorage()->parseAndStore(
        (*r.first).second, httpRequest_->getHost(), httpRequest_->getDir(),
        now.getTimeFromEpoch());
  }
}

bool HttpResponse::isRedirect() const
{
  switch (getStatusCode()) {
  case 300: // Multiple Choices
  case 301: // Moved Permanently
  case 302: // Found
  case 303: // See Other
  case 307: // Temporary Redirect
  case 308: // Permanent Redirect
    return httpHeader_->defined(HttpHeader::LOCATION);
  }
  return false;
}

void HttpResponse::processRedirect()
{
  const auto& req = httpRequest_->getRequest();
  if (!req->redirectUri(util::percentEncodeMini(getRedirectURI()))) {
    throw DL_RETRY_EX(fmt(
        "CUID#%" PRId64 " - Redirect to %s failed. It may not be a valid URI.",
        cuid_, req->getCurrentUri().c_str()));
  }

  A2_LOG_NOTICE(fmt(MSG_REDIRECT, cuid_,
                    httpRequest_->getRequest()->getCurrentUri().c_str()));
}

const std::string& HttpResponse::getRedirectURI() const
{
  return httpHeader_->find(HttpHeader::LOCATION);
}

bool HttpResponse::isTransferEncodingSpecified() const
{
  return httpHeader_->defined(HttpHeader::TRANSFER_ENCODING);
}

const std::string& HttpResponse::getTransferEncoding() const
{
  // TODO See TODO in getTransferEncodingStreamFilter()
  return httpHeader_->find(HttpHeader::TRANSFER_ENCODING);
}

std::unique_ptr<StreamFilter>
HttpResponse::getTransferEncodingStreamFilter() const
{
  // TODO Transfer-Encoding header field can contains multiple tokens. We should
  // parse the field and retrieve each token.
  if (isTransferEncodingSpecified()) {
    if (util::strieq(getTransferEncoding(), "chunked")) {
      return make_unique<ChunkedDecodingStreamFilter>();
    }
  }
  return nullptr;
}

bool HttpResponse::isContentEncodingSpecified() const
{
  return httpHeader_->defined(HttpHeader::CONTENT_ENCODING);
}

const std::string& HttpResponse::getContentEncoding() const
{
  return httpHeader_->find(HttpHeader::CONTENT_ENCODING);
}

std::unique_ptr<StreamFilter>
HttpResponse::getContentEncodingStreamFilter() const
{
#ifdef HAVE_ZLIB
  if (util::strieq(getContentEncoding(), "gzip") ||
      util::strieq(getContentEncoding(), "deflate")) {
    return make_unique<GZipDecodingStreamFilter>();
  }
#endif // HAVE_ZLIB

  return nullptr;
}

int64_t HttpResponse::getContentLength() const
{
  if (!httpHeader_) {
    return 0;
  }

  return httpHeader_->getRange().getContentLength();
}

int64_t HttpResponse::getEntityLength() const
{
  if (!httpHeader_) {
    return 0;
  }

  return httpHeader_->getRange().entityLength;
}

std::string HttpResponse::getContentType() const
{
  if (!httpHeader_) {
    return A2STR::NIL;
  }

  const auto& ctype = httpHeader_->find(HttpHeader::CONTENT_TYPE);
  auto i = std::find(ctype.begin(), ctype.end(), ';');
  Scip p = util::stripIter(ctype.begin(), i);
  return std::string(p.first, p.second);
}

void HttpResponse::setHttpHeader(std::unique_ptr<HttpHeader> httpHeader)
{
  httpHeader_ = std::move(httpHeader);
}

const std::unique_ptr<HttpHeader>& HttpResponse::getHttpHeader() const
{
  return httpHeader_;
}

void HttpResponse::setHttpRequest(std::unique_ptr<HttpRequest> httpRequest)
{
  httpRequest_ = std::move(httpRequest);
}

int HttpResponse::getStatusCode() const { return httpHeader_->getStatusCode(); }

Time HttpResponse::getLastModifiedTime() const
{
  return Time::parseHTTPDate(httpHeader_->find(HttpHeader::LAST_MODIFIED));
}

bool HttpResponse::supportsPersistentConnection() const
{
  return httpHeader_->isKeepAlive();
}

namespace {

bool parseMetalinkHttpLink(MetalinkHttpEntry& result, const std::string& s)
{
  const auto first = std::find(s.begin(), s.end(), '<');
  if (first == s.end()) {
    return false;
  }

  auto last = std::find(first, s.end(), '>');
  if (last == s.end()) {
    return false;
  }

  auto p = util::stripIter(first + 1, last);
  if (p.first == p.second) {
    return false;
  }

  result.uri.assign(p.first, p.second);
  last = std::find(last, s.end(), ';');
  if (last != s.end()) {
    ++last;
  }
  bool ok = false;
  while (1) {
    std::string name, value;
    auto r = util::nextParam(name, value, last, s.end(), ';');
    last = r.first;

    if (!r.second) {
      break;
    }

    if (value.empty()) {
      if (name == "pref") {
        result.pref = true;
      }
      continue;
    }

    if (name == "rel") {
      if (value == "duplicate") {
        ok = true;
      }
      else {
        ok = false;
      }
      continue;
    }

    if (name == "pri") {
      int32_t priValue;
      if (util::parseIntNoThrow(priValue, value)) {
        if (1 <= priValue && priValue <= 999999) {
          result.pri = priValue;
        }
      }
      continue;
    }

    if (name == "geo") {
      util::lowercase(value);
      result.geo = value;
      continue;
    }
  }
  return ok;
}

} // namespace

// Metalink/HTTP is defined by http://tools.ietf.org/html/rfc6249.
// Link header field is defined by http://tools.ietf.org/html/rfc5988.
void HttpResponse::getMetalinKHttpEntries(
    std::vector<MetalinkHttpEntry>& result,
    const std::shared_ptr<Option>& option) const
{
  auto p = httpHeader_->equalRange(HttpHeader::LINK);
  for (; p.first != p.second; ++p.first) {
    MetalinkHttpEntry e;
    if (parseMetalinkHttpLink(e, (*p.first).second)) {
      result.push_back(e);
    }
  }

  if (!result.empty()) {
    std::vector<std::string> locs;
    if (option->defined(PREF_METALINK_LOCATION)) {
      const std::string& loc = option->get(PREF_METALINK_LOCATION);
      util::split(loc.begin(), loc.end(), std::back_inserter(locs), ',', true);
      for (auto& l : locs) {
        util::lowercase(l);
      }
    }
    for (auto& r : result) {
      if (std::find(locs.begin(), locs.end(), r.geo) != locs.end()) {
        r.pri -= 999999;
      }
    }
  }

  std::sort(result.begin(), result.end());
}

// Digest header field is defined by
// http://tools.ietf.org/html/rfc3230.
void HttpResponse::getDigest(std::vector<Checksum>& result) const
{
  auto p = httpHeader_->equalRange(HttpHeader::DIGEST);
  for (; p.first != p.second; ++p.first) {
    const std::string& s = (*p.first).second;
    std::string::const_iterator itr = s.begin();
    while (1) {
      std::string hashType, digest;
      auto r = util::nextParam(hashType, digest, itr, s.end(), ',');
      itr = r.first;
      if (!r.second) {
        break;
      }

      util::lowercase(hashType);
      digest = base64::decode(digest.begin(), digest.end());
      if (!MessageDigest::supports(hashType) ||
          MessageDigest::getDigestLength(hashType) != digest.size()) {
        continue;
      }

      result.push_back(Checksum(hashType, digest));
    }
  }

  std::sort(result.begin(), result.end(), HashTypeStronger());
  std::vector<Checksum> temp;
  for (auto i = result.begin(), eoi = result.end(); i != eoi;) {
    bool ok = true;
    auto j = i + 1;
    for (; j != eoi; ++j) {
      if ((*i).getHashType() != (*j).getHashType()) {
        break;
      }

      if ((*i).getDigest() != (*j).getDigest()) {
        ok = false;
      }
    }

    if (ok) {
      temp.push_back(*i);
    }
    i = j;
  }
  std::swap(temp, result);
}

} // namespace aria2
