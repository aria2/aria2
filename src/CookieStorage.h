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
#ifndef D_COOKIE_STORAGE_H
#define D_COOKIE_STORAGE_H

#include "common.h"

#include <string>
#include <deque>
#include <vector>
#include <set>
#include <algorithm>

#include "a2time.h"
#include "Cookie.h"
#include "a2functional.h"

namespace aria2 {

class BufferedFile;

class CookieStorage {
public:

  static const size_t MAX_COOKIE_PER_DOMAIN = 50;

  class DomainEntry {
  private:
    // This is reversed domain level string.
    // e.g. net.sourceforge.aria2
    // e.g. 192.168.0.1
    std::string key_;

    time_t lastAccessTime_;

    std::deque<Cookie> cookies_;
  public:
    DomainEntry(const std::string& domain);
    DomainEntry(const DomainEntry& c);
    ~DomainEntry();

    void swap(DomainEntry& c);

    DomainEntry& operator=(const DomainEntry& c);

    const std::string& getKey() const
    {
      return key_;
    }

    void findCookie
    (std::vector<Cookie>& out,
     const std::string& requestHost,
     const std::string& requestPath,
     time_t now, bool secure);

    size_t countCookie() const;

    bool addCookie(const Cookie& cookie, time_t now);

    void setLastAccessTime(time_t lastAccessTime)
    {
      lastAccessTime_ = lastAccessTime;
    }

    time_t getLastAccessTime() const
    {
      return lastAccessTime_;
    }

    bool writeCookie(BufferedFile& fp) const;

    bool contains(const Cookie& cookie) const;

    template<typename OutputIterator>
    OutputIterator dumpCookie(OutputIterator out) const
    {
      return std::copy(cookies_.begin(), cookies_.end(), out);
    }

    bool operator==(const DomainEntry& de) const;
    bool operator<(const DomainEntry& de) const;
  };
private:
  typedef std::set<SharedHandle<DomainEntry>,
                   DerefLess<SharedHandle<DomainEntry> > > DomainEntrySet;
  DomainEntrySet domains_;

  template<typename InputIterator>
  void storeCookies(InputIterator first, InputIterator last, time_t now)
  {
    for(; first != last; ++first) {
      store(*first, now);
    }
  }
public:
  CookieStorage();

  ~CookieStorage();

  // Returns true if cookie is stored or updated existing cookie.
  // Returns false if cookie is expired. now is used as last access
  // time.
  bool store(const Cookie& cookie, time_t now);

  // Returns true if cookie is stored or updated existing cookie.
  // Otherwise, returns false. now is used as creation time and last
  // access time.
  bool parseAndStore
  (const std::string& setCookieString,
   const std::string& requestHost,
   const std::string& requestPath,
   time_t now);

  // Finds cookies matched with given criteria and returns them.
  // Matched cookies' lastAccess_ property is updated.
  std::vector<Cookie> criteriaFind(const std::string& requestHost,
                                   const std::string& requestPath,
                                   time_t now, bool secure);

  // Loads Cookies from file denoted by filename.  If compiled with
  // libsqlite3, this method automatically detects the specified file
  // is sqlite3 or just plain text file and calls appropriate parser
  // implementation class.  If Cookies are successfully loaded, this
  // method returns true.  Otherwise, this method returns false.  now
  // is used as creation time and last access time.
  bool load(const std::string& filename, time_t now);
  
  // Saves Cookies in Netspace format which is used in
  // Firefox1.2/Netscape/Mozilla.  If Cookies are successfully saved,
  // this method returns true, otherwise returns false.
  bool saveNsFormat(const std::string& filename);

  // Returns the number of cookies this object stores.
  size_t size() const;
  
  // Returns true if this object contains a cookie x where x == cookie
  // satisfies.
  bool contains(const Cookie& cookie) const;

  // Searches Cookie using given domain, requestHost, requestPath,
  // current time and secure flag. The found Cookies are stored in
  // out.
  void searchCookieByDomainSuffix
  (std::vector<Cookie>& out,
   const std::string& domain,
   const std::string& requestHost,
   const std::string& requestPath,
   time_t now, bool secure);

  template<typename OutputIterator>
  OutputIterator dumpCookie(OutputIterator out) const
  {
    for(DomainEntrySet::iterator i = domains_.begin(), eoi = domains_.end();
        i != eoi; ++i) {
      out = (*i)->dumpCookie(out);
    }
    return out;
  }
};

void swap(CookieStorage::DomainEntry& a, CookieStorage::DomainEntry& b);

} // namespace aria2

namespace std {
template<>
void swap<aria2::CookieStorage::DomainEntry>
(aria2::CookieStorage::DomainEntry& a,
 aria2::CookieStorage::DomainEntry& b);
} // namespace std

#endif // D_COOKIE_STORAGE_H
