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
  return (!domain.empty() && domain[0] != '.') ? A2STR::DOT_C+domain : domain;
}

std::string Cookie::normalizeDomain(const std::string& domain)
{
  if(domain.empty() || util::isNumericHost(domain)) {
    return domain;
  }
  std::string md = prependDotIfNotExists(domain);
  // TODO use util::split to strict verification
  std::string::size_type p = md.find_last_of(A2STR::DOT_C);
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
  name_(name),
  value_(value),
  expiry_(expiry),
  path_(path),
  domain_(normalizeDomain(domain)),
  secure_(secure),
  creationTime_(time(0)),
  lastAccess_(creationTime_) {}

Cookie::Cookie(const std::string& name,
               const std::string& value,
               const std::string& path,
               const std::string& domain,
               bool secure):
  name_(name),
  value_(value),
  expiry_(0),
  path_(path),
  domain_(normalizeDomain(domain)),
  secure_(secure),
  creationTime_(time(0)),
  lastAccess_(creationTime_) {}

Cookie::Cookie():expiry_(0), secure_(false), lastAccess_(time(0)) {}

Cookie::~Cookie() {}

std::string Cookie::toString() const
{
  return strconcat(name_, "=", value_);
}

bool Cookie::good() const
{
  return !name_.empty();
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

  // domain_ always starts ".". See Cookie::Cookie().
  return util::endsWith(normReqHost, domain);
}

bool Cookie::match(const std::string& requestHost,
                   const std::string& requestPath,
                   time_t date, bool secure) const
{
  if((secure || (!secure_ && !secure)) &&
     (requestHost == domain_ || // For default domain or IP address
      domainMatch(normalizeDomain(requestHost), domain_)) &&
     pathInclude(requestPath, path_) &&
     (isSessionCookie() || (date < expiry_))) {
    return true;
  } else {
    return false;
  }
}

bool Cookie::validate(const std::string& requestHost,
                      const std::string& requestPath) const
{
  // If domain_ doesn't start with "." or it is IP address, then it
  // must equal to requestHost. Otherwise, do domain tail match.
  if(requestHost != domain_) {
    std::string normReqHost = normalizeDomain(requestHost);
    if(normReqHost != domain_) {
      // domain must start with '.'
      if(*domain_.begin() != '.') {
        return false;
      }
      // domain must not end with '.'
      if(*domain_.rbegin() == '.') {
        return false;
      }
      // domain must include at least one embeded '.'
      if(domain_.size() < 4 ||
         domain_.find(A2STR::DOT_C, 1) == std::string::npos) {
        return false;
      }
      if(!util::endsWith(normReqHost, domain_)) {
        return false;
      }
      // From RFC2965 3.3.2 Rejecting Cookies
      // * The request-host is a HDN (not IP address) and has the form HD,
      //   where D is the value of the Domain attribute, and H is a string
      //   that contains one or more dots.
      size_t dotCount = std::count(normReqHost.begin(),
                                   normReqHost.begin()+
                                   (normReqHost.size()-domain_.size()), '.');
      if(dotCount > 1 || (dotCount == 1 && normReqHost[0] != '.')) {
        return false;
      } 
    }
  }
  if(requestPath != path_) {
    // From RFC2965 3.3.2 Rejecting Cookies
    // * The value for the Path attribute is not a prefix of the request-URI.
    if(!pathInclude(requestPath, path_)) {
      return false;
    }
  }
  return good();
}

bool Cookie::operator==(const Cookie& cookie) const
{
  return domain_ == cookie.domain_ && path_ == cookie.path_ &&
    name_ == cookie.name_;
}

bool Cookie::isExpired() const
{
  return !expiry_ == 0 && Time().getTime() >= expiry_;
}

std::string Cookie::toNsCookieFormat() const
{
  std::stringstream ss;
  ss << domain_ << "\t";
  if(util::startsWith(domain_, A2STR::DOT_C)) {
    ss << "TRUE";
  } else {
    ss << "FALSE";
  }
  ss << "\t";
  ss << path_ << "\t";
  if(secure_) {
    ss << "TRUE";
  } else {
    ss << "FALSE";
  }
  ss << "\t";
  ss << expiry_ << "\t";
  ss << name_ << "\t";
  ss << value_;
  return ss.str();
}

void Cookie::markOriginServerOnly()
{
  if(util::startsWith(domain_, A2STR::DOT_C)) {
    domain_.erase(domain_.begin(), domain_.begin()+1);
  }
}

void Cookie::updateLastAccess()
{
  lastAccess_ = time(0);
}

} // namespace aria2
