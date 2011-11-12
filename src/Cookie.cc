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
#include "Cookie.h"

#include <sstream>

#include "A2STR.h"
#include "a2functional.h"
#include "cookie_helper.h"

namespace aria2 {

Cookie::Cookie
(const std::string& name,
 const std::string& value,
 time_t  expiryTime,
 bool persistent,
 const std::string& domain,
 bool hostOnly,
 const std::string& path,
 bool secure,
 bool httpOnly,
 time_t creationTime):
  name_(name),
  value_(value),
  expiryTime_(expiryTime),
  persistent_(persistent),
  domain_(domain),
  hostOnly_(hostOnly),
  path_(path),
  secure_(secure),
  httpOnly_(httpOnly),
  creationTime_(creationTime),
  lastAccessTime_(creationTime) {}

Cookie::Cookie():
  expiryTime_(0),
  persistent_(false),
  hostOnly_(false),
  secure_(false),
  httpOnly_(false),
  creationTime_(0),
  lastAccessTime_(0) {}

Cookie::Cookie(const Cookie& c)
  : name_(c.name_),
    value_(c.value_),
    expiryTime_(c.expiryTime_),
    persistent_(c.persistent_),
    domain_(c.domain_),
    hostOnly_(c.hostOnly_),
    path_(c.path_),
    secure_(c.secure_),
    httpOnly_(c.httpOnly_),
    creationTime_(c.creationTime_),
    lastAccessTime_(c.lastAccessTime_)
{}

Cookie::~Cookie() {}

Cookie& Cookie::operator=(const Cookie& c)
{
  if(this != &c) {
    name_ = c.name_;
    value_ = c.value_;
    expiryTime_ = c.expiryTime_;
    persistent_ = c.persistent_;
    domain_ = c.domain_;
    hostOnly_ = c.hostOnly_;
    path_ = c.path_;
    secure_ = c.secure_;
    httpOnly_ = c.httpOnly_;
    creationTime_ = c.creationTime_;
    lastAccessTime_ = c.lastAccessTime_;
  }
  return *this;
}

std::string Cookie::toString() const
{
  std::string s = name_;
  s += "=";
  s += value_;
  return s;
}

bool Cookie::match
(const std::string& requestHost,
 const std::string& requestPath,
 time_t date, bool secure) const
{
  if((secure_ && !secure) || isExpired(date) ||
     !cookie::pathMatch(requestPath, path_)) {
    return false;
  }
  if(hostOnly_) {
    return requestHost == domain_ ;
  } else {
    return cookie::domainMatch(requestHost, domain_);
  }
}

bool Cookie::operator==(const Cookie& cookie) const
{
  return domain_ == cookie.domain_ && path_ == cookie.path_ &&
    name_ == cookie.name_;
}

bool Cookie::isExpired(time_t base) const
{
  return persistent_ && base > expiryTime_;
}

std::string Cookie::toNsCookieFormat() const
{
  std::stringstream ss;
  if(!hostOnly_) {
    ss << A2STR::DOT_C;
  }
  ss << domain_ << "\t";
  if(hostOnly_) {
    ss << "FALSE";
  } else {
    ss << "TRUE";
  }
  ss << "\t";
  ss << path_ << "\t";
  if(secure_) {
    ss << "TRUE";
  } else {
    ss << "FALSE";
  }
  ss << "\t";
  if(persistent_) {
    ss << expiryTime_;
  } else {
    ss << 0;
  }
  ss << "\t";
  ss << name_ << "\t";
  ss << value_;
  return ss.str();
}

void Cookie::setName(const std::string& name)
{
  name_ = name;
}

void Cookie::setValue(const std::string& value)
{
  value_ = value;
}

void Cookie::setDomain(const std::string& domain)
{
  domain_ = domain;
}

void Cookie::setPath(const std::string& path)
{
  path_ = path;
}

} // namespace aria2
