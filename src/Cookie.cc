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
#include "Cookie.h"
#include "Util.h"
#include "A2STR.h"
#include "TimeA2.h"
#include <algorithm>

namespace aria2 {

Cookie::Cookie(const std::string& name,
	       const std::string& value,
	       time_t  expires,
	       const std::string& path,
	       const std::string& domain,
	       bool secure):
  name(name),
  value(value),
  expires(expires),
  path(path),
  domain(Util::toLower(domain)),
  secure(secure),
  onetime(false) {}

Cookie::Cookie(const std::string& name,
	       const std::string& value,
	       const std::string& path,
	       const std::string& domain,
	       bool secure):
  name(name),
  value(value),
  path(path),
  domain(Util::toLower(domain)),
  secure(secure),
  onetime(true) {}

Cookie::Cookie():expires(0), secure(false), onetime(true) {}

Cookie::~Cookie() {}

std::string Cookie::toString() const
{
  return name+"="+value;
}

void Cookie::clear()
{
  name = value = path = domain = A2STR::NIL;
  expires = 0;
  secure = false;
}

bool Cookie::good() const
{
  return !name.empty();
}

static bool pathInclude(const std::string& requestPath, const std::string& path)
{
  if(requestPath == path) {
    return true;
  }
  if(Util::startsWith(requestPath, path)) {
    if(*path.rbegin() != '/' && requestPath[path.size()] != '/') {
      return false;
    }
  } else if(*path.rbegin() != '/' || *requestPath.rbegin() == '/' ||
	    !Util::startsWith(requestPath+"/", path)) {
    return false;
  }
  return true;
}

static bool domainMatch(const std::string& requestHost,
			const std::string& domain)
{
  if(*domain.begin() == '.') {
    return Util::endsWith("."+requestHost, domain);
  } else {
    return requestHost == domain;
  }
}

bool Cookie::match(const std::string& requestHost,
		   const std::string& requestPath,
		   time_t date, bool secure) const
{
  std::string lowerRequestHost = Util::toLower(requestHost);
  if((secure || (!this->secure && !secure)) &&
     domainMatch(lowerRequestHost, this->domain) &&
     pathInclude(requestPath, path) &&
     (this->onetime || (date < this->expires))) {
    return true;
  } else {
    return false;
  }
}

bool Cookie::validate(const std::string& requestHost,
		      const std::string& requestPath) const
{
  std::string lowerRequestHost = Util::toLower(requestHost);
  if(lowerRequestHost != domain) {
    // domain must start with '.'
    if(*domain.begin() != '.') {
      return false;
    }
    // domain must not end with '.'
    if(*domain.rbegin() == '.') {
      return false;
    }
    // domain must include at least one embeded '.'
    if(domain.size() < 4 || domain.find(".", 1) == std::string::npos) {
      return false;
    }
    if(!Util::endsWith(lowerRequestHost, domain)) {
      return false;
    }
    // From RFC2109
    // * The request-host is a FQDN (not IP address) and has the form HD,
    //   where D is the value of the Domain attribute, and H is a string
    //   that contains one or more dots.
    if(std::count(lowerRequestHost.begin(),
		  lowerRequestHost.begin()+
		  (lowerRequestHost.size()-domain.size()), '.')
       > 0) {
      return false;
    } 
  }
  if(requestPath != path) {
    // From RFC2109
    // * The value for the Path attribute is not a prefix of the request-
    //   URI.
    if(!pathInclude(requestPath, path)) {
      return false;
    }
  }
  return !name.empty();
}

bool Cookie::operator==(const Cookie& cookie) const
{
  return domain == cookie.domain && path == cookie.path && name == cookie.name;
}

bool Cookie::isExpired() const
{
  return !onetime && Time().getTime() >= expires;
}

} // namespace aria2
