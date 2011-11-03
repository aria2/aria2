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
#include "Range.h"
#include "util.h"
#include "A2STR.h"

namespace aria2 {

const std::string HttpHeader::LOCATION("Location");

const std::string HttpHeader::TRANSFER_ENCODING("Transfer-Encoding");

const std::string HttpHeader::CONTENT_ENCODING("Content-Encoding");

const std::string HttpHeader::CONTENT_DISPOSITION("Content-Disposition");

const std::string HttpHeader::SET_COOKIE("Set-Cookie");

const std::string HttpHeader::CONTENT_TYPE("Content-Type");

const std::string HttpHeader::RETRY_AFTER("Retry-After");

const std::string HttpHeader::CONNECTION("Connection");

const std::string HttpHeader::CONTENT_LENGTH("Content-Length");

const std::string HttpHeader::CONTENT_RANGE("Content-Range");

const std::string HttpHeader::LAST_MODIFIED("Last-Modified");

const std::string HttpHeader::ACCEPT_ENCODING("Accept-Encoding");

const std::string HttpHeader::LINK("Link");

const std::string HttpHeader::DIGEST("Digest");

const char HttpHeader::HTTP_1_1[] = "HTTP/1.1";
const char HttpHeader::CLOSE[] = "close";
const char HttpHeader::CHUNKED[] = "chunked";
const char HttpHeader::GZIP[] = "gzip";
const char HttpHeader::DEFLATE[] = "deflate";

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

std::pair<std::multimap<std::string, std::string>::const_iterator,
          std::multimap<std::string, std::string>::const_iterator>
HttpHeader::getIterator(const std::string& name) const
{
  std::string n(util::toLower(name));
  return table_.equal_range(n);
}

unsigned int HttpHeader::getFirstAsUInt(const std::string& name) const {
  return getFirstAsULLInt(name);
}

uint64_t HttpHeader::getFirstAsULLInt(const std::string& name) const {
  const std::string& value = getFirst(name);
  if(value.empty()) {
    return 0;
  } else {
    return util::parseULLInt(value.begin(), value.end());
  }
}

RangeHandle HttpHeader::getRange() const
{
  const std::string& rangeStr = getFirst(CONTENT_RANGE);
  if(rangeStr.empty()) {
    const std::string& clenStr = getFirst(CONTENT_LENGTH);
    if(clenStr.empty()) {
      return SharedHandle<Range>(new Range());
    } else {
      uint64_t contentLength =
        util::parseULLInt(clenStr.begin(), clenStr.end());
      if(contentLength == 0) {
        return SharedHandle<Range>(new Range());
      } else {
        return SharedHandle<Range>
          (new Range(0, contentLength-1, contentLength));
      }
    }
  }
  // we expect that rangeStr looks like 'bytes 100-199/100'
  // but some server returns '100-199/100', omitting bytes-unit sepcifier
  // 'bytes'.
  std::string::const_iterator byteRangeSpec =
    std::find(rangeStr.begin(), rangeStr.end(), ' ');
  if(byteRangeSpec == rangeStr.end()) {
    // we assume bytes-unit specifier omitted.
    byteRangeSpec = rangeStr.begin();
  } else {
    while(byteRangeSpec != rangeStr.end() &&
          (*byteRangeSpec == ' ' || *byteRangeSpec == '\t')) {
      ++byteRangeSpec;
    }
  }
  std::string::const_iterator slash =
    std::find(byteRangeSpec, rangeStr.end(), '/');
  if(slash == rangeStr.end() || slash+1 == rangeStr.end() ||
     (byteRangeSpec+1 == slash && *byteRangeSpec == '*') ||
     (slash+2 == rangeStr.end() && *(slash+1) == '*')) {
    // If byte-range-resp-spec or instance-length is "*", we returns
    // empty Range. The former is usually sent with 416 (Request range
    // not satisfiable) status.
    return SharedHandle<Range>(new Range());
  }
  std::string::const_iterator minus = std::find(byteRangeSpec, slash, '-');
  if(minus == slash) {
    return SharedHandle<Range>(new Range());
  }
  off_t startByte = util::parseLLInt(byteRangeSpec, minus);
  off_t endByte = util::parseLLInt(minus+1, slash);
  uint64_t entityLength = util::parseULLInt(slash+1, rangeStr.end());
  return SharedHandle<Range>(new Range(startByte, endByte, entityLength));
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

void HttpHeader::fill
(std::string::const_iterator first,
 std::string::const_iterator last)
{
  std::string name;
  std::string value;
  while(first != last) {
    std::string::const_iterator j = first;
    while(j != last && *j != '\r' && *j != '\n') {
      ++j;
    }
    if(first != j) {
      std::string::const_iterator sep = std::find(first, j, ':');
      if(sep == j) {
        // multiline header?
        if(*first == ' ' || *first == '\t') {
          std::pair<std::string::const_iterator,
                    std::string::const_iterator> p = util::stripIter(first, j);
          if(!name.empty() && p.first != p.second) {
            if(!value.empty()) {
              value += ' ';
            }
            value.append(p.first, p.second);
          }
        }
      } else {
        if(!name.empty()) {
          put(name, value);
        }
        std::pair<std::string::const_iterator,
                  std::string::const_iterator> p = util::stripIter(first, sep);
        name.assign(p.first, p.second);
        p = util::stripIter(sep+1, j);
        value.assign(p.first, p.second);
      }
    }
    while(j != last && (*j == '\r' || *j == '\n')) {
      ++j;
    }
    first = j;
  }
  if(!name.empty()) {
    put(name, value);
  }
}

void HttpHeader::clearField()
{
  table_.clear();
}

} // namespace aria2
