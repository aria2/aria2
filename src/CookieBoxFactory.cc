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
#include "CookieBoxFactory.h"
#include "CookieParser.h"
#include "CookieBox.h"
#include "Util.h"
#include "RecoverableException.h"
#include <istream>

namespace aria2 {

CookieBoxHandle CookieBoxFactory::createNewInstance()
{
  CookieBoxHandle box(new CookieBox());
  box->add(defaultCookies);
  return box;
}

void CookieBoxFactory::loadDefaultCookie(std::istream& s)
{
  std::string line;
  while(getline(s, line)) {
    if(Util::startsWith(line, "#")) {
      continue;
    }
    try {
      Cookie c = parseNsCookie(line);
      if(c.good()) {
	defaultCookies.push_back(c);
      }
    } catch(RecoverableException* e) {
      // ignore malformed cookie entry
      // TODO better to log it
      delete e;
    }
  }
}

Cookie CookieBoxFactory::parseNsCookie(const std::string& nsCookieStr) const
{
  std::deque<std::string> vs;
  Util::slice(vs, nsCookieStr, '\t', true);
  Cookie c;
  if(vs.size() < 6 ) {
    return c;
  }
  c.domain = vs[0];
  c.path = vs[2];
  c.secure = vs[3] == "TRUE" ? true : false;
  int64_t expireDate = Util::parseLLInt(vs[4]);
  // TODO assuming time_t is int32_t...
  if(expireDate > INT32_MAX) {
    expireDate = INT32_MAX;
  }
  c.expires = expireDate;
  c.name = vs[5];
  if(vs.size() >= 7) {
    c.value = vs[6];
  }
  return c;
}

} // namespace aria2
