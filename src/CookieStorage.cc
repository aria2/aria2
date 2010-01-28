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
#include "CookieStorage.h"

#include <cstring>
#include <algorithm>
#include <fstream>

#include "util.h"
#include "LogFactory.h"
#include "Logger.h"
#include "DlAbortEx.h"
#include "StringFormat.h"
#include "NsCookieParser.h"
#include "File.h"
#include "a2functional.h"
#ifdef HAVE_SQLITE3
# include "Sqlite3MozCookieParser.h"
#endif // HAVE_SQLITE3

namespace aria2 {

CookieStorage::DomainEntry::DomainEntry
(const std::string& domain):_key(domain)
{
  std::reverse(_key.begin(), _key.end());
}

void CookieStorage::DomainEntry::updateLastAccess()
{
  _lastAccess = time(0);
}

bool CookieStorage::DomainEntry::addCookie(const Cookie& cookie)
{
  updateLastAccess();
  std::deque<Cookie>::iterator i = std::find(_cookies.begin(), _cookies.end(),
                                             cookie);
  if(i == _cookies.end()) {
    if(cookie.isExpired()) {
      return false;
    } else {
      if(_cookies.size() >= CookieStorage::MAX_COOKIE_PER_DOMAIN) {
        std::deque<Cookie>::iterator m = std::min_element
          (_cookies.begin(), _cookies.end(), LeastRecentAccess<Cookie>());
        *m = cookie;
      } else {
        _cookies.push_back(cookie);
      }
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

bool CookieStorage::DomainEntry::contains(const Cookie& cookie) const
{
  return std::find(_cookies.begin(), _cookies.end(), cookie) != _cookies.end();
}

void CookieStorage::DomainEntry::writeCookie(std::ostream& o) const
{
  for(std::deque<Cookie>::const_iterator i = _cookies.begin();
      i != _cookies.end(); ++i) {
    o << (*i).toNsCookieFormat() << "\n";
  }
}

CookieStorage::CookieStorage():_logger(LogFactory::getInstance()) {}

CookieStorage::~CookieStorage() {}

// See CookieStorageTest::testDomainIsFull() in CookieStorageTest.cc
static const size_t DOMAIN_EVICTION_TRIGGER = 600;

static const double DOMAIN_EVICTION_RATE = 0.1;

bool CookieStorage::store(const Cookie& cookie)
{
  if(!cookie.good()) {
    return false;
  }
  if(_domains.size() >= DOMAIN_EVICTION_TRIGGER) {
    std::sort(_domains.begin(), _domains.end(),
              LeastRecentAccess<DomainEntry>());
    size_t delnum = _domains.size()*DOMAIN_EVICTION_RATE;
    _domains.erase(_domains.begin(), _domains.begin()+delnum);
    std::sort(_domains.begin(), _domains.end());
  }
  DomainEntry v(cookie.getDomain());
  std::deque<DomainEntry>::iterator i =
    std::lower_bound(_domains.begin(), _domains.end(), v);
  bool added = false;
  if(i != _domains.end() && (*i).getKey() == v.getKey()) {
    added = (*i).addCookie(cookie);
  } else {
    added = v.addCookie(cookie);
    if(added) {
      _domains.insert(i, v);
    }
  }
  return added;
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

struct CookiePathDivider {
  Cookie _cookie;
  int _pathDepth;
  CookiePathDivider(const Cookie& cookie):_cookie(cookie)
  {
    std::vector<std::string> paths;
    util::split(_cookie.getPath(), std::back_inserter(paths), A2STR::SLASH_C);
    _pathDepth = paths.size();
  }
};

class CookiePathDividerConverter {
public:
  CookiePathDivider operator()(const Cookie& cookie) const
  {
    return CookiePathDivider(cookie);
  }

  Cookie operator()(const CookiePathDivider& cookiePathDivider) const
  {
    return cookiePathDivider._cookie;
  }
};

class OrderByPathDepthDesc:public std::binary_function<Cookie, Cookie, bool> {
public:
  bool operator()
  (const CookiePathDivider& lhs, const CookiePathDivider& rhs) const
  {
    // Sort by path-length.
    //
    // RFC2965 says: Note that the NAME=VALUE pair for the cookie with
    // the more specific Path attribute, /acme/ammo, comes before the
    // one with the less specific Path attribute, /acme.  Further note
    // that the same cookie name appears more than once.
    //
    // Netscape spec says: When sending cookies to a server, all
    // cookies with a more specific path mapping should be sent before
    // cookies with less specific path mappings. For example, a cookie
    // "name1=foo" with a path mapping of "/" should be sent after a
    // cookie "name1=foo2" with a path mapping of "/bar" if they are
    // both to be sent.
    int comp = lhs._pathDepth-rhs._pathDepth;
    if(comp == 0) {
      return lhs._cookie.getCreationTime() < rhs._cookie.getCreationTime();
    } else {
      return comp > 0;
    }
  }
};

template<typename DomainInputIterator, typename CookieOutputIterator>
static void searchCookieByDomainSuffix
(const std::string& domain,
 DomainInputIterator first, DomainInputIterator last, CookieOutputIterator out,
 const std::string& requestHost,
 const std::string& requestPath,
 time_t date, bool secure)
{
  CookieStorage::DomainEntry v(domain);
  std::deque<CookieStorage::DomainEntry>::iterator i =
    std::lower_bound(first, last, v);
  if(i != last && (*i).getKey() == v.getKey()) {
    (*i).updateLastAccess();
    (*i).findCookie(out, requestHost, requestPath, date, secure);
  }
}

bool CookieStorage::contains(const Cookie& cookie) const
{
  CookieStorage::DomainEntry v(cookie.getDomain());
  std::deque<CookieStorage::DomainEntry>::const_iterator i =
    std::lower_bound(_domains.begin(), _domains.end(), v);
  if(i != _domains.end() && (*i).getKey() == v.getKey()) {
    return (*i).contains(cookie);
  } else {
    return false;
  }
}

std::deque<Cookie> CookieStorage::criteriaFind(const std::string& requestHost,
                                               const std::string& requestPath,
                                               time_t date, bool secure)
{
  std::deque<Cookie> res;
  bool numericHost = util::isNumbersAndDotsNotation(requestHost);
  searchCookieByDomainSuffix
    ((!numericHost && requestHost.find(A2STR::DOT_C) == std::string::npos)?
     requestHost+".local":requestHost,
     _domains.begin(), _domains.end(),
     std::back_inserter(res),
     requestHost, requestPath, date, secure);
  if(!numericHost) {
    std::string normRequestHost = Cookie::normalizeDomain(requestHost);
    std::vector<std::string> domainComponents;
    util::split(normRequestHost, std::back_inserter(domainComponents),
                A2STR::DOT_C);
    if(domainComponents.size() <= 1) {
      return res;
    }
    std::reverse(domainComponents.begin(), domainComponents.end());
    std::string domain = A2STR::DOT_C;
    domain += domainComponents[0];
    for(std::vector<std::string>::const_iterator di =
          domainComponents.begin()+1; di != domainComponents.end(); ++di) {
      domain = strconcat(A2STR::DOT_C, *di, domain);
      const size_t prenum = res.size();
      searchCookieByDomainSuffix(domain, _domains.begin(), _domains.end(),
                                 std::back_inserter(res),
                                 normRequestHost, requestPath, date, secure);
      if(prenum == res.size()) {
        break;
      } 
    }
  }
  std::vector<CookiePathDivider> divs;
  std::transform(res.begin(), res.end(), std::back_inserter(divs),
                 CookiePathDividerConverter());
  std::sort(divs.begin(), divs.end(), OrderByPathDepthDesc());
  std::transform(divs.begin(), divs.end(), res.begin(),
                 CookiePathDividerConverter());
  return res;
}

size_t CookieStorage::size() const
{
  size_t numCookie = 0;
  for(std::deque<DomainEntry>::const_iterator i = _domains.begin();
      i != _domains.end(); ++i) {
    numCookie += (*i).countCookie();
  }
  return numCookie;
}

bool CookieStorage::load(const std::string& filename)
{
  char header[16]; // "SQLite format 3" plus \0
  std::ifstream s(filename.c_str(), std::ios::binary);
  if(!s) {
    _logger->error("Failed to open cookie file %s", filename.c_str());
    return false;
  }
  s.get(header, sizeof(header));
  if(!s) {
    _logger->error("Failed to read header of cookie file %s",
                   filename.c_str());
    return false;
  }
  try {
    if(std::string(header) == "SQLite format 3") {
#ifdef HAVE_SQLITE3
      storeCookies(Sqlite3MozCookieParser().parse(filename));
#else // !HAVE_SQLITE3
      throw DL_ABORT_EX
        ("Cannot read SQLite3 database because SQLite3 support is disabled by"
         " configuration.");
#endif // !HAVE_SQLITE3
    } else {
      storeCookies(NsCookieParser().parse(filename));
    }
    return true;
  } catch(RecoverableException& e) {
    _logger->error("Failed to load cookies from %s", filename.c_str());
    return false;
  }
}

bool CookieStorage::saveNsFormat(const std::string& filename)
{
  std::string tempfilename = filename+"__temp";
  {
    std::ofstream o(tempfilename.c_str(), std::ios::binary);
    if(!o) {
      _logger->error("Cannot create cookie file %s, cause %s",
                     filename.c_str(), strerror(errno));
      return false;
    }
    for(std::deque<DomainEntry>::const_iterator i = _domains.begin();
        i != _domains.end(); ++i) {
      (*i).writeCookie(o);
    }
    o.flush();
    if(!o) {
      _logger->error("Failed to save cookies to %s, cause %s",
                     filename.c_str(), strerror(errno));
      return false;
    }  
  }
  if(File(tempfilename).renameTo(filename)) {
    return true;
  } else {
    _logger->error("Could not rename file %s as %s",
                   tempfilename.c_str(), filename.c_str());
    return false;
  }
}

} // namespace aria2
