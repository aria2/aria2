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

#include <string>
#include <deque>

#include "a2time.h"

namespace aria2 {

class Cookie {
private:
  std::string _name;
  std::string _value;
  time_t _expiry;
  std::string _path;
  std::string _domain;
  bool _secure;
public:
  /*
   * If expires = 0 is given, then the cookie becomes session cookie.
   */
  Cookie(const std::string& name,
	 const std::string& value,
	 time_t  expires,
	 const std::string& path,
	 const std::string& domain,
	 bool secure);

  /*
   * Creates session cookie. This is equivalent to
   * Cookie(name, value, 0, path, domain, secure);
   */
  Cookie(const std::string& name,
	 const std::string& value,
	 const std::string& path,
	 const std::string& domain,
	 bool secure);

  Cookie();

  ~Cookie();

  std::string toString() const;

  bool good() const;

  bool match(const std::string& requestHost, const std::string& requestPath,
	     time_t date, bool secure) const;

  bool validate(const std::string& requestHost,
		const std::string& requestPath) const;

  bool operator==(const Cookie& cookie) const;

  bool isExpired() const;

  const std::string& getName() const
  {
    return _name;
  }

  const std::string& getValue() const
  {
    return _value;
  }

  const std::string& getPath() const
  {
    return _path;
  }

  const std::string& getDomain() const
  {
    return _domain;
  }

  time_t getExpiry() const
  {
    return _expiry;
  }

  bool isSecureCookie() const
  {
    return _secure;
  }

  bool isSessionCookie() const
  {
    return _expiry == 0;
  }

  std::string toNsCookieFormat() const;
};

typedef std::deque<Cookie> Cookies;

} // namespace aria2

#endif // _D_COOKIE_H_
