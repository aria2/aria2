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
#include "array_fun.h"

namespace aria2 {

NsCookieParser::NsCookieParser() = default;

NsCookieParser::~NsCookieParser() = default;

namespace {
std::unique_ptr<Cookie> parseNsCookie(const std::string& cookieStr,
                                      time_t creationTime)
{
  std::vector<Scip> vs;
  util::splitIter(cookieStr.begin(), cookieStr.end(), std::back_inserter(vs),
                  '\t', true);
  if (vs.size() < 6) {
    return nullptr;
  }
  vs[0].first = util::lstripIter(vs[0].first, vs[0].second, '.');
  if (vs[5].first == vs[5].second || vs[0].first == vs[0].second ||
      !cookie::goodPath(vs[2].first, vs[2].second)) {
    return nullptr;
  }
  int64_t expiryTime;
  {
    // chrome extension uses subsecond resolution for expiry time.
    double expiryTimeDouble;
    if (!util::parseDoubleNoThrow(expiryTimeDouble,
                                  std::string(vs[4].first, vs[4].second))) {
      return nullptr;
    }
    expiryTime = static_cast<int64_t>(expiryTimeDouble);
  }
  if (std::numeric_limits<time_t>::max() < expiryTime) {
    expiryTime = std::numeric_limits<time_t>::max();
  }
  else if (std::numeric_limits<time_t>::min() > expiryTime) {
    expiryTime = std::numeric_limits<time_t>::min();
  }
  auto cookie = make_unique<Cookie>();
  cookie->setName(vs[5].first, vs[5].second);
  if (vs.size() >= 7) {
    cookie->setValue(vs[6].first, vs[6].second);
  }
  else {
    cookie->setValue("");
  }
  cookie->setExpiryTime(expiryTime == 0 ? std::numeric_limits<time_t>::max()
                                        : expiryTime);
  // aria2 treats expiryTime == 0 means session cookie->
  cookie->setPersistent(expiryTime != 0);
  cookie->setDomain(vs[0].first, vs[0].second);
  cookie->setHostOnly(util::isNumericHost(cookie->getDomain()) ||
                      !util::streq(vs[1].first, vs[1].second, "TRUE"));
  cookie->setPath(vs[2].first, vs[2].second);
  cookie->setSecure(util::streq(vs[3].first, vs[3].second, "TRUE"));
  cookie->setCreationTime(creationTime);
  return cookie;
}
} // namespace

std::vector<std::unique_ptr<Cookie>>
NsCookieParser::parse(const std::string& filename, time_t creationTime)
{
  BufferedFile fp{filename.c_str(), BufferedFile::READ};
  if (!fp) {
    throw DL_ABORT_EX(fmt("Failed to open file %s", filename.c_str()));
  }
  std::vector<std::unique_ptr<Cookie>> cookies;
  while (1) {
    std::string line = fp.getLine();
    if (line.empty()) {
      if (fp.eof()) {
        break;
      }
      else if (!fp) {
        throw DL_ABORT_EX("CookieParser:I/O error.");
      }
      else {
        continue;
      }
    }
    if (line[0] == '#') {
      continue;
    }
    auto c = parseNsCookie(line, creationTime);
    if (c) {
      cookies.push_back(std::move(c));
    }
  }
  return cookies;
}

} // namespace aria2
