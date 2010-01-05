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
#ifndef _D_COOKIE_STORAGE_H_
#define _D_COOKIE_STORAGE_H_

#include "common.h"

#include <string>
#include <deque>

#include "a2time.h"
#include "Cookie.h"
#include "CookieParser.h"

namespace aria2 {

class Logger;

class CookieStorage {
private:
  std::deque<Cookie> _cookies;

  CookieParser _parser;

  Logger* _logger;

  void storeCookies(const std::deque<Cookie>& cookies);
public:
  CookieStorage();

  ~CookieStorage();

  // Returns true if cookie is stored or updated existing cookie.
  // Returns false if cookie is expired.
  bool store(const Cookie& cookie);

  // Returns true if cookie is stored or updated existing cookie.
  // Otherwise, returns false.
  bool parseAndStore(const std::string& setCookieString,
                     const std::string& requestHost,
                     const std::string& requestPath);

  std::deque<Cookie> criteriaFind(const std::string& requestHost,
                                  const std::string& requestPath,
                                  time_t date, bool secure) const;

  // Loads Cookies from file denoted by filename.  If compiled with
  // libsqlite3, this method automatically detects the specified file
  // is sqlite3 or just plain text file and calls appropriate parser
  // implementation class.  If Cookies are successfully loaded, this
  // method returns true.  Otherwise, this method returns false.
  bool load(const std::string& filename);
  
  // Saves Cookies in Netspace format which is used in
  // Firefox1.2/Netscape/Mozilla.  If Cookies are successfully saved,
  // this method returns true, otherwise returns false.
  bool saveNsFormat(const std::string& filename);

  size_t size() const;
  
  std::deque<Cookie>::const_iterator begin() const
  {
    return _cookies.begin();
  }

  std::deque<Cookie>::const_iterator end() const
  {
    return _cookies.end();
  }

};

} // namespace aria2

#endif // _D_COOKIE_STORAGE_H_
