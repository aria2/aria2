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
#include "A2STR.h"
#include "message.h"
#ifdef HAVE_SQLITE3
# include "Sqlite3CookieParserImpl.h"
#endif // HAVE_SQLITE3

namespace aria2 {

CookieStorage::DomainEntry::DomainEntry
(const std::string& domain):key_(domain)
{
  std::reverse(key_.begin(), key_.end());
}

void CookieStorage::DomainEntry::updateLastAccess()
{
  lastAccess_ = time(0);
}

bool CookieStorage::DomainEntry::addCookie(const Cookie& cookie)
{
  updateLastAccess();
  std::deque<Cookie>::iterator i = std::find(cookies_.begin(), cookies_.end(),
                                             cookie);
  if(i == cookies_.end()) {
    if(cookie.isExpired()) {
      return false;
    } else {
      if(cookies_.size() >= CookieStorage::MAX_COOKIE_PER_DOMAIN) {
        std::deque<Cookie>::iterator m = std::min_element
          (cookies_.begin(), cookies_.end(), LeastRecentAccess<Cookie>());
        *m = cookie;
      } else {
        cookies_.push_back(cookie);
      }
      return true;
    }
  } else if(cookie.isExpired()) {
    cookies_.erase(i);
    return false;
  } else {
    *i = cookie;
    return true;
  }
}

bool CookieStorage::DomainEntry::contains(const Cookie& cookie) const
{
  return std::find(cookies_.begin(), cookies_.end(), cookie) != cookies_.end();
}

void CookieStorage::DomainEntry::writeCookie(std::ostream& o) const
{
  for(std::deque<Cookie>::const_iterator i = cookies_.begin(),
        eoi = cookies_.end(); i != eoi; ++i) {
    o << (*i).toNsCookieFormat() << "\n";
  }
}

CookieStorage::CookieStorage():logger_(LogFactory::getInstance()) {}

CookieStorage::~CookieStorage() {}

// See CookieStorageTest::testDomainIsFull() in CookieStorageTest.cc
static const size_t DOMAIN_EVICTION_TRIGGER = 2000;

static const double DOMAIN_EVICTION_RATE = 0.1;

bool CookieStorage::store(const Cookie& cookie)
{
  if(!cookie.good()) {
    return false;
  }
  if(domains_.size() >= DOMAIN_EVICTION_TRIGGER) {
    std::sort(domains_.begin(), domains_.end(),
              LeastRecentAccess<DomainEntry>());
    size_t delnum = (size_t)(domains_.size()*DOMAIN_EVICTION_RATE);
    domains_.erase(domains_.begin(), domains_.begin()+delnum);
    std::sort(domains_.begin(), domains_.end());
  }
  DomainEntry v(cookie.getDomain());
  std::deque<DomainEntry>::iterator i =
    std::lower_bound(domains_.begin(), domains_.end(), v);
  bool added = false;
  if(i != domains_.end() && (*i).getKey() == v.getKey()) {
    added = (*i).addCookie(cookie);
  } else {
    added = v.addCookie(cookie);
    if(added) {
      domains_.insert(i, v);
    }
  }
  return added;
}

bool CookieStorage::parseAndStore(const std::string& setCookieString,
                                  const std::string& requestHost,
                                  const std::string& requestPath)
{
  Cookie cookie = parser_.parse(setCookieString, requestHost, requestPath);
  if(cookie.validate(requestHost, requestPath)) {
    return store(cookie);
  } else {
    return false;
  }
}

struct CookiePathDivider {
  Cookie cookie_;
  int pathDepth_;
  CookiePathDivider(const Cookie& cookie):cookie_(cookie)
  {
    std::vector<std::string> paths;
    util::split(cookie_.getPath(), std::back_inserter(paths), A2STR::SLASH_C);
    pathDepth_ = paths.size();
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
    return cookiePathDivider.cookie_;
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
    int comp = lhs.pathDepth_-rhs.pathDepth_;
    if(comp == 0) {
      return lhs.cookie_.getCreationTime() < rhs.cookie_.getCreationTime();
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
    std::lower_bound(domains_.begin(), domains_.end(), v);
  if(i != domains_.end() && (*i).getKey() == v.getKey()) {
    return (*i).contains(cookie);
  } else {
    return false;
  }
}

std::vector<Cookie> CookieStorage::criteriaFind(const std::string& requestHost,
                                               const std::string& requestPath,
                                               time_t date, bool secure)
{
  std::vector<Cookie> res;
  bool numericHost = util::isNumericHost(requestHost);
  searchCookieByDomainSuffix
    ((!numericHost && requestHost.find(A2STR::DOT_C) == std::string::npos)?
     requestHost+".local":requestHost,
     domains_.begin(), domains_.end(),
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
          domainComponents.begin()+1, eoi = domainComponents.end();
        di != eoi; ++di) {
      domain = strconcat(A2STR::DOT_C, *di, domain);
      searchCookieByDomainSuffix(domain, domains_.begin(), domains_.end(),
                                 std::back_inserter(res),
                                 normRequestHost, requestPath, date, secure);
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
  for(std::deque<DomainEntry>::const_iterator i = domains_.begin(),
        eoi = domains_.end(); i != eoi; ++i) {
    numCookie += (*i).countCookie();
  }
  return numCookie;
}

bool CookieStorage::load(const std::string& filename)
{
  char header[16]; // "SQLite format 3" plus \0
  std::ifstream s(filename.c_str(), std::ios::binary);
  if(!s) {
    logger_->error("Failed to open cookie file %s", filename.c_str());
    return false;
  }
  s.get(header, sizeof(header));
  if(!s) {
    logger_->error("Failed to read header of cookie file %s",
                   filename.c_str());
    return false;
  }
  try {
    if(std::string(header) == "SQLite format 3") {
#ifdef HAVE_SQLITE3
      std::vector<Cookie> cookies;
      try {
        Sqlite3MozCookieParser(filename).parse(cookies);
      } catch(RecoverableException& e) {
        if(logger_->info()) {
          logger_->info(EX_EXCEPTION_CAUGHT, e);
          logger_->info("This does not look like Firefox3 cookie file."
                        " Retrying, assuming it is Chromium cookie file.");
        }
        // Try chrome cookie format
        Sqlite3ChromiumCookieParser(filename).parse(cookies);
      }
      storeCookies(cookies.begin(), cookies.end());
#else // !HAVE_SQLITE3
      throw DL_ABORT_EX
        ("Cannot read SQLite3 database because SQLite3 support is disabled by"
         " configuration.");
#endif // !HAVE_SQLITE3
    } else {
      std::vector<Cookie> cookies = NsCookieParser().parse(filename);
      storeCookies(cookies.begin(), cookies.end());
    }
    return true;
  } catch(RecoverableException& e) {
    logger_->error("Failed to load cookies from %s", filename.c_str());
    return false;
  }
}

bool CookieStorage::saveNsFormat(const std::string& filename)
{
  std::string tempfilename = filename+"__temp";
  {
    std::ofstream o(tempfilename.c_str(), std::ios::binary);
    if(!o) {
      logger_->error("Cannot create cookie file %s, cause %s",
                     filename.c_str(), strerror(errno));
      return false;
    }
    for(std::deque<DomainEntry>::const_iterator i = domains_.begin(),
          eoi = domains_.end(); i != eoi; ++i) {
      (*i).writeCookie(o);
    }
    o.flush();
    if(!o) {
      logger_->error("Failed to save cookies to %s, cause %s",
                     filename.c_str(), strerror(errno));
      return false;
    }  
  }
  if(File(tempfilename).renameTo(filename)) {
    return true;
  } else {
    logger_->error("Could not rename file %s as %s",
                   tempfilename.c_str(), filename.c_str());
    return false;
  }
}

} // namespace aria2
