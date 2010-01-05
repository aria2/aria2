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
#include "CookieParser.h"

#include <strings.h>

#include <utility>
#include <istream>
#include <map>
#include <vector>

#include "util.h"
#include "A2STR.h"
#include "TimeA2.h"

namespace aria2 {

const std::string CookieParser::C_SECURE("secure");

const std::string CookieParser::C_DOMAIN("domain");

const std::string CookieParser::C_PATH("path");

const std::string CookieParser::C_EXPIRES("expires");

Cookie CookieParser::parse(const std::string& cookieStr) const
{
  return parse(cookieStr, A2STR::NIL, A2STR::NIL);
}

Cookie CookieParser::parse(const std::string& cookieStr, const std::string& defaultDomain, const std::string& defaultPath) const
{
  std::vector<std::string> terms;
  util::split(cookieStr, std::back_inserter(terms), ";", true);
  if(terms.empty()) {
    return Cookie();
  }
  std::pair<std::string, std::string> nameValue;
  util::split(nameValue, terms.front(), '=');

  std::map<std::string, std::string> values;
  values[C_DOMAIN] = defaultDomain;
  values[C_PATH] = defaultPath;
  
  for(std::vector<std::string>::iterator itr = terms.begin()+1;
      itr != terms.end(); ++itr) {
    std::pair<std::string, std::string> nv;
    util::split(nv, *itr, '=');
    values[nv.first] = nv.second;
  }
  time_t expiry = 0;
  if(values.find(C_EXPIRES) != values.end()) {
    Time expiryTime = Time::parseHTTPDate(values[C_EXPIRES]);
    if(expiryTime.good()) {
      expiry = expiryTime.getTime();
    }
  }
  return Cookie(nameValue.first, nameValue.second,
                expiry,
                values[C_PATH], values[C_DOMAIN],
                values.find(C_SECURE) != values.end());
}


Cookies CookieParser::parse(std::istream& s) const
{
  Cookies cookies;
  std::string line;
  while(getline(s, line)) {
    if(util::trim(line).empty() || util::startsWith(line, A2STR::SHARP_C)) {
      continue;
    }
    Cookie cookie = parse(line);
    if(cookie.good()) {
      cookies.push_back(cookie);
    }
  }
  return cookies;
}

} // namespace aria2
