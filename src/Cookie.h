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
#ifndef _D_COOKIE_H_
#define _D_COOKIE_H_

#include "common.h"

#include <string>

#include "a2time.h"

namespace aria2 {

class Cookie {
private:
  std::string name_;
  std::string value_;
  time_t expiry_;
  std::string path_;
  std::string domain_;
  bool secure_;
  time_t creationTime_;
  time_t lastAccess_;
public:
  /*
   * If expires = 0 is given, then the cookie becomes session cookie.
   * domain is normalized using normalizeDomain() function and
   * assigned to domain_.  If domain is not specified in cookie, call
   * markOriginServerOnly() after construction.
   */
  Cookie(const std::string& name,
         const std::string& value,
         time_t  expires,
         const std::string& path,
         const std::string& domain,
         bool secure);

  /*
   * Creates session cookie. This is equivalent to Cookie(name, value,
   * 0, path, domain, secure); domain is normalized using
   * normalizeDomain() function and assigned to domain_.  If domain is
   * not specified in cookie, call markOriginServerOnly() after
   * construction.
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
    return name_;
  }

  const std::string& getValue() const
  {
    return value_;
  }

  const std::string& getPath() const
  {
    return path_;
  }

  const std::string& getDomain() const
  {
    return domain_;
  }

  time_t getExpiry() const
  {
    return expiry_;
  }

  bool isSecureCookie() const
  {
    return secure_;
  }

  bool isSessionCookie() const
  {
    return expiry_ == 0;
  }

  std::string toNsCookieFormat() const;

  // Makes this Cookie only sent to the origin server.  This function
  // removes first "." from domain_ if domain_ starts with ".".
  void markOriginServerOnly();

  time_t getCreationTime() const
  {
    return creationTime_;
  }

  void updateLastAccess();

  time_t getLastAccess() const
  {
    return lastAccess_;
  }

  static std::string normalizeDomain(const std::string& domain);
};

} // namespace aria2

#endif // _D_COOKIE_H_
