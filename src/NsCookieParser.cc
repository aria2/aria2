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
#include "NsCookieParser.h"

#include <cstdio>
#include <cstring>
#include <limits>

#include "util.h"
#include "A2STR.h"
#include "DlAbortEx.h"
#include "fmt.h"
#include "Cookie.h"
#include "cookie_helper.h"
#include "BufferedFile.h"

namespace aria2 {

NsCookieParser::NsCookieParser() {}

NsCookieParser::~NsCookieParser() {}

namespace {
const std::string C_TRUE("TRUE");
} // namespace

namespace {
bool parseNsCookie
(Cookie& cookie, const std::string& nsCookieStr, time_t creationTime)
{
  std::vector<std::string> vs;
  util::split(nsCookieStr, std::back_inserter(vs), "\t", true);
  if(vs.size() < 6) {
    return false;
  }
  std::string cookieDomain = cookie::removePrecedingDots(vs[0]);
  if(vs[5].empty() || cookieDomain.empty() || !cookie::goodPath(vs[2])) {
    return false;
  }
  int64_t expiryTime;
  if(!util::parseLLIntNoThrow(expiryTime, vs[4])) {
    return false;
  }
  if(std::numeric_limits<time_t>::max() < expiryTime) {
    expiryTime = std::numeric_limits<time_t>::max();
  } else if(std::numeric_limits<time_t>::min() > expiryTime) {
    expiryTime = std::numeric_limits<time_t>::min();
  }
  cookie.setName(vs[5]);
  cookie.setValue(vs.size() >= 7? vs[6]:A2STR::NIL);
  cookie.setExpiryTime(expiryTime == 0?
                       std::numeric_limits<time_t>::max():expiryTime);
  // aria2 treats expiryTime == 0 means session cookie.
  cookie.setPersistent(expiryTime != 0);
  cookie.setDomain(cookieDomain);
  cookie.setHostOnly(util::isNumericHost(cookieDomain) || vs[1] != C_TRUE);
  cookie.setPath(vs[2]);
  cookie.setSecure(vs[3] == C_TRUE);
  cookie.setCreationTime(creationTime);
  return true;
}
} // namespace

std::vector<Cookie> NsCookieParser::parse
(const std::string& filename, time_t creationTime)
{
  BufferedFile fp(filename, BufferedFile::READ);
  if(!fp) {
    throw DL_ABORT_EX(fmt("Failed to open file %s",
                          utf8ToNative(filename).c_str()));
  }
  std::vector<Cookie> cookies;
  char buf[8192];
  while(1) {
    if(!fp.getsn(buf, sizeof(buf))) {
      break;
    }
    std::string line(buf);
    if(util::startsWith(line, A2STR::SHARP_C)) {
      continue;
    }
    Cookie c;
    if(parseNsCookie(c, line, creationTime)) {
      cookies.push_back(c);
    }
  }
  return cookies;
}

} // namespace aria2
