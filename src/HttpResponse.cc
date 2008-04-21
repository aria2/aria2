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
#include "HttpResponse.h"
#include "Request.h"
#include "Segment.h"
#include "CookieBox.h"
#include "HttpRequest.h"
#include "HttpHeader.h"
#include "Range.h"
#include "LogFactory.h"
#include "Logger.h"
#include "ChunkedEncoding.h"
#include "Util.h"
#include "message.h"
#include "DlAbortEx.h"
#include <deque>

namespace aria2 {

HttpResponse::HttpResponse():cuid(0),
			     logger(LogFactory::getInstance())
{}

HttpResponse::~HttpResponse() {}

void HttpResponse::validateResponse() const
{
  const std::string& status = getResponseStatus();
  if(status == "401") {
    throw new DlAbortEx(EX_AUTH_FAILED);
  }
  if(status == "404") {
    throw new DlAbortEx(MSG_RESOURCE_NOT_FOUND);
  }
  if(status >= "400") {
    throw new DlAbortEx(EX_BAD_STATUS, Util::parseUInt(status));
  }
  if(status >= "300") {
    if(!httpHeader->defined("Location")) {
      throw new DlAbortEx(EX_LOCATION_HEADER_REQUIRED, Util::parseUInt(status));
    }
  } else {
    if(!httpHeader->defined("Transfer-Encoding")) {
      // compare the received range against the requested range
      RangeHandle responseRange = httpHeader->getRange();
      if(!httpRequest->isRangeSatisfied(responseRange)) {
	throw new DlAbortEx(EX_INVALID_RANGE_HEADER,
			    Util::itos(httpRequest->getStartByte(), true).c_str(),
			    Util::itos(httpRequest->getEndByte(), true).c_str(),
			    Util::uitos(httpRequest->getEntityLength(), true).c_str(),
			    Util::itos(responseRange->getStartByte(), true).c_str(),
			    Util::itos(responseRange->getEndByte(), true).c_str(),
			    Util::uitos(responseRange->getEntityLength(), true).c_str());
      }
    }
  }
}

std::string HttpResponse::determinFilename() const
{
  std::string contentDisposition =
    Util::getContentDispositionFilename(httpHeader->getFirst("Content-Disposition"));
  if(contentDisposition.empty()) {
    return Util::urldecode(httpRequest->getFile());
  } else {
    logger->info(MSG_CONTENT_DISPOSITION_DETECTED,
		 cuid, contentDisposition.c_str());
    return Util::urldecode(contentDisposition);
  }
}

void HttpResponse::retrieveCookie()
{
  std::deque<std::string> v = httpHeader->get("Set-Cookie");
  for(std::deque<std::string>::const_iterator itr = v.begin(); itr != v.end(); itr++) {
    std::string domain = httpRequest->getHost();
    std::string path = httpRequest->getDir();
    httpRequest->getRequest()->cookieBox->add(*itr, domain, path);
  }
}

bool HttpResponse::isRedirect() const
{
  const std::string& status = getResponseStatus();
  return "300" <= status && status < "400" && httpHeader->defined("Location");
}

void HttpResponse::processRedirect()
{
  httpRequest->getRequest()->redirectUrl(getRedirectURI());

}

std::string HttpResponse::getRedirectURI() const
{
  return httpHeader->getFirst("Location");
}

bool HttpResponse::isTransferEncodingSpecified() const
{
  return httpHeader->defined("Transfer-Encoding");
}

std::string HttpResponse::getTransferEncoding() const
{
  return httpHeader->getFirst("Transfer-Encoding");
}

TransferEncodingHandle HttpResponse::getTransferDecoder() const
{
  if(isTransferEncodingSpecified()) {
    if(getTransferEncoding() == "chunked") {
      return SharedHandle<TransferEncoding>(new ChunkedEncoding());
    }
  }
  return SharedHandle<TransferEncoding>();
}

uint64_t HttpResponse::getContentLength() const
{
  if(httpHeader.isNull()) {
    return 0;
  } else {
    return httpHeader->getRange()->getContentLength();
  }
}

uint64_t HttpResponse::getEntityLength() const
{
  if(httpHeader.isNull()) {
    return 0;
  } else {
    return httpHeader->getRange()->getEntityLength();
  }
}

std::string HttpResponse::getContentType() const
{
  if(httpHeader.isNull()) {
    return "";
  } else {
    return httpHeader->getFirst("Content-Type");
  }
}

void HttpResponse::setHttpHeader(const SharedHandle<HttpHeader>& httpHeader)
{
  this->httpHeader = httpHeader;
}

SharedHandle<HttpHeader> HttpResponse::getHttpHeader() const
{
  return httpHeader;
}

void HttpResponse::setHttpRequest(const SharedHandle<HttpRequest>& httpRequest)
{
  this->httpRequest = httpRequest;
}

SharedHandle<HttpRequest> HttpResponse::getHttpRequest() const
{
  return httpRequest;
}

// TODO return std::string
const std::string& HttpResponse::getResponseStatus() const
{
  return httpHeader->getResponseStatus();
}

} // namespace aria2
