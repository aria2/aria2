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
#include <cstdio>
#include <algorithm>

#include "util.h"
#include "LogFactory.h"
#include "Logger.h"
#include "DlAbortEx.h"
#include "fmt.h"
#include "NsCookieParser.h"
#include "File.h"
#include "a2functional.h"
#include "A2STR.h"
#include "message.h"
#include "cookie_helper.h"
#include "BufferedFile.h"
#ifdef HAVE_SQLITE3
# include "Sqlite3CookieParserImpl.h"
#endif // HAVE_SQLITE3

namespace aria2 {

CookieStorage::DomainEntry::DomainEntry(const std::string& domain)
 : key_(util::isNumericHost(domain)?domain:cookie::reverseDomainLevel(domain))
{}

CookieStorage::DomainEntry::DomainEntry
(const DomainEntry& c)
  : key_(c.key_),
    lastAccessTime_(c.lastAccessTime_),
    cookies_(c.cookies_)
{}

CookieStorage::DomainEntry::~DomainEntry() {}

CookieStorage::DomainEntry& CookieStorage::DomainEntry::operator=
(const DomainEntry& c)
{
  if(this != &c) {
    key_ = c.key_;
    lastAccessTime_ = c.lastAccessTime_;
    cookies_ = c.cookies_;
  }
  return *this;
}

void CookieStorage::DomainEntry::swap(CookieStorage::DomainEntry& c)
{
  using std::swap;
  swap(key_, c.key_);
  swap(lastAccessTime_, c.lastAccessTime_);
  swap(cookies_, c.cookies_);
}

void swap(CookieStorage::DomainEntry& a, CookieStorage::DomainEntry& b)
{
  a.swap(b);
}

void CookieStorage::DomainEntry::findCookie
(std::vector<Cookie>& out,
 const std::string& requestHost,
 const std::string& requestPath,
 time_t now, bool secure)
{
  for(std::deque<Cookie>::iterator i = cookies_.begin(),
        eoi = cookies_.end(); i != eoi; ++i) {
    if((*i).match(requestHost, requestPath, now, secure)) {
      (*i).setLastAccessTime(now);
      out.push_back(*i);
    }
  }
}

bool CookieStorage::DomainEntry::addCookie(const Cookie& cookie, time_t now)
{
  setLastAccessTime(now);
  std::deque<Cookie>::iterator i =
    std::find(cookies_.begin(), cookies_.end(), cookie);
  if(i == cookies_.end()) {
    if(cookie.isExpired(now)) {
      return false;
    } else {
      if(cookies_.size() >= CookieStorage::MAX_COOKIE_PER_DOMAIN) {
        cookies_.erase
          (std::remove_if(cookies_.begin(), cookies_.end(),
                          std::bind2nd
                          (std::mem_fun_ref(&Cookie::isExpired), now)),
           cookies_.end());
        if(cookies_.size() >= CookieStorage::MAX_COOKIE_PER_DOMAIN) {
          std::deque<Cookie>::iterator m = std::min_element
            (cookies_.begin(), cookies_.end(), LeastRecentAccess<Cookie>());
          *m = cookie;
        } else {
          cookies_.push_back(cookie);
        }
      } else {
        cookies_.push_back(cookie);
      }
      return true;
    }
  } else if(cookie.isExpired(now)) {
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

bool CookieStorage::DomainEntry::writeCookie(BufferedFile& fp) const
{
  for(std::deque<Cookie>::const_iterator i = cookies_.begin(),
        eoi = cookies_.end(); i != eoi; ++i) {
    std::string data = (*i).toNsCookieFormat();
    data += "\n";
    if(fp.write(data.data(), data.size()) != data.size()) {
      return false;
    }
  }
  return true;
}

size_t CookieStorage::DomainEntry::countCookie() const
{
  return cookies_.size();
}

bool CookieStorage::DomainEntry::operator==(const DomainEntry& de) const
{
  return key_ == de.key_;
}

bool CookieStorage::DomainEntry::operator<(const DomainEntry& de) const
{
  return key_ < de.key_;
}

CookieStorage::CookieStorage() {}

CookieStorage::~CookieStorage() {}

namespace {
// See CookieStorageTest::testDomainIsFull() in CookieStorageTest.cc
const size_t DOMAIN_EVICTION_TRIGGER = 2000;

const double DOMAIN_EVICTION_RATE = 0.1;
} // namespace

bool CookieStorage::store(const Cookie& cookie, time_t now)
{
  if(domains_.size() >= DOMAIN_EVICTION_TRIGGER) {
    std::vector<SharedHandle<DomainEntry> > evictions(domains_.begin(),
                                                      domains_.end());
    std::sort(evictions.begin(), evictions.end(),
              LeastRecentAccess<DomainEntry>());
    size_t delnum = (size_t)(evictions.size()*DOMAIN_EVICTION_RATE);
    domains_.clear();
    domains_.insert(evictions.begin()+delnum, evictions.end());
  }
  SharedHandle<DomainEntry> v(new DomainEntry(cookie.getDomain()));
  DomainEntrySet::iterator i = domains_.lower_bound(v);
  bool added = false;
  if(i != domains_.end() && *(*i) == *v) {
    added = (*i)->addCookie(cookie, now);
  } else {
    added = v->addCookie(cookie, now);
    if(added) {
      domains_.insert(i, v);
    }
  }
  return added;
}

bool CookieStorage::parseAndStore
(const std::string& setCookieString,
 const std::string& requestHost,
 const std::string& defaultPath,
 time_t now)
{
  Cookie cookie;
  if(cookie::parse(cookie, setCookieString, requestHost, defaultPath, now)) {
    return store(cookie, now);
  } else {
    return false;
  }
}

struct CookiePathDivider {
  Cookie cookie_;
  int pathDepth_;
  CookiePathDivider(const Cookie& cookie):cookie_(cookie), pathDepth_(0)
  {
    const std::string& path = cookie_.getPath();
    if(!path.empty()) {
      for(size_t i = 1, len = path.size(); i < len; ++i) {
        if(path[i] == '/' && path[i-1] != '/') {
          ++pathDepth_;
        }
      }
      if(path[path.size()-1] != '/') {
        ++pathDepth_;
      }
    }
  }
};

namespace {
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
} // namespace

namespace {
class OrderByPathDepthDesc:public std::binary_function<Cookie, Cookie, bool> {
public:
  bool operator()
  (const CookiePathDivider& lhs, const CookiePathDivider& rhs) const
  {
    // From http://tools.ietf.org/html/rfc6265#section-5.4:
    // 2.  The user agent SHOULD sort the cookie-list in the following
    //    order:
    //
    //    *  Cookies with longer paths are listed before cookies with
    //       shorter paths.
    //
    //    *  Among cookies that have equal-length path fields, cookies with
    //       earlier creation-times are listed before cookies with later
    //       creation-times.
    return lhs.pathDepth_ > rhs.pathDepth_ ||
      (!(rhs.pathDepth_ > lhs.pathDepth_) &&
       lhs.cookie_.getCreationTime() < rhs.cookie_.getCreationTime());
  }
};
} // namespace

void CookieStorage::searchCookieByDomainSuffix
(std::vector<Cookie>& out,
 const std::string& domain,
 const std::string& requestHost,
 const std::string& requestPath,
 time_t now, bool secure)
{
  SharedHandle<DomainEntry> v(new DomainEntry(domain));
  DomainEntrySet::iterator i = domains_.lower_bound(v);
  if(i != domains_.end() && *(*i) == *v) {
    (*i)->setLastAccessTime(now);
    (*i)->findCookie(out, requestHost, requestPath, now, secure);
  }
}

bool CookieStorage::contains(const Cookie& cookie) const
{
  SharedHandle<DomainEntry> v(new DomainEntry(cookie.getDomain()));
  DomainEntrySet::iterator i = domains_.find(v);
  if(i != domains_.end()) {
    return (*i)->contains(cookie);
  } else {
    return false;
  }
}

std::vector<Cookie> CookieStorage::criteriaFind
(const std::string& requestHost,
 const std::string& requestPath,
 time_t now,
 bool secure)
{
  std::vector<Cookie> res;
  if(requestPath.empty()) {
    return res;
  }
  if(util::isNumericHost(requestHost)) {
    searchCookieByDomainSuffix
      (res, requestHost, requestHost, requestPath, now, secure);
  } else {
    std::vector<Scip> levels;
    util::splitIter(requestHost.begin(), requestHost.end(),
                    std::back_inserter(levels), '.');
    std::string domain;
    for(std::vector<Scip>::const_reverse_iterator i =
          levels.rbegin(), eoi = levels.rend();
        i != eoi; ++i, domain.insert(domain.begin(), '.')) {
      domain.insert(domain.begin(), (*i).first, (*i).second);
      searchCookieByDomainSuffix
        (res, domain, requestHost, requestPath, now, secure);
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
  for(DomainEntrySet::iterator i = domains_.begin(), eoi = domains_.end();
      i != eoi; ++i) {
    numCookie += (*i)->countCookie();
  }
  return numCookie;
}

bool CookieStorage::load(const std::string& filename, time_t now)
{
  char header[16]; // "SQLite format 3" plus \0
  size_t headlen;
  {
    BufferedFile fp(filename, BufferedFile::READ);
    if(!fp) {
      A2_LOG_ERROR(fmt("Failed to open cookie file %s", filename.c_str()));
      return false;
    }
    headlen = fp.read(header, sizeof(header));
  }
  try {
    if(headlen == 16 && memcmp(header, "SQLite format 3\0", 16) == 0) {
#ifdef HAVE_SQLITE3
      std::vector<Cookie> cookies;
      try {
        Sqlite3MozCookieParser(filename).parse(cookies);
      } catch(RecoverableException& e) {
        A2_LOG_INFO_EX(EX_EXCEPTION_CAUGHT, e);
        A2_LOG_INFO("This does not look like Firefox3 cookie file."
                    " Retrying, assuming it is Chromium cookie file.");
        // Try chrome cookie format
        Sqlite3ChromiumCookieParser(filename).parse(cookies);
      }
      storeCookies(cookies.begin(), cookies.end(), now);
#else // !HAVE_SQLITE3
      throw DL_ABORT_EX
        ("Cannot read SQLite3 database because SQLite3 support is disabled by"
         " configuration.");
#endif // !HAVE_SQLITE3
    } else {
      std::vector<Cookie> cookies = NsCookieParser().parse(filename, now);
      storeCookies(cookies.begin(), cookies.end(), now);
    }
    return true;
  } catch(RecoverableException& e) {
    A2_LOG_ERROR(fmt("Failed to load cookies from %s", filename.c_str()));
    return false;
  }
}

bool CookieStorage::saveNsFormat(const std::string& filename)
{
  std::string tempfilename = filename;
  tempfilename += "__temp";
  {
    BufferedFile fp(tempfilename, BufferedFile::WRITE);
    if(!fp) {
      A2_LOG_ERROR(fmt("Cannot create cookie file %s", filename.c_str()));
      return false;
    }
    for(DomainEntrySet::iterator i = domains_.begin(), eoi = domains_.end();
        i != eoi; ++i) {
      if(!(*i)->writeCookie(fp)) {
        A2_LOG_ERROR(fmt("Failed to save cookies to %s", filename.c_str()));
        return false;
      }
    }
    if(fp.close() == EOF) {
      A2_LOG_ERROR(fmt("Failed to save cookies to %s", filename.c_str()));
      return false;
    }  
  }
  if(File(tempfilename).renameTo(filename)) {
    return true;
  } else {
    A2_LOG_ERROR(fmt("Could not rename file %s as %s",
                     tempfilename.c_str(),
                     filename.c_str()));
    return false;
  }
}

} // namespace aria2

namespace std {
template<>
void swap<aria2::CookieStorage::DomainEntry>
(aria2::CookieStorage::DomainEntry& a,
 aria2::CookieStorage::DomainEntry& b)
{
  a.swap(b);
}
} // namespace std

