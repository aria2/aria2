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
#include "cookie_helper.h"

#include <cstring>
#include <vector>
#include <limits>

#include "util.h"
#include "array_fun.h"
#include "Cookie.h"
#include "a2functional.h"

namespace aria2 {

namespace cookie {

namespace {
bool isDelimiter(unsigned char c)
{
  return c == 0x09u || in(c, 0x20u, 0x2fu) || in(c, 0x3bu, 0x40u) ||
         in(c, 0x5bu, 0x60u) || in(c, 0x7bu, 0x7eu);
}
} // namespace

namespace {
std::string::const_iterator getNextDigit(std::string::const_iterator first,
                                         std::string::const_iterator last)
{
  for (; first != last && in(static_cast<unsigned char>(*first), 0x30u, 0x39u);
       ++first)
    ;
  return first;
}
} // namespace

namespace {
template <typename InputIterator>
int getInteger(InputIterator first, InputIterator last)
{
  int res = 0;
  // We assume *dest won't overflow.
  for (; first != last; ++first) {
    res *= 10;
    res += (*first) - '0';
  }
  return res;
}
} // namespace

bool parseDate(time_t& time, std::string::const_iterator first,
               std::string::const_iterator last)
{
  // Following algorithm is described in
  // http://tools.ietf.org/html/rfc6265#section-5.1.1
  std::vector<std::string> dateTokens;
  for (std::string::const_iterator i = first, eoi = last; i != eoi;) {
    unsigned char c = *i;
    if (isDelimiter(c)) {
      ++i;
      continue;
    }
    std::string::const_iterator s = i;
    for (; s != eoi && !isDelimiter(static_cast<unsigned char>(*s)); ++s)
      ;
    dateTokens.push_back(std::string(i, s));
    i = s;
  }
  int dayOfMonth = 0;
  bool foundDayOfMonth = false;
  int month = 0;
  bool foundMonth = false;
  int year = 0;
  bool foundYear = false;
  int hour = 0;
  int minute = 0;
  int second = 0;
  bool foundTime = false;
  for (std::vector<std::string>::const_iterator i = dateTokens.begin(),
                                                eoi = dateTokens.end();
       i != eoi; ++i) {
    if (!foundTime) {
      std::string::const_iterator hEnd;
      std::string::const_iterator mEnd;
      std::string::const_iterator sEnd;
      hEnd = getNextDigit((*i).begin(), (*i).end());
      size_t len = std::distance((*i).begin(), hEnd);
      if (len == 0 || 2 < len || hEnd == (*i).end() || *hEnd != ':') {
        goto NOT_TIME;
      }
      mEnd = getNextDigit(hEnd + 1, (*i).end());
      len = std::distance(hEnd + 1, mEnd);
      if (len == 0 || 2 < len || mEnd == (*i).end() || *mEnd != ':') {
        goto NOT_TIME;
      }
      sEnd = getNextDigit(mEnd + 1, (*i).end());
      len = std::distance(mEnd + 1, sEnd);
      if (len == 0 || 2 < len) {
        goto NOT_TIME;
      }
      foundTime = true;
      hour = getInteger((*i).begin(), hEnd);
      minute = getInteger(hEnd + 1, mEnd);
      second = getInteger(mEnd + 1, sEnd);
      continue;
    NOT_TIME:;
    }
    if (!foundDayOfMonth) {
      std::string::const_iterator j = getNextDigit((*i).begin(), (*i).end());
      size_t len = std::distance((*i).begin(), j);
      if (1 <= len && len <= 2) {
        foundDayOfMonth = true;
        dayOfMonth = getInteger((*i).begin(), j);
        continue;
      }
    }
    if (!foundMonth) {
      static const char MONTH[][12] = {"jan", "feb", "mar", "apr",
                                       "may", "jun", "jul", "aug",
                                       "sep", "oct", "nov", "dec"};
      if ((*i).size() >= 3) {
        bool found = false;
        size_t j;
        for (j = 0; j < 12; ++j) {
          if (util::strieq((*i).begin(), (*i).begin() + 3, &MONTH[j][0],
                           &MONTH[j][3])) {
            found = true;
            break;
          }
        }
        if (found) {
          foundMonth = true;
          month = j + 1;
          continue;
        }
      }
    }
    if (!foundYear) {
      std::string::const_iterator j = getNextDigit((*i).begin(), (*i).end());
      size_t len = std::distance((*i).begin(), j);
      if (1 <= len && len <= 4) {
        foundYear = true;
        year = getInteger((*i).begin(), j);
        continue;
      }
    }
  }
  if (in(year, 70, 99)) {
    year += 1900;
  }
  else if (in(year, 0, 69)) {
    year += 2000;
  }
  if (!foundDayOfMonth || !foundMonth || !foundYear || !foundTime ||
      !in(dayOfMonth, 1, 31) || year < 1601 || hour > 23 || minute > 59 ||
      second > 59) {
    return false;
  }
  if ((month == 4 || month == 6 || month == 9 || month == 11) &&
      dayOfMonth > 30) {
    return false;
  }
  if (month == 2) {
    if ((year % 4 == 0 && year % 100 != 0) || year % 400 == 0) {
      if (dayOfMonth > 29) {
        return false;
      }
    }
    else if (dayOfMonth > 28) {
      return false;
    }
  }

  tm timespec;
  memset(&timespec, 0, sizeof(timespec));
  timespec.tm_sec = second;
  timespec.tm_min = minute;
  timespec.tm_hour = hour;
  timespec.tm_mday = dayOfMonth;
  timespec.tm_mon = month - 1;
  timespec.tm_year = year - 1900;
  time = timegm(&timespec);

  return time != -1;
}

std::unique_ptr<Cookie> parse(const std::string& cookieStr,
                              const std::string& requestHost,
                              const std::string& defaultPath,
                              time_t creationTime)
{
  // This implementation is based on the algorithm listed in
  // http://tools.ietf.org/html/rfc6265
  std::string::const_iterator nvEnd = cookieStr.begin();
  std::string::const_iterator end = cookieStr.end();
  for (; nvEnd != end && *nvEnd != ';'; ++nvEnd)
    ;
  std::string::const_iterator eq = cookieStr.begin();
  for (; eq != nvEnd && *eq != '='; ++eq)
    ;
  if (eq == nvEnd) {
    return nullptr;
  }
  std::pair<std::string::const_iterator, std::string::const_iterator> p =
      util::stripIter(cookieStr.begin(), eq);
  if (p.first == p.second) {
    return nullptr;
  }
  Scip cookieName(p.first, p.second);
  p = util::stripIter(eq + 1, nvEnd);
  p = util::stripIter(p.first, p.second, "\"");
  Scip cookieValue(p.first, p.second);
  time_t expiryTime = 0;
  bool foundExpires = false;
  bool persistent = false;
  time_t maxAge = 0;
  bool foundMaxAge = false;
  std::string cookieDomain;
  bool hostOnly = false;
  std::string cookiePath;
  bool secure = false;
  bool httpOnly = false;

  if (nvEnd != end) {
    ++nvEnd;
  }
  for (std::string::const_iterator i = nvEnd; i != end;) {
    std::string::const_iterator j = std::find(i, end, ';');
    std::string::const_iterator eq = std::find(i, j, '=');
    p = util::stripIter(i, eq);
    std::pair<std::string::const_iterator, std::string::const_iterator> attrp;
    if (eq == j) {
      attrp.first = attrp.second = j;
    }
    else {
      attrp = util::stripIter(eq + 1, j);
    }
    i = j;
    if (j != end) {
      ++i;
    }
    if (util::strieq(p.first, p.second, "expires")) {
      if (parseDate(expiryTime, attrp.first, attrp.second)) {
        foundExpires = true;
      }
      else {
        return nullptr;
      }
    }
    else if (util::strieq(p.first, p.second, "max-age")) {
      if (attrp.first == attrp.second ||
          (!in(static_cast<unsigned char>(*attrp.first), 0x30u, 0x39u) &&
           *attrp.first != '-')) {
        return nullptr;
      }
      for (std::string::const_iterator s = attrp.first + 1, eos = attrp.second;
           s != eos; ++s) {
        if (!in(static_cast<unsigned char>(*s), 0x30u, 0x39u)) {
          return nullptr;
        }
      }
      int64_t delta;
      if (util::parseLLIntNoThrow(delta,
                                  std::string(attrp.first, attrp.second))) {
        foundMaxAge = true;
        if (delta <= 0) {
          maxAge = 0;
        }
        else {
          int64_t n = creationTime;
          n += delta;
          if (n < 0 || std::numeric_limits<time_t>::max() < n) {
            maxAge = std::numeric_limits<time_t>::max();
          }
          else {
            maxAge = n;
          }
        }
      }
      else {
        return nullptr;
      }
    }
    else if (util::strieq(p.first, p.second, "domain")) {
      if (attrp.first == attrp.second) {
        return nullptr;
      }
      std::string::const_iterator noDot = attrp.first;
      std::string::const_iterator end = attrp.second;
      for (; noDot != end && *noDot == '.'; ++noDot)
        ;
      if (noDot == end) {
        return nullptr;
      }
      cookieDomain.assign(noDot, end);
    }
    else if (util::strieq(p.first, p.second, "path")) {
      if (goodPath(attrp.first, attrp.second)) {
        cookiePath.assign(attrp.first, attrp.second);
      }
      else {
        cookiePath = defaultPath;
      }
    }
    else if (util::strieq(p.first, p.second, "secure")) {
      secure = true;
    }
    else if (util::strieq(p.first, p.second, "httponly")) {
      httpOnly = true;
    }
  }

  if (foundMaxAge) {
    expiryTime = maxAge;
    persistent = true;
  }
  else if (foundExpires) {
    persistent = true;
  }
  else {
    expiryTime = std::numeric_limits<time_t>::max();
    persistent = false;
  }

  std::string canonicalizedHost = canonicalizeHost(requestHost);
  if (cookieDomain.empty()) {
    hostOnly = true;
    cookieDomain = canonicalizedHost;
  }
  else if (domainMatch(canonicalizedHost, cookieDomain)) {
    hostOnly = util::isNumericHost(canonicalizedHost);
  }
  else {
    return nullptr;
  }

  if (cookiePath.empty()) {
    cookiePath = defaultPath;
  }

  auto cookie = make_unique<Cookie>();
  cookie->setName(cookieName.first, cookieName.second);
  cookie->setValue(cookieValue.first, cookieValue.second);
  cookie->setExpiryTime(expiryTime);
  cookie->setPersistent(persistent);
  cookie->setDomain(std::move(cookieDomain));
  cookie->setHostOnly(hostOnly);
  cookie->setPath(std::move(cookiePath));
  cookie->setSecure(secure);
  cookie->setHttpOnly(httpOnly);
  cookie->setCreationTime(creationTime);
  cookie->setLastAccessTime(creationTime);

  return cookie;
}

bool goodPath(std::string::const_iterator first,
              std::string::const_iterator last)
{
  return first != last && *first == '/';
}

std::string canonicalizeHost(const std::string& host)
{
  return util::toLower(host);
}

bool domainMatch(const std::string& requestHost, const std::string& domain)
{
  return requestHost == domain ||
         (util::endsWith(requestHost, domain) &&
          requestHost[requestHost.size() - domain.size() - 1] == '.' &&
          !util::isNumericHost(requestHost));
}

bool pathMatch(const std::string& requestPath, const std::string& path)
{
  return requestPath == path ||
         (util::startsWith(requestPath, path) &&
          (path[path.size() - 1] == '/' || requestPath[path.size()] == '/'));
}

std::string reverseDomainLevel(const std::string& domain)
{
  std::string r;
  if (domain.empty()) {
    return r;
  }
  r.reserve(domain.size());
  // Cut trailing dots
  std::string::const_iterator s = domain.end() - 1;
  for (; *s == '.'; --s) {
    if (s == domain.begin()) {
      return r;
    }
  }
  std::string::const_iterator t = s + 1;
  for (;; --s) {
    if (*s == '.') {
      r.append(s + 1, t);
      r += '.';
      t = s;
    }
    if (s == domain.begin()) {
      r.append(s, t);
      break;
    }
  }
  return r;
}

} // namespace cookie

} // namespace aria2
