/* <!-- copyright */
/*
 * aria2 - The high speed download utility
 *
 * Copyright (C) 2015 Tatsuhiro Tsujikawa
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

#include "TimeA2.h"

#include <cstring>

#include "util.h"
#include "array_fun.h"

namespace aria2 {

Time::Time() : tp_(Clock::now()), good_(true) {}

Time::Time(time_t t) : tp_(Clock::from_time_t(t)), good_(true) {}

void Time::reset()
{
  tp_ = Clock::now();
  good_ = true;
}

Time::Clock::duration Time::difference() const { return Clock::now() - tp_; }

Time::Clock::duration Time::difference(const Time& time) const
{
  return time.tp_ - tp_;
}

void Time::setTimeFromEpoch(time_t t)
{
  tp_ = Clock::from_time_t(t);
  good_ = true;
}

std::string Time::toHTTPDate() const
{
  char buf[32];
  time_t t = getTimeFromEpoch();
  struct tm* tms = gmtime(&t); // returned struct is statically allocated.
  size_t r = strftime(buf, sizeof(buf), "%a, %d %b %Y %H:%M:%S GMT", tms);
  return std::string(&buf[0], &buf[r]);
}

Time Time::parse(const std::string& datetime, const std::string& format)
{
  struct tm tm;
  memset(&tm, 0, sizeof(tm));
  char* r = strptime(datetime.c_str(), format.c_str(), &tm);
  if (r != datetime.c_str() + datetime.size()) {
    return Time::null();
  }
  time_t thetime = timegm(&tm);
  if (thetime == -1) {
    if (tm.tm_year >= 2038 - 1900) {
      thetime = INT32_MAX;
    }
  }
  return Time(thetime);
}

Time Time::parseRFC1123(const std::string& datetime)
{
  return parse(datetime, "%a, %d %b %Y %H:%M:%S GMT");
}

Time Time::parseRFC1123Alt(const std::string& datetime)
{
  return parse(datetime, "%a, %d %b %Y %H:%M:%S +0000");
}

Time Time::parseRFC850(const std::string& datetime)
{
  return parse(datetime, "%a, %d-%b-%y %H:%M:%S GMT");
}

Time Time::parseRFC850Ext(const std::string& datetime)
{
  return parse(datetime, "%a, %d-%b-%Y %H:%M:%S GMT");
}

Time Time::parseAsctime(const std::string& datetime)
{
  return parse(datetime, "%a %b %d %H:%M:%S %Y");
}

Time Time::parseHTTPDate(const std::string& datetime)
{
  Time (*funcs[])(const std::string&) = {
      &parseRFC1123, &parseRFC1123Alt, &parseRFC850,
      &parseAsctime, &parseRFC850Ext,
  };
  for (auto func : funcs) {
    Time t = func(datetime);
    if (t.good()) {
      return t;
    }
  }
  return Time::null();
}

} // namespace aria2
