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
#include "CookieStorage.h"
#include "Util.h"
#include "LogFactory.h"
#include "Logger.h"
#include "RecoverableException.h"
#include "StringFormat.h"
#include "NsCookieParser.h"
#ifdef HAVE_SQLITE3
# include "Sqlite3MozCookieParser.h"
#endif // HAVE_SQLITE3
#include <algorithm>
#include <fstream>

namespace aria2 {

CookieStorage::CookieStorage():_logger(LogFactory::getInstance()) {}

CookieStorage::~CookieStorage() {}

bool CookieStorage::store(const Cookie& cookie)
{
  if(!cookie.good()) {
    return false;
  }
  std::deque<Cookie>::iterator i = std::find(_cookies.begin(), _cookies.end(),
					     cookie);
  if(i == _cookies.end()) {
    if(cookie.isExpired()) {
      return false;
    } else {
      _cookies.push_back(cookie);
      return true;
    }
  } else if(cookie.isExpired()) {
    _cookies.erase(i);
    return false;
  } else {
    *i = cookie;
    return true;
  }
}

void CookieStorage::storeCookies(const std::deque<Cookie>& cookies)
{
  for(std::deque<Cookie>::const_iterator i = cookies.begin();
      i != cookies.end(); ++i) {
    store(*i);
  }
}

bool CookieStorage::parseAndStore(const std::string& setCookieString,
				  const std::string& requestHost,
				  const std::string& requestPath)
{
  Cookie cookie = _parser.parse(setCookieString, requestHost, requestPath);
  if(cookie.validate(requestHost, requestPath)) {
    return store(cookie);
  } else {
    return false;
  }
}

class CriteriaMatch:public std::unary_function<Cookie, bool> {
private:
  std::string _requestHost;
  std::string _requestPath;
  time_t _date;
  bool _secure;
public:
  CriteriaMatch(const std::string& requestHost, const std::string& requestPath,
		time_t date, bool secure):
    _requestHost(requestHost),
    _requestPath(requestPath),
    _date(date),
    _secure(secure) {}

  bool operator()(const Cookie& cookie) const
  {
    return cookie.match(_requestHost, _requestPath, _date, _secure);
  }
};

class OrderByPathDesc:public std::binary_function<Cookie, Cookie, bool> {
public:
  bool operator()(const Cookie& lhs, const Cookie& rhs) const
  {
    return lhs.getPath() > rhs.getPath();
  }
};

std::deque<Cookie> CookieStorage::criteriaFind(const std::string& requestHost,
					       const std::string& requestPath,
					       time_t date, bool secure) const
{
  std::deque<Cookie> res;
  std::remove_copy_if(_cookies.begin(), _cookies.end(), std::back_inserter(res),
		      std::not1(CriteriaMatch(requestHost, requestPath, date, secure)));
  std::sort(res.begin(), res.end(), OrderByPathDesc());
  return res;
}

size_t CookieStorage::size() const
{
  return _cookies.size();
}

void CookieStorage::load(const std::string& filename)
{
  char header[16]; // "SQLite format 3" plus \0
  {
    std::ifstream s(filename.c_str());
    s.get(header, sizeof(header));
    if(s.bad()) {
      throw RecoverableException
	(StringFormat("Failed to read header of cookie file %s",
		      filename.c_str()).str());
    }
  }
  try {
    if(std::string(header) == "SQLite format 3") {
#ifdef HAVE_SQLITE3
      storeCookies(Sqlite3MozCookieParser().parse(filename));
#else // !HAVE_SQLITE3
      throw RecoverableException
	("Cannot read SQLite3 database because SQLite3 support is disabled by"
	 " configuration.");
#endif // !HAVE_SQLITE3
    } else {
      storeCookies(NsCookieParser().parse(filename));
    }
  } catch(RecoverableException& e) {
    throw RecoverableException
      (StringFormat("Failed to load cookies from %s", filename.c_str()).str(),
       e);
  }
}

} // namespace aria2
