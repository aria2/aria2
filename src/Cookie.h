/* <!-- copyright */
/*
 * aria2 - The high speed download utility
 *
 * Copyright (C) 2010 Tatsuhiro Tsujikawa
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
#ifndef D_COOKIE_H
#define D_COOKIE_H

#include "common.h"

#include <string>

#include "a2time.h"

namespace aria2 {

class Cookie {
private:
  std::string name_;
  std::string value_;
  time_t expiryTime_;
  // If persistent_ is false, this is a session scope cookie and it is
  // never expired during session. So isExpired() always returns
  // false.
  bool persistent_;
  std::string domain_;
  bool hostOnly_;
  std::string path_;
  bool secure_;
  bool httpOnly_;
  time_t creationTime_;
  time_t lastAccessTime_;
public:
  Cookie();

  Cookie
  (const std::string& name,
   const std::string& value,
   time_t  expiryTime,
   bool persistent,
   const std::string& domain,
   bool hostOnly,
   const std::string& path,
   bool secure,
   bool httpOnly,
   time_t creationTime);

  Cookie(const Cookie& c);

  ~Cookie();

  Cookie& operator=(const Cookie& c);

  std::string toString() const;

  bool match
  (const std::string& requestHost, const std::string& requestPath,
   time_t date, bool secure) const;

  bool operator==(const Cookie& cookie) const;

  bool isExpired(time_t base) const;

  const std::string& getName() const
  {
    return name_;
  }

  void setName(const std::string& name);

  template<typename InputIterator>
  void setName(InputIterator first, InputIterator last)
  {
    name_.assign(first, last);
  }

  const std::string& getValue() const
  {
    return value_;
  }

  void setValue(const std::string& value);

  template<typename InputIterator>
  void setValue(InputIterator first, InputIterator last)
  {
    value_.assign(first, last);
  }

  time_t getExpiryTime() const
  {
    return expiryTime_;
  }

  void setExpiryTime(time_t expiryTime)
  {
    expiryTime_ = expiryTime;
  }

  bool getPersistent() const
  {
    return persistent_;
  }

  void setPersistent(bool persistent)
  {
    persistent_ = persistent;
  }

  const std::string& getDomain() const
  {
    return domain_;
  }

  void setDomain(const std::string& domain);

  template<typename InputIterator>
  void setDomain(InputIterator first, InputIterator last)
  {
    domain_.assign(first, last);
  }

  bool getHostOnly() const
  {
    return hostOnly_;
  }

  void setHostOnly(bool hostOnly)
  {
    hostOnly_ = hostOnly;
  }

  const std::string& getPath() const
  {
    return path_;
  }

  void setPath(const std::string& path);

  template<typename InputIterator>
  void setPath(InputIterator first, InputIterator last)
  {
    path_.assign(first, last);
  }

  bool getSecure() const
  {
    return secure_;
  }

  void setSecure(bool secure)
  {
    secure_ = secure;
  }

  bool getHttpOnly() const
  {
    return httpOnly_;
  }

  void setHttpOnly(bool httpOnly)
  {
    httpOnly_ = httpOnly;
  }

  time_t getCreationTime() const
  {
    return creationTime_;
  }

  void setCreationTime(time_t creationTime)
  {
    creationTime_ = creationTime;
  }

  time_t getLastAccessTime() const
  {
    return lastAccessTime_;
  }

  void setLastAccessTime(time_t lastAccessTime)
  {
    lastAccessTime_ = lastAccessTime;
  }

  std::string toNsCookieFormat() const;
};

} // namespace aria2

#endif // D_COOKIE_H
