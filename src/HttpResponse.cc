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
#include "ChunkedEncoding.h"
#include "Util.h"
#include "message.h"

void HttpResponse::validateResponse() const
  throw(DlAbortEx*, DlRetryEx*)
{
  if(status == 401) {
    throw new DlAbortEx(EX_AUTH_FAILED);
  }
  if(status >= 400) {
    throw new DlRetryEx(EX_BAD_STATUS, status);
  }
  if(status >= 300) {
    if(!httpHeader->defined("Location")) {
      throw new DlRetryEx(EX_LOCATION_HEADER_REQUIRED,
			  status);
    }
  } else {
    if(!httpHeader->defined("Transfer-Encoding")) {
      // compare the received range against the requested range
      RangeHandle responseRange = httpHeader->getRange();
      if(!httpRequest->isRangeSatisfied(responseRange)) {
	throw new DlRetryEx(EX_INVALID_RANGE_HEADER,
			    Util::llitos(httpRequest->getStartByte(), true).c_str(),
			    Util::llitos(httpRequest->getEndByte(), true).c_str(),
			    Util::llitos(httpRequest->getEntityLength(), true).c_str(),
			    Util::llitos(responseRange->getStartByte(), true).c_str(),
			    Util::llitos(responseRange->getEndByte(), true).c_str(),
			    Util::llitos(responseRange->getEntityLength(), true).c_str());
      }
    }
  }
}

string HttpResponse::determinFilename() const
{
  string contentDisposition =
    Util::getContentDispositionFilename(httpHeader->getFirst("Content-Disposition"));
  if(contentDisposition.empty()) {
    return Util::urldecode(httpRequest->getRequest()->getFile());
  } else {
    logger->info(MSG_CONTENT_DISPOSITION_DETECTED,
		 cuid, contentDisposition.c_str());
    return Util::urldecode(contentDisposition);
  }
}

void HttpResponse::retrieveCookie()
{
  Strings v = httpHeader->get("Set-Cookie");
  for(Strings::const_iterator itr = v.begin(); itr != v.end(); itr++) {
    httpRequest->getRequest()->cookieBox->add(*itr);
  }
}

bool HttpResponse::isRedirect() const
{
  return 300 <= status && status < 400 && httpHeader->defined("Location");
}

void HttpResponse::processRedirect()
{
  httpRequest->getRequest()->redirectUrl(getRedirectURI());

}

string HttpResponse::getRedirectURI() const
{
  return httpHeader->getFirst("Location");
}

bool HttpResponse::isTransferEncodingSpecified() const
{
  return httpHeader->defined("Transfer-Encoding");
}

string HttpResponse::getTransferEncoding() const
{
  return httpHeader->getFirst("Transfer-Encoding");
}

TransferEncodingHandle HttpResponse::getTransferDecoder() const
{
  if(isTransferEncodingSpecified()) {
    if(getTransferEncoding() == "chunked") {
      return new ChunkedEncoding();
    }
  }
  return 0;
}

int64_t HttpResponse::getContentLength() const
{
  if(httpHeader.isNull()) {
    return 0;
  } else {
    return httpHeader->getRange()->getContentLength();
  }
}

int64_t HttpResponse::getEntityLength() const
{
  if(httpHeader.isNull()) {
    return 0;
  } else {
    return httpHeader->getRange()->getEntityLength();
  }
}
