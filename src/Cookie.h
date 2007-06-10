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
#ifndef _D_COOKIE_H_
#define _D_COOKIE_H_

#include "common.h"

class Cookie {
public:
  string name;
  string value;
  time_t expires;
  string path;
  string domain;
  bool secure;
  bool onetime; // if true, this cookie will expire when the user's session ends.
public:
  Cookie(const string& name,
	 const string& value,
	 time_t  expires,
	 const string& path,
	 const string& domain,
	 bool secure):
    name(name),
    value(value),
    expires(expires),
    path(path),
    domain(domain),
    secure(secure),
    onetime(false) {}

  Cookie(const string& name,
	 const string& value,
	 const string& path,
	 const string& domain,
	 bool secure):
    name(name),
    value(value),
    path(path),
    domain(domain),
    secure(secure),
    onetime(true) {}

  Cookie():expires(0), secure(false), onetime(true) {}

  ~Cookie() {}
  string toString() const {
    return name+"="+value;
  }
  void clear() {
    name = value = path = domain = "";
    expires = 0;
    secure = false;
  }

  bool good() const
  {
    return !name.empty();
  }

  bool match(const string& host, const string& dir, time_t date, bool secure) const;
};

typedef deque<Cookie> Cookies;

#endif // _D_COOKIE_H_
