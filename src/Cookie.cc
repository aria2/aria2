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
#include "Cookie.h"

#include <algorithm>
#include <sstream>

#include "util.h"
#include "A2STR.h"
#include "TimeA2.h"
#include "a2functional.h"

namespace aria2 {

static std::string prependDotIfNotExists(const std::string& domain)
{
  // From RFC2965:
  // * Domain=value
  //   OPTIONAL.  The value of the Domain attribute specifies the domain
  //   for which the cookie is valid.  If an explicitly specified value
  //   does not start with a dot, the user agent supplies a leading dot.
  return (!domain.empty() && domain[0] != '.') ? "."+domain : domain;
}

std::string Cookie::normalizeDomain(const std::string& domain)
{
  if(domain.empty() || util::isNumericHost(domain)) {
    return domain;
  }
  std::string md = prependDotIfNotExists(domain);
  // TODO use util::split to strict verification
  std::string::size_type p = md.find_last_of(".");
  if(p == 0 || p == std::string::npos) {
    md += ".local";
  }
  return util::toLower(prependDotIfNotExists(md));
}

Cookie::Cookie(const std::string& name,
               const std::string& value,
               time_t  expiry,
               const std::string& path,
               const std::string& domain,
               bool secure):
  _name(name),
  _value(value),
  _expiry(expiry),
  _path(path),
  _domain(normalizeDomain(domain)),
  _secure(secure),
  _creationTime(time(0)),
  _lastAccess(_creationTime) {}

Cookie::Cookie(const std::string& name,
               const std::string& value,
               const std::string& path,
               const std::string& domain,
               bool secure):
  _name(name),
  _value(value),
  _expiry(0),
  _path(path),
  _domain(normalizeDomain(domain)),
  _secure(secure),
  _creationTime(time(0)),
  _lastAccess(_creationTime) {}

Cookie::Cookie():_expiry(0), _secure(false), _lastAccess(time(0)) {}

Cookie::~Cookie() {}

std::string Cookie::toString() const
{
  return strconcat(_name, "=", _value);
}

bool Cookie::good() const
{
  return !_name.empty();
}

static bool pathInclude(const std::string& requestPath, const std::string& path)
{
  if(requestPath == path) {
    return true;
  }
  if(util::startsWith(requestPath, path)) {
    if(*path.rbegin() != '/' && requestPath[path.size()] != '/') {
      return false;
    }
  } else if(*path.rbegin() != '/' || *requestPath.rbegin() == '/' ||
            !util::startsWith(requestPath+"/", path)) {
    return false;
  }
  return true;
}

static bool domainMatch(const std::string& normReqHost,
                        const std::string& domain)
{
  // RFC2965 stated that:
  //
  // A Set-Cookie2 with Domain=ajax.com will be accepted, and the
  // value for Domain will be taken to be .ajax.com, because a dot
  // gets prepended to the value.
  //
  // Also original Netscape implementation behaves exactly the same.

  // _domain always starts ".". See Cookie::Cookie().
  return util::endsWith(normReqHost, domain);
}

bool Cookie::match(const std::string& requestHost,
                   const std::string& requestPath,
                   time_t date, bool secure) const
{
  if((secure || (!_secure && !secure)) &&
     (requestHost == _domain || // For default domain or IP address
      domainMatch(normalizeDomain(requestHost), _domain)) &&
     pathInclude(requestPath, _path) &&
     (isSessionCookie() || (date < _expiry))) {
    return true;
  } else {
    return false;
  }
}

bool Cookie::validate(const std::string& requestHost,
                      const std::string& requestPath) const
{
  // If _domain doesn't start with "." or it is IP address, then it
  // must equal to requestHost. Otherwise, do domain tail match.
  if(requestHost != _domain) {
    std::string normReqHost = normalizeDomain(requestHost);
    if(normReqHost != _domain) {
      // domain must start with '.'
      if(*_domain.begin() != '.') {
        return false;
      }
      // domain must not end with '.'
      if(*_domain.rbegin() == '.') {
        return false;
      }
      // domain must include at least one embeded '.'
      if(_domain.size() < 4 || _domain.find(".", 1) == std::string::npos) {
        return false;
      }
      if(!util::endsWith(normReqHost, _domain)) {
        return false;
      }
      // From RFC2965 3.3.2 Rejecting Cookies
      // * The request-host is a HDN (not IP address) and has the form HD,
      //   where D is the value of the Domain attribute, and H is a string
      //   that contains one or more dots.
      size_t dotCount = std::count(normReqHost.begin(),
                                   normReqHost.begin()+
                                   (normReqHost.size()-_domain.size()), '.');
      if(dotCount > 1 || (dotCount == 1 && normReqHost[0] != '.')) {
        return false;
      } 
    }
  }
  if(requestPath != _path) {
    // From RFC2965 3.3.2 Rejecting Cookies
    // * The value for the Path attribute is not a prefix of the request-URI.
    if(!pathInclude(requestPath, _path)) {
      return false;
    }
  }
  return good();
}

bool Cookie::operator==(const Cookie& cookie) const
{
  return _domain == cookie._domain && _path == cookie._path &&
    _name == cookie._name;
}

bool Cookie::isExpired() const
{
  return !_expiry == 0 && Time().getTime() >= _expiry;
}

std::string Cookie::toNsCookieFormat() const
{
  std::stringstream ss;
  ss << _domain << "\t";
  if(util::startsWith(_domain, ".")) {
    ss << "TRUE";
  } else {
    ss << "FALSE";
  }
  ss << "\t";
  ss << _path << "\t";
  if(_secure) {
    ss << "TRUE";
  } else {
    ss << "FALSE";
  }
  ss << "\t";
  ss << _expiry << "\t";
  ss << _name << "\t";
  ss << _value;
  return ss.str();
}

void Cookie::markOriginServerOnly()
{
  if(util::startsWith(_domain, A2STR::DOT_C)) {
    _domain.erase(_domain.begin(), _domain.begin()+1);
  }
}

void Cookie::updateLastAccess()
{
  _lastAccess = time(0);
}

} // namespace aria2
