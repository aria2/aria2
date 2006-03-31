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
#include "CookieBox.h"
#include "Util.h"

CookieBox::CookieBox() {}

CookieBox::~CookieBox() {}

void CookieBox::add(const Cookie& cookie) {
  cookies.push_back(cookie);
}

void CookieBox::add(const string& cookieStr) {
  Cookie c;
  parse(c, cookieStr);
  cookies.push_back(c);
}

void CookieBox::setField(Cookie& cookie, const string& name, const string& value) const {
  if(name.size() == string("secure").size() &&
     strcasecmp(name.c_str(), "secure") == 0) {
    cookie.secure = true;
  } else if(name.size() == string("domain").size() && strcasecmp(name.c_str(), "domain") == 0) {
    cookie.domain = value;
  } else if(name.size() == string("path").size() && strcasecmp(name.c_str(), "path") == 0) {
    cookie.path = value;
  } else if(name.size() == string("expires").size() && strcasecmp(name.c_str(), "expires") == 0) {
    cookie.expires = value;
  } else {
    cookie.name = name;
    cookie.value = value;
  }
}

void CookieBox::parse(Cookie& cookie, const string& cookieStr) const {
  cookie.clear();
  Strings terms;
  Util::slice(terms, cookieStr, ';');
  for(Strings::iterator itr = terms.begin(); itr != terms.end(); itr++) {
    pair<string, string> nv;
    Util::split(nv, *itr, '=');
    setField(cookie, nv.first, nv.second);
  }
}

Cookies CookieBox::criteriaFind(const string& host, const string& dir, bool secure) const {
  Cookies result;
  for(Cookies::const_iterator itr = cookies.begin(); itr != cookies.end(); itr++) {
    const Cookie& c = *itr;
    if((secure || !c.secure && !secure) &&
       Util::endsWith(host, c.domain) &&
       Util::startsWith(dir, c.path)) {
      // TODO we currently ignore expire date.
      result.push_back(c);
    }
  }
  return result;
}

