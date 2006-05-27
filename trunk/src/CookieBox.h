/* <!-- copyright */
/*
 * aria2 - a simple utility for downloading files faster
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
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */
/* copyright --> */
#ifndef _D_COOKIE_BOX_H_
#define _D_COOKIE_BOX_H_

#include "common.h"
#include <string>
#include <deque>

using namespace std;

class Cookie {
public:
  string name;
  string value;
  string expires;
  string path;
  string domain;
  bool secure;
public:
  Cookie(const string& name, const string& value, const string& expires, const string& path, const string& domain, bool secure):name(name), value(value), expires(expires), path(path), domain(domain), secure(secure) {}
  Cookie():secure(false) {}
  ~Cookie() {}
  string toString() const {
    return name+"="+value;
  }
  void clear() {
    name = value = expires = path = domain = "";
    secure = false;
  }
};

typedef deque<Cookie> Cookies;

class CookieBox {
private:
  Cookies cookies;
  void setField(Cookie& cookie, const string& name, const string& value) const;
public:
  CookieBox();
  ~CookieBox();
  void clear();
  void add(const Cookie& cookie);
  void add(const string& cookieStr);
  void parse(Cookie& cookie, const string& cookieStr) const;
  Cookies criteriaFind(const string& host, const string& dir, bool secure) const;
};

#endif // _D_COOKIE_BOX_H_
