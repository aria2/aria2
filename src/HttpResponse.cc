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
#ifdef HAVE_LIBZ
# include "GZipDecodingStreamFilter.h"
#endif // HAVE_LIBZ

namespace aria2 {

HttpResponse::HttpResponse()
  : cuid_(0)
{}

HttpResponse::~HttpResponse() {}

void HttpResponse::validateResponse() const
{
  int statusCode = getStatusCode();
  if(statusCode >= 400) {
    return;
  }
  if(statusCode == 304) {
    if(httpRequest_->getIfModifiedSinceHeader().empty()) {
      throw DL_ABORT_EX2("Got 304 without If-Modified-Since",
                         error_code::HTTP_PROTOCOL_ERROR);
    }
  } else if(statusCode == 301 ||
            statusCode == 302 ||
            statusCode == 303 ||
            statusCode == 307) {
    if(!httpHeader_->defined(HttpHeader::LOCATION)) {
      throw DL_ABORT_EX2(fmt(EX_LOCATION_HEADER_REQUIRED, statusCode),
                         error_code::HTTP_PROTOCOL_ERROR);
    }
    return;
  } else if(statusCode == 200 || statusCode == 206) {
    if(!httpHeader_->defined(HttpHeader::TRANSFER_ENCODING)) {
      // compare the received range against the requested range
      RangeHandle responseRange = httpHeader_->getRange();
      if(!httpRequest_->isRangeSatisfied(responseRange)) {
        throw DL_ABORT_EX2
          (fmt(EX_INVALID_RANGE_HEADER,
               util::itos(httpRequest_->getStartByte(), true).c_str(),
               util::itos(httpRequest_->getEndByte(), true).c_str(),
               util::uitos(httpRequest_->getEntityLength(), true).c_str(),
               util::itos(responseRange->getStartByte(), true).c_str(),
               util::itos(responseRange->getEndByte(), true).c_str(),
               util::uitos(responseRange->getEntityLength(), true).c_str()),
           error_code::CANNOT_RESUME);
      }
    }
  } else {
    throw DL_ABORT_EX2(fmt("Unexpected status %d", statusCode),
                       error_code::HTTP_PROTOCOL_ERROR);
  }
}

std::string HttpResponse::determinFilename() const
{
  std::string contentDisposition =
    util::getContentDispositionFilename
    (httpHeader_->getFirst(HttpHeader::CONTENT_DISPOSITION));
  if(contentDisposition.empty()) {
    std::string file = util::percentDecode(httpRequest_->getFile());
    if(file.empty()) {
      return "index.html";
    } else {
      return file;
    }
  } else {
    A2_LOG_INFO(fmt(MSG_CONTENT_DISPOSITION_DETECTED,
                    cuid_,
                    contentDisposition.c_str()));
    return contentDisposition;
  }
}

void HttpResponse::retrieveCookie()
{
  Time now;
  std::vector<std::string> v = httpHeader_->get(HttpHeader::SET_COOKIE);
  for(std::vector<std::string>::const_iterator itr = v.begin(), eoi = v.end();
      itr != eoi; ++itr) {
    httpRequest_->getCookieStorage()->parseAndStore
      (*itr, httpRequest_->getHost(), httpRequest_->getDir(), now.getTime());
  }
}

bool HttpResponse::isRedirect() const
{
  int statusCode = getStatusCode();
  return (301 == statusCode ||
          302 == statusCode ||
          303 == statusCode ||
          307 == statusCode) &&
    httpHeader_->defined(HttpHeader::LOCATION);
}

void HttpResponse::processRedirect()
{
  
  if(httpRequest_->getRequest()->redirectUri(getRedirectURI())) {
    A2_LOG_INFO(fmt(MSG_REDIRECT,
                    cuid_,
                    httpRequest_->getRequest()->getCurrentUri().c_str()));
  } else {
    throw DL_RETRY_EX
      (fmt("CUID#%lld - Redirect to %s failed. It may not be a valid URI.",
           cuid_,
           httpRequest_->getRequest()->getCurrentUri().c_str()));
  }
}

std::string HttpResponse::getRedirectURI() const
{
  return httpHeader_->getFirst(HttpHeader::LOCATION);
}

bool HttpResponse::isTransferEncodingSpecified() const
{
  return httpHeader_->defined(HttpHeader::TRANSFER_ENCODING);
}

std::string HttpResponse::getTransferEncoding() const
{
  // TODO See TODO in getTransferEncodingStreamFilter()
  return httpHeader_->getFirst(HttpHeader::TRANSFER_ENCODING);
}

SharedHandle<StreamFilter> HttpResponse::getTransferEncodingStreamFilter() const
{
  SharedHandle<StreamFilter> filter;
  // TODO Transfer-Encoding header field can contains multiple tokens. We should
  // parse the field and retrieve each token.
  if(isTransferEncodingSpecified()) {
    if(getTransferEncoding() == HttpHeader::CHUNKED) {
      filter.reset(new ChunkedDecodingStreamFilter());
    }
  }
  return filter;
}

bool HttpResponse::isContentEncodingSpecified() const
{
  return httpHeader_->defined(HttpHeader::CONTENT_ENCODING);
}

const std::string& HttpResponse::getContentEncoding() const
{
  return httpHeader_->getFirst(HttpHeader::CONTENT_ENCODING);
}

SharedHandle<StreamFilter> HttpResponse::getContentEncodingStreamFilter() const
{
  SharedHandle<StreamFilter> filter;
#ifdef HAVE_LIBZ
  if(getContentEncoding() == HttpHeader::GZIP ||
     getContentEncoding() == HttpHeader::DEFLATE) {
    filter.reset(new GZipDecodingStreamFilter());
  }
#endif // HAVE_LIBZ
  return filter;
}

uint64_t HttpResponse::getContentLength() const
{
  if(!httpHeader_) {
    return 0;
  } else {
    return httpHeader_->getRange()->getContentLength();
  }
}

uint64_t HttpResponse::getEntityLength() const
{
  if(!httpHeader_) {
    return 0;
  } else {
    return httpHeader_->getRange()->getEntityLength();
  }
}

std::string HttpResponse::getContentType() const
{
  if(!httpHeader_) {
    return A2STR::NIL;
  } else {
    std::pair<std::string, std::string> p;
    util::divide(p, httpHeader_->getFirst(HttpHeader::CONTENT_TYPE), ';');
    return p.first;
  }
}

void HttpResponse::setHttpHeader(const SharedHandle<HttpHeader>& httpHeader)
{
  httpHeader_ = httpHeader;
}

void HttpResponse::setHttpRequest(const SharedHandle<HttpRequest>& httpRequest)
{
  httpRequest_ = httpRequest;
}

int HttpResponse::getStatusCode() const
{
  return httpHeader_->getStatusCode();
}

bool HttpResponse::hasRetryAfter() const
{
  return httpHeader_->defined(HttpHeader::RETRY_AFTER);
}

time_t HttpResponse::getRetryAfter() const
{
  return httpHeader_->getFirstAsUInt(HttpHeader::RETRY_AFTER);
}

Time HttpResponse::getLastModifiedTime() const
{
  return Time::parseHTTPDate(httpHeader_->getFirst(HttpHeader::LAST_MODIFIED));
}

bool HttpResponse::supportsPersistentConnection() const
{
  std::string connection =
    util::toLower(httpHeader_->getFirst(HttpHeader::CONNECTION));
  std::string version = httpHeader_->getVersion();

  return
    connection.find(HttpHeader::CLOSE) == std::string::npos &&
    (version == HttpHeader::HTTP_1_1 ||
     connection.find("keep-alive") != std::string::npos) &&
    (!httpRequest_->isProxyRequestSet() ||
     util::toLower(httpHeader_->getFirst("Proxy-Connection")).find("keep-alive")
     != std::string::npos);
}

} // namespace aria2
