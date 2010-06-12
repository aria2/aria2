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
#include "StringFormat.h"
#include "A2STR.h"
#include "Decoder.h"
#include "ChunkedDecoder.h"
#ifdef HAVE_LIBZ
# include "GZipDecoder.h"
#endif // HAVE_LIBZ
#include "CookieStorage.h"
#include "AuthConfigFactory.h"
#include "AuthConfig.h"

namespace aria2 {

HttpResponse::HttpResponse():_cuid(0),
                             _logger(LogFactory::getInstance())
{}

HttpResponse::~HttpResponse() {}

void HttpResponse::validateResponse() const
{
  const std::string& status = getResponseStatus();
  if(status >= HttpHeader::S400) {
    return;
  }
  if(status >= HttpHeader::S300) {
    if(!_httpHeader->defined(HttpHeader::LOCATION)) {
      throw DL_ABORT_EX
        (StringFormat(EX_LOCATION_HEADER_REQUIRED,
                      util::parseUInt(status)).str());
    }
  } else if(!_httpHeader->defined(HttpHeader::TRANSFER_ENCODING)) {
    // compare the received range against the requested range
    RangeHandle responseRange = _httpHeader->getRange();
    if(!_httpRequest->isRangeSatisfied(responseRange)) {
      throw DL_ABORT_EX2
        (StringFormat
         (EX_INVALID_RANGE_HEADER,
          util::itos(_httpRequest->getStartByte(), true).c_str(),
          util::itos(_httpRequest->getEndByte(), true).c_str(),
          util::uitos(_httpRequest->getEntityLength(), true).c_str(),
          util::itos(responseRange->getStartByte(), true).c_str(),
          util::itos(responseRange->getEndByte(), true).c_str(),
          util::uitos(responseRange->getEntityLength(), true).c_str()).str(),
         downloadresultcode::CANNOT_RESUME);
    }
  }
}

std::string HttpResponse::determinFilename() const
{
  std::string contentDisposition =
    util::getContentDispositionFilename
    (_httpHeader->getFirst(HttpHeader::CONTENT_DISPOSITION));
  if(contentDisposition.empty()) {
    std::string file = util::percentDecode(_httpRequest->getFile());
    if(file.empty()) {
      return "index.html";
    } else {
      return file;
    }
  } else {
    if(_logger->info()) {
      _logger->info(MSG_CONTENT_DISPOSITION_DETECTED,
                    util::itos(_cuid).c_str(), contentDisposition.c_str());
    }
    return contentDisposition;
  }
}

void HttpResponse::retrieveCookie()
{
  std::vector<std::string> v = _httpHeader->get(HttpHeader::SET_COOKIE);
  for(std::vector<std::string>::const_iterator itr = v.begin(), eoi = v.end();
      itr != eoi; ++itr) {
    _httpRequest->getCookieStorage()->parseAndStore(*itr,
                                                    _httpRequest->getHost(),
                                                    _httpRequest->getDir());
  }
}

bool HttpResponse::isRedirect() const
{
  const std::string& status = getResponseStatus();
  return HttpHeader::S300 <= status && status < HttpHeader::S400 &&
    _httpHeader->defined(HttpHeader::LOCATION);
}

void HttpResponse::processRedirect()
{
  
  if(_httpRequest->getRequest()->redirectUri(getRedirectURI())) {
    if(_logger->info()) {
      _logger->info(MSG_REDIRECT, util::itos(_cuid).c_str(),
                    _httpRequest->getRequest()->getCurrentUri().c_str());
    }
  } else {
    throw DL_RETRY_EX
      (StringFormat("CUID#%s - Redirect to %s failed. It may not be a valid"
                    " URI.", util::itos(_cuid).c_str(),
                    _httpRequest->getRequest()->getCurrentUri().c_str()).str());
  }
}

std::string HttpResponse::getRedirectURI() const
{
  return _httpHeader->getFirst(HttpHeader::LOCATION);
}

bool HttpResponse::isTransferEncodingSpecified() const
{
  return _httpHeader->defined(HttpHeader::TRANSFER_ENCODING);
}

std::string HttpResponse::getTransferEncoding() const
{
  // TODO See TODO in getTransferEncodingDecoder()
  return _httpHeader->getFirst(HttpHeader::TRANSFER_ENCODING);
}

SharedHandle<Decoder> HttpResponse::getTransferEncodingDecoder() const
{
  // TODO Transfer-Encoding header field can contains multiple tokens. We should
  // parse the field and retrieve each token.
  if(isTransferEncodingSpecified()) {
    if(getTransferEncoding() == HttpHeader::CHUNKED) {
      return SharedHandle<Decoder>(new ChunkedDecoder());
    }
  }
  return SharedHandle<Decoder>();
}

bool HttpResponse::isContentEncodingSpecified() const
{
  return _httpHeader->defined(HttpHeader::CONTENT_ENCODING);
}

const std::string& HttpResponse::getContentEncoding() const
{
  return _httpHeader->getFirst(HttpHeader::CONTENT_ENCODING);
}

SharedHandle<Decoder> HttpResponse::getContentEncodingDecoder() const
{
#ifdef HAVE_LIBZ
  if(getContentEncoding() == HttpHeader::GZIP ||
     getContentEncoding() == HttpHeader::DEFLATE) {
    return SharedHandle<Decoder>(new GZipDecoder());
  }
#endif // HAVE_LIBZ
  return SharedHandle<Decoder>();
}

uint64_t HttpResponse::getContentLength() const
{
  if(_httpHeader.isNull()) {
    return 0;
  } else {
    return _httpHeader->getRange()->getContentLength();
  }
}

uint64_t HttpResponse::getEntityLength() const
{
  if(_httpHeader.isNull()) {
    return 0;
  } else {
    return _httpHeader->getRange()->getEntityLength();
  }
}

std::string HttpResponse::getContentType() const
{
  if(_httpHeader.isNull()) {
    return A2STR::NIL;
  } else {
    return
      util::split(_httpHeader->getFirst(HttpHeader::CONTENT_TYPE), ";").first;
  }
}

void HttpResponse::setHttpHeader(const SharedHandle<HttpHeader>& httpHeader)
{
  _httpHeader = httpHeader;
}

void HttpResponse::setHttpRequest(const SharedHandle<HttpRequest>& httpRequest)
{
  _httpRequest = httpRequest;
}

// TODO return std::string
const std::string& HttpResponse::getResponseStatus() const
{
  return _httpHeader->getResponseStatus();
}

bool HttpResponse::hasRetryAfter() const
{
  return _httpHeader->defined(HttpHeader::RETRY_AFTER);
}

time_t HttpResponse::getRetryAfter() const
{
  return _httpHeader->getFirstAsUInt(HttpHeader::RETRY_AFTER);
}

Time HttpResponse::getLastModifiedTime() const
{
  return Time::parseHTTPDate(_httpHeader->getFirst(HttpHeader::LAST_MODIFIED));
}

bool HttpResponse::supportsPersistentConnection() const
{
  std::string connection =
    util::toLower(_httpHeader->getFirst(HttpHeader::CONNECTION));
  std::string version = _httpHeader->getVersion();

  return
    connection.find(HttpHeader::CLOSE) == std::string::npos &&
    (version == HttpHeader::HTTP_1_1 ||
     connection.find("keep-alive") != std::string::npos) &&
    (!_httpRequest->isProxyRequestSet() ||
     util::toLower(_httpHeader->getFirst("Proxy-Connection")).find("keep-alive")
     != std::string::npos);
}

} // namespace aria2
