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
#include "HttpHeader.h"

#include <istream>

#include "Range.h"
#include "util.h"
#include "A2STR.h"

namespace aria2 {

const std::string HttpHeader::LOCATION("Location");

const std::string HttpHeader::TRANSFER_ENCODING("Transfer-Encoding");

const std::string HttpHeader::CONTENT_ENCODING("Content-Encoding");

const std::string HttpHeader::CONTENT_DISPOSITION("Content-Disposition");

const std::string HttpHeader::SET_COOKIE("Set-Cookie");

const std::string HttpHeader::CHUNKED("chunked");

const std::string HttpHeader::GZIP("gzip");

const std::string HttpHeader::DEFLATE("deflate");

const std::string HttpHeader::CONTENT_TYPE("Content-Type");

const std::string HttpHeader::RETRY_AFTER("Retry-After");

const std::string HttpHeader::CONNECTION("Connection");

const std::string HttpHeader::CLOSE("close");

const std::string HttpHeader::CONTENT_LENGTH("Content-Length");

const std::string HttpHeader::CONTENT_RANGE("Content-Range");

const std::string HttpHeader::LAST_MODIFIED("Last-Modified");

const std::string HttpHeader::ACCEPT_ENCODING("Accept-Encoding");

const std::string HttpHeader::HTTP_1_1("HTTP/1.1");

const std::string HttpHeader::S200("200");

const std::string HttpHeader::S206("206");

const std::string HttpHeader::S300("300");

const std::string HttpHeader::S301("301");

const std::string HttpHeader::S302("302");

const std::string HttpHeader::S303("303");

const std::string HttpHeader::S304("304");

const std::string HttpHeader::S307("307");

const std::string HttpHeader::S400("400");

const std::string HttpHeader::S401("401");
  
const std::string HttpHeader::S404("404");

HttpHeader::HttpHeader() {}
HttpHeader::~HttpHeader() {}

void HttpHeader::put(const std::string& name, const std::string& value) {
  std::multimap<std::string, std::string>::value_type vt
    (util::toLower(name), value);
  table_.insert(vt);
}

bool HttpHeader::defined(const std::string& name) const {
  return table_.count(util::toLower(name)) >= 1;
}

const std::string& HttpHeader::getFirst(const std::string& name) const {
  std::multimap<std::string, std::string>::const_iterator itr =
    table_.find(util::toLower(name));
  if(itr == table_.end()) {
    return A2STR::NIL;
  } else {
    return (*itr).second;
  }
}

std::vector<std::string> HttpHeader::get(const std::string& name) const
{
  std::vector<std::string> v;
  std::string n(util::toLower(name));
  std::pair<std::multimap<std::string, std::string>::const_iterator,
    std::multimap<std::string, std::string>::const_iterator> itrpair =
    table_.equal_range(n);
  std::multimap<std::string, std::string>::const_iterator first = itrpair.first;
  while(first != itrpair.second) {
    v.push_back((*first).second);
    ++first;
  }
  return v;
}

unsigned int HttpHeader::getFirstAsUInt(const std::string& name) const {
  return getFirstAsULLInt(name);
}

uint64_t HttpHeader::getFirstAsULLInt(const std::string& name) const {
  const std::string& value = getFirst(name);
  if(value.empty()) {
    return 0;
  } else {
    return util::parseULLInt(value);
  }
}

RangeHandle HttpHeader::getRange() const
{
  const std::string& rangeStr = getFirst(CONTENT_RANGE);
  if(rangeStr.empty()) {
    const std::string& contentLengthStr = getFirst(CONTENT_LENGTH);
    if(contentLengthStr.empty()) {
      return SharedHandle<Range>(new Range());
    } else {
      uint64_t contentLength = util::parseULLInt(contentLengthStr);
      if(contentLength == 0) {
        return SharedHandle<Range>(new Range());
      } else {
        return SharedHandle<Range>
          (new Range(0, contentLength-1, contentLength));
      }
    }
  }
  std::string byteRangeSpec;
  {
    // we expect that rangeStr looks like 'bytes 100-199/100'
    // but some server returns '100-199/100', omitting bytes-unit sepcifier
    // 'bytes'.
    std::pair<std::string, std::string> splist;
    util::divide(splist, rangeStr, ' ');
    if(splist.second.empty()) {
      // we assume bytes-unit specifier omitted.
      byteRangeSpec = splist.first;
    } else {
      byteRangeSpec = splist.second;
    }
  }
  std::pair<std::string, std::string> byteRangeSpecPair;
  util::divide(byteRangeSpecPair, byteRangeSpec, '/');

  if(util::strip(byteRangeSpecPair.first) == "*" ||
     util::strip(byteRangeSpecPair.second) == "*") {
    // If byte-range-resp-spec or instance-length is "*", we returns
    // empty Range. The former is usually sent with 416 (Request range
    // not satisfiable) status.
    return SharedHandle<Range>(new Range());
  }

  std::pair<std::string, std::string> byteRangeRespSpecPair;
  util::divide(byteRangeRespSpecPair, byteRangeSpecPair.first, '-');

  off_t startByte = util::parseLLInt(byteRangeRespSpecPair.first);
  off_t endByte = util::parseLLInt(byteRangeRespSpecPair.second);
  uint64_t entityLength = util::parseULLInt(byteRangeSpecPair.second);

  return SharedHandle<Range>(new Range(startByte, endByte, entityLength));
}

void HttpHeader::setResponseStatus(const std::string& responseStatus)
{
  responseStatus_ = responseStatus;
}

void HttpHeader::setVersion(const std::string& version)
{
  version_ = version;
}

void HttpHeader::setMethod(const std::string& method)
{
  method_ = method;
}

void HttpHeader::setRequestPath(const std::string& requestPath)
{
  requestPath_ = requestPath;
}

void HttpHeader::fill(std::istream& in)
{
  std::string line;
  while(std::getline(in, line)) {
    line = util::strip(line);
    if(line.empty()) {
      continue;
    }
    std::pair<std::string, std::string> hp;
    util::divide(hp, line, ':');
    put(hp.first, hp.second);
  }
}

void HttpHeader::clearField()
{
  table_.clear();
}

} // namespace aria2
