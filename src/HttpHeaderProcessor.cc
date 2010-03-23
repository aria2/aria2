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
#include "HttpHeaderProcessor.h"

#include <sstream>
#include <vector>

#include "HttpHeader.h"
#include "message.h"
#include "util.h"
#include "DlRetryEx.h"
#include "DlAbortEx.h"
#include "A2STR.h"

namespace aria2 {

HttpHeaderProcessor::HttpHeaderProcessor():
  _limit(21/*lines*/*8190/*per line*/) {}
// The above values come from Apache's documentation
// http://httpd.apache.org/docs/2.2/en/mod/core.html: See
// LimitRequestFieldSize and LimitRequestLine directive.  Also the
// page states that the number of request fields rarely exceeds 20.
// aria2 uses this class in both client and server side.

HttpHeaderProcessor::~HttpHeaderProcessor() {}

void HttpHeaderProcessor::update(const unsigned char* data, size_t length)
{
  checkHeaderLimit(length);
  _buf += std::string(&data[0], &data[length]);
}

void HttpHeaderProcessor::update(const std::string& data)
{
  checkHeaderLimit(data.size());
  _buf += data;
}

void HttpHeaderProcessor::checkHeaderLimit(size_t incomingLength)
{
  if(_buf.size()+incomingLength > _limit) {
    throw DL_ABORT_EX("Too large http header");
  }
}

bool HttpHeaderProcessor::eoh() const
{
  if(_buf.find("\r\n\r\n") == std::string::npos &&
     _buf.find("\n\n") == std::string::npos) {
    return false;
  } else {
    return true;
  }
}

size_t HttpHeaderProcessor::getPutBackDataLength() const
{
  std::string::size_type delimpos = std::string::npos;
  if((delimpos = _buf.find("\r\n\r\n")) != std::string::npos) {
    return _buf.size()-(delimpos+4);
  } else if((delimpos = _buf.find("\n\n")) != std::string::npos) {
    return _buf.size()-(delimpos+2);
  } else {
    return 0;
  }
}

void HttpHeaderProcessor::clear()
{
  _buf.erase();
}

SharedHandle<HttpHeader> HttpHeaderProcessor::getHttpResponseHeader()
{
  std::string::size_type delimpos = std::string::npos;
  if(((delimpos = _buf.find("\r\n")) == std::string::npos &&
      (delimpos = _buf.find("\n")) == std::string::npos) ||
     delimpos < 12) {
    throw DL_RETRY_EX(EX_NO_STATUS_HEADER);
  }
  HttpHeaderHandle httpHeader(new HttpHeader());
  httpHeader->setVersion(_buf.substr(0, 8));
  httpHeader->setResponseStatus(_buf.substr(9, 3));
  std::istringstream strm(_buf);
  // TODO 1st line(HTTP/1.1 200...) is also send to HttpHeader, but it should
  // not.
  httpHeader->fill(strm);
  return httpHeader;
}

SharedHandle<HttpHeader> HttpHeaderProcessor::getHttpRequestHeader()
{
  // The minimum case of the first line is:
  // GET / HTTP/1.x
  // At least 14bytes before \r\n or \n.
  std::string::size_type delimpos = std::string::npos;
  if(((delimpos = _buf.find("\r\n")) == std::string::npos &&
      (delimpos = _buf.find("\n")) == std::string::npos) ||
     delimpos < 14) {
    throw DL_RETRY_EX(EX_NO_STATUS_HEADER);
  }
  std::vector<std::string> firstLine;
  util::split(_buf.substr(0, delimpos), std::back_inserter(firstLine)," ",true);
  if(firstLine.size() != 3) {
    throw DL_ABORT_EX("Malformed HTTP request header.");    
  }
  SharedHandle<HttpHeader> httpHeader(new HttpHeader());
  httpHeader->setMethod(firstLine[0]);
  httpHeader->setRequestPath(firstLine[1]);
  httpHeader->setVersion(firstLine[2]);
  std::istringstream strm(_buf.substr(delimpos));
  httpHeader->fill(strm);
  return httpHeader;
}

std::string HttpHeaderProcessor::getHeaderString() const
{
  std::string::size_type delimpos = std::string::npos;
  if((delimpos = _buf.find("\r\n\r\n")) == std::string::npos &&
     (delimpos = _buf.find("\n\n")) == std::string::npos) {
    return _buf;
  } else {
    return _buf.substr(0, delimpos);
  }
}

} // namespace aria2
