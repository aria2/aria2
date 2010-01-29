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

#include <fstream>
#include <vector>

#include "util.h"
#include "A2STR.h"
#include "DlAbortEx.h"
#include "StringFormat.h"

namespace aria2 {

NsCookieParser::NsCookieParser() {}

NsCookieParser::~NsCookieParser() {}

static const std::string C_TRUE("TRUE");

static Cookie parseNsCookie(const std::string& nsCookieStr)
{
  std::vector<std::string> vs;
  util::split(nsCookieStr, std::back_inserter(vs), "\t", true);
  if(vs.size() < 6 ) {
    return Cookie();
  }

  int64_t expireDate = util::parseLLInt(vs[4]);
  // TODO assuming time_t is int32_t...
  if(expireDate > INT32_MAX) {
    expireDate = INT32_MAX;
  }

  Cookie c(vs[5], // name
           vs.size() >= 7? vs[6]:A2STR::NIL, // value
           expireDate, // expires
           vs[2], // path
           vs[0], // domain
           vs[3] == C_TRUE ? true : false);
  if(!util::startsWith(vs[0], A2STR::DOT_C)) {
    c.markOriginServerOnly();
  }
  return c;
}

std::deque<Cookie> NsCookieParser::parse(const std::string& filename)
{
  std::ifstream s(filename.c_str(), std::ios::binary);
  if(!s) {
    throw DL_ABORT_EX
      (StringFormat("Failed to open file %s", filename.c_str()).str());
  }
  std::string line;
  std::deque<Cookie> cookies;
  while(getline(s, line)) {
    if(util::startsWith(line, A2STR::SHARP_C)) {
      continue;
    }
    try {
      Cookie c = parseNsCookie(line);
      if(c.good()) {
        cookies.push_back(c);
      }
    } catch(RecoverableException& e) {
      // ignore malformed cookie entry
      // TODO better to log it
    }
  }
  return cookies;
}

} // namespace aria2
