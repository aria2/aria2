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
#include "DownloadFailureException.h"

namespace aria2 {

const std::string HttpHeader::LOCATION("location");
const std::string HttpHeader::TRANSFER_ENCODING("transfer-encoding");
const std::string HttpHeader::CONTENT_ENCODING("content-encoding");
const std::string HttpHeader::CONTENT_DISPOSITION("content-disposition");
const std::string HttpHeader::SET_COOKIE("set-cookie");
const std::string HttpHeader::CONTENT_TYPE("content-type");
const std::string HttpHeader::RETRY_AFTER("retry-after");
const std::string HttpHeader::CONNECTION("connection");
const std::string HttpHeader::CONTENT_LENGTH("content-length");
const std::string HttpHeader::CONTENT_RANGE("content-range");
const std::string HttpHeader::LAST_MODIFIED("last-modified");
const std::string HttpHeader::ACCEPT_ENCODING("accept-encoding");
const std::string HttpHeader::LINK("link");
const std::string HttpHeader::DIGEST("digest");
const std::string HttpHeader::PROXY_CONNECTION("proxy-connection");
const std::string HttpHeader::AUTHORIZATION("authorization");

const std::string HttpHeader::HTTP_1_1 = "HTTP/1.1";
const std::string HttpHeader::CLOSE = "close";
const std::string HttpHeader::KEEP_ALIVE = "keep-alive";
const std::string HttpHeader::CHUNKED = "chunked";
const std::string HttpHeader::GZIP = "gzip";
const std::string HttpHeader::DEFLATE = "deflate";

HttpHeader::HttpHeader() {}
HttpHeader::~HttpHeader() {}

void HttpHeader::put(const std::string& name, const std::string& value)
{
  std::multimap<std::string, std::string>::value_type vt(name, value);
  table_.insert(vt);
}

bool HttpHeader::defined(const std::string& name) const
{
  return table_.count(name);
}

const std::string& HttpHeader::find(const std::string& name) const
{
  std::multimap<std::string, std::string>::const_iterator itr =
    table_.find(name);
  if(itr == table_.end()) {
    return A2STR::NIL;
  } else {
    return (*itr).second;
  }
}

std::vector<std::string> HttpHeader::findAll(const std::string& name) const
{
  std::vector<std::string> v;
  std::pair<std::multimap<std::string, std::string>::const_iterator,
            std::multimap<std::string, std::string>::const_iterator> itrpair =
    table_.equal_range(name);
  while(itrpair.first != itrpair.second) {
    v.push_back((*itrpair.first).second);
    ++itrpair.first;
  }
  return v;
}

std::pair<std::multimap<std::string, std::string>::const_iterator,
          std::multimap<std::string, std::string>::const_iterator>
HttpHeader::equalRange(const std::string& name) const
{
  return table_.equal_range(name);
}

int32_t HttpHeader::findAsInt(const std::string& name) const
{
  const std::string& value = find(name);
  if(value.empty()) {
    return 0;
  } else {
    return util::parseInt(value);
  }
}

int64_t HttpHeader::findAsLLInt(const std::string& name) const
{
  const std::string& value = find(name);
  if(value.empty()) {
    return 0;
  } else {
    return util::parseLLInt(value);
  }
}

RangeHandle HttpHeader::getRange() const
{
  const std::string& rangeStr = find(CONTENT_RANGE);
  if(rangeStr.empty()) {
    const std::string& clenStr = find(CONTENT_LENGTH);
    if(clenStr.empty()) {
      return SharedHandle<Range>(new Range());
    } else {
      int64_t contentLength = util::parseLLInt(clenStr);
      if(contentLength < 0) {
        throw DL_ABORT_EX("Content-Length must be positive");
      } else if(contentLength > std::numeric_limits<off_t>::max()) {
        throw DOWNLOAD_FAILURE_EXCEPTION
          (fmt(EX_TOO_LARGE_FILE, contentLength));
      } else if(contentLength == 0) {
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
  int64_t startByte = util::parseLLInt(std::string(byteRangeSpec, minus));
  int64_t endByte = util::parseLLInt(std::string(minus+1, slash));
  int64_t entityLength =
    util::parseLLInt(std::string(slash+1, rangeStr.end()));
  if(startByte < 0 || endByte < 0 || entityLength < 0) {
    throw DL_ABORT_EX("byte-range-spec must be positive");
  }
  if(startByte > std::numeric_limits<off_t>::max()) {
    throw DOWNLOAD_FAILURE_EXCEPTION(fmt(EX_TOO_LARGE_FILE, startByte));
  }
  if(endByte > std::numeric_limits<off_t>::max()) {
    throw DOWNLOAD_FAILURE_EXCEPTION(fmt(EX_TOO_LARGE_FILE, endByte));
  }
  if(entityLength > std::numeric_limits<off_t>::max()) {
    throw DOWNLOAD_FAILURE_EXCEPTION(fmt(EX_TOO_LARGE_FILE, entityLength));
  }
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

void HttpHeader::clearField()
{
  table_.clear();
}

int HttpHeader::getStatusCode() const
{
  return statusCode_;
}

void HttpHeader::setStatusCode(int code)
{
  statusCode_ = code;
}

const std::string& HttpHeader::getVersion() const
{
  return version_;
}

const std::string& HttpHeader::getMethod() const
{
  return method_;
}

const std::string& HttpHeader::getRequestPath() const
{
  return requestPath_;
}

const std::string& HttpHeader::getReasonPhrase() const
{
  return reasonPhrase_;
}

void HttpHeader::setReasonPhrase(const std::string& reasonPhrase)
{
  reasonPhrase_ = reasonPhrase;
}

bool HttpHeader::fieldContains(const std::string& name,
                               const std::string& value)
{
  std::pair<std::multimap<std::string, std::string>::const_iterator,
            std::multimap<std::string, std::string>::const_iterator> range =
    equalRange(name);
  for(std::multimap<std::string, std::string>::const_iterator i = range.first;
      i != range.second; ++i) {
    std::vector<Scip> values;
    util::splitIter((*i).second.begin(), (*i).second.end(),
                    std::back_inserter(values),
                    ',',
                    true // doStrip
                    );
    for(std::vector<Scip>::const_iterator j = values.begin(),
          eoj = values.end(); j != eoj; ++j) {
      if(util::strieq((*j).first, (*j).second, value.begin(), value.end())) {
        return true;
      }
    }
  }
  return false;
}

} // namespace aria2
