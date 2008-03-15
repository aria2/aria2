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
#include "HttpHeader.h"
#include "Range.h"
#include "Util.h"
#include <cstdlib>

namespace aria2 {

void HttpHeader::put(const std::string& name, const std::string& value) {
  std::multimap<std::string, std::string>::value_type vt(Util::toLower(name), value);
  table.insert(vt);
}

bool HttpHeader::defined(const std::string& name) const {
  return table.count(Util::toLower(name)) >= 1;
}

std::string HttpHeader::getFirst(const std::string& name) const {
  std::multimap<std::string, std::string>::const_iterator itr = table.find(Util::toLower(name));
  if(itr == table.end()) {
    return "";
  } else {
    return (*itr).second;
  }
}

std::deque<std::string> HttpHeader::get(const std::string& name) const {
  std::deque<std::string> v;
  for(std::multimap<std::string, std::string>::const_iterator itr = table.find(Util::toLower(name)); itr != table.end(); itr++) {
    v.push_back((*itr).second);
  }
  return v;
}

int32_t HttpHeader::getFirstAsInt(const std::string& name) const {
  return getFirstAsLLInt(name);
}

int64_t HttpHeader::getFirstAsLLInt(const std::string& name) const {
  std::string value = getFirst(name);
  if(value == "") {
    return 0;
  } else {
    return strtoll(value.c_str(), 0, 10);
  }
}

RangeHandle HttpHeader::getRange() const
{
  std::string rangeStr = getFirst("Content-Range");
  if(rangeStr == "") {
    std::string contentLengthStr = getFirst("Content-Length");
    if(contentLengthStr == "") {
      return new Range(0, 0, 0);
    } else {
      int64_t contentLength = strtoll(contentLengthStr.c_str(), 0, 10);
      return new Range(0, contentLength-1, contentLength);
    }
  }
  
  std::string byteRangeSpec;
  {
    // we expect that rangeStr looks like 'bytes 100-199/100'
    // but some server returns '100-199/100', omitting bytes-unit sepcifier
    // 'bytes'.
    std::pair<std::string, std::string> splist;
    Util::split(splist, rangeStr, ' ');
    if(splist.second.empty()) {
      // we assume bytes-unit specifier omitted.
      byteRangeSpec = splist.first;
    } else {
      byteRangeSpec = splist.second;
    }
  }
  std::pair<std::string, std::string> byteRangeSpecPair;
  Util::split(byteRangeSpecPair, byteRangeSpec, '/');

  std::pair<std::string, std::string> byteRangeRespSpecPair;
  Util::split(byteRangeRespSpecPair, byteRangeSpecPair.first, '-');

  int64_t startByte = STRTOLL(byteRangeRespSpecPair.first.c_str());
  int64_t endByte = STRTOLL(byteRangeRespSpecPair.second.c_str());
  int64_t entityLength = STRTOLL(byteRangeSpecPair.second.c_str());

  return new Range(startByte, endByte, entityLength);
}

} // namespace aria2
