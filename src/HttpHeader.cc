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
#include "array_fun.h"

namespace aria2 {

HttpHeader::HttpHeader() : statusCode_(0) {}

HttpHeader::~HttpHeader() = default;

void HttpHeader::put(int hdKey, const std::string& value)
{
  std::multimap<int, std::string>::value_type vt(hdKey, value);
  table_.insert(vt);
}

void HttpHeader::remove(int hdKey) { table_.erase(hdKey); }

bool HttpHeader::defined(int hdKey) const { return table_.count(hdKey); }

const std::string& HttpHeader::find(int hdKey) const
{
  auto itr = table_.find(hdKey);
  if (itr == table_.end()) {
    return A2STR::NIL;
  }
  else {
    return (*itr).second;
  }
}

std::vector<std::string> HttpHeader::findAll(int hdKey) const
{
  std::vector<std::string> v;
  auto itrpair = table_.equal_range(hdKey);
  while (itrpair.first != itrpair.second) {
    v.push_back((*itrpair.first).second);
    ++itrpair.first;
  }
  return v;
}

std::pair<std::multimap<int, std::string>::const_iterator,
          std::multimap<int, std::string>::const_iterator>
HttpHeader::equalRange(int hdKey) const
{
  return table_.equal_range(hdKey);
}

Range HttpHeader::getRange() const
{
  const auto& rangeStr = find(CONTENT_RANGE);
  if (rangeStr.empty()) {
    const std::string& clenStr = find(CONTENT_LENGTH);
    if (clenStr.empty()) {
      return Range();
    }
    else {
      int64_t contentLength;
      if (!util::parseLLIntNoThrow(contentLength, clenStr) ||
          contentLength < 0) {
        throw DL_ABORT_EX("Content-Length must be positive integer");
      }
      else if (contentLength > std::numeric_limits<a2_off_t>::max()) {
        throw DOWNLOAD_FAILURE_EXCEPTION(fmt(EX_TOO_LARGE_FILE, contentLength));
      }
      else if (contentLength == 0) {
        return Range();
      }
      else {
        return Range(0, contentLength - 1, contentLength);
      }
    }
  }
  // we expect that rangeStr looks like 'bytes 100-199/200' but some
  // server returns '100-199/200', omitting bytes-unit specifier
  // 'bytes'.  Moreover, some server may return like
  // 'bytes=100-199/200'.
  auto byteRangeSpec = std::find(rangeStr.begin(), rangeStr.end(), ' ');
  if (byteRangeSpec == rangeStr.end()) {
    // check for 'bytes=100-199/200' case
    byteRangeSpec = std::find(rangeStr.begin(), rangeStr.end(), '=');
    if (byteRangeSpec == rangeStr.end()) {
      // we assume bytes-unit specifier omitted.
      byteRangeSpec = rangeStr.begin();
    }
    else {
      ++byteRangeSpec;
    }
  }
  else {
    while (byteRangeSpec != rangeStr.end() &&
           (*byteRangeSpec == ' ' || *byteRangeSpec == '\t')) {
      ++byteRangeSpec;
    }
  }
  auto slash = std::find(byteRangeSpec, rangeStr.end(), '/');
  if (slash == rangeStr.end() || slash + 1 == rangeStr.end() ||
      (byteRangeSpec + 1 == slash && *byteRangeSpec == '*') ||
      (slash + 2 == rangeStr.end() && *(slash + 1) == '*')) {
    // If byte-range-resp-spec or instance-length is "*", we returns
    // empty Range. The former is usually sent with 416 (Request range
    // not satisfiable) status.
    return Range();
  }
  auto minus = std::find(byteRangeSpec, slash, '-');
  if (minus == slash) {
    return Range();
  }
  int64_t startByte, endByte, entityLength;
  if (!util::parseLLIntNoThrow(startByte, std::string(byteRangeSpec, minus)) ||
      !util::parseLLIntNoThrow(endByte, std::string(minus + 1, slash)) ||
      !util::parseLLIntNoThrow(entityLength,
                               std::string(slash + 1, rangeStr.end())) ||
      startByte < 0 || endByte < 0 || entityLength < 0) {
    throw DL_ABORT_EX("byte-range-spec must be positive");
  }
  if (startByte > std::numeric_limits<a2_off_t>::max()) {
    throw DOWNLOAD_FAILURE_EXCEPTION(fmt(EX_TOO_LARGE_FILE, startByte));
  }
  if (endByte > std::numeric_limits<a2_off_t>::max()) {
    throw DOWNLOAD_FAILURE_EXCEPTION(fmt(EX_TOO_LARGE_FILE, endByte));
  }
  if (entityLength > std::numeric_limits<a2_off_t>::max()) {
    throw DOWNLOAD_FAILURE_EXCEPTION(fmt(EX_TOO_LARGE_FILE, entityLength));
  }
  return Range(startByte, endByte, entityLength);
}

void HttpHeader::setVersion(const std::string& version) { version_ = version; }

void HttpHeader::setMethod(const std::string& method) { method_ = method; }

void HttpHeader::setRequestPath(const std::string& requestPath)
{
  requestPath_ = requestPath;
}

void HttpHeader::clearField() { table_.clear(); }

int HttpHeader::getStatusCode() const { return statusCode_; }

void HttpHeader::setStatusCode(int code) { statusCode_ = code; }

const std::string& HttpHeader::getVersion() const { return version_; }

const std::string& HttpHeader::getMethod() const { return method_; }

const std::string& HttpHeader::getRequestPath() const { return requestPath_; }

const std::string& HttpHeader::getReasonPhrase() const { return reasonPhrase_; }

void HttpHeader::setReasonPhrase(const std::string& reasonPhrase)
{
  reasonPhrase_ = reasonPhrase;
}

bool HttpHeader::fieldContains(int hdKey, const char* value)
{
  std::pair<std::multimap<int, std::string>::const_iterator,
            std::multimap<int, std::string>::const_iterator>
      range = equalRange(hdKey);
  for (auto i = range.first; i != range.second; ++i) {
    std::vector<Scip> values;
    util::splitIter((*i).second.begin(), (*i).second.end(),
                    std::back_inserter(values), ',',
                    true // doStrip
    );
    for (const auto& v : values) {
      if (util::strieq(v.first, v.second, value)) {
        return true;
      }
    }
  }
  return false;
}

bool HttpHeader::isKeepAlive() const
{
  const std::string& connection = find(CONNECTION);
  return !util::strieq(connection, "close") &&
         (version_ == "HTTP/1.1" || util::strieq(connection, "keep-alive"));
}

namespace {
constexpr const char* INTERESTING_HEADER_NAMES[] = {
    "accept-encoding",
    "access-control-request-headers",
    "access-control-request-method",
    "authorization",
    "connection",
    "content-disposition",
    "content-encoding",
    "content-length",
    "content-range",
    "content-type",
    "digest",
    "infohash",
    "last-modified",
    "link",
    "location",
    "origin",
    "port",
    "retry-after",
    "sec-websocket-key",
    "sec-websocket-version",
    "set-cookie",
    "transfer-encoding",
    "upgrade",
};
} // namespace

int idInterestingHeader(const char* hdName)
{
  auto i = std::lower_bound(std::begin(INTERESTING_HEADER_NAMES),
                            std::end(INTERESTING_HEADER_NAMES), hdName,
                            util::strless);
  if (i != std::end(INTERESTING_HEADER_NAMES) && strcmp(*i, hdName) == 0) {
    return i - std::begin(INTERESTING_HEADER_NAMES);
  }
  else {
    return HttpHeader::MAX_INTERESTING_HEADER;
  }
}

} // namespace aria2
