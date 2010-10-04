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

#include "TimeA2.h"

#include <cstring>

#include "util.h"
#include "array_fun.h"

namespace aria2 {

Time::Time():good_(true)
{
  reset();
}

Time::Time(const Time& time)
{
  tv_ = time.tv_;
  good_ = time.good_;
}

Time::Time(time_t sec):good_(true)
{
  setTimeInSec(sec);
}

Time::Time(const struct timeval& tv):good_(true)
{
  tv_ = tv;
}

Time::~Time() {}

Time& Time::operator=(const Time& time)
{
  if(this != &time) {
    tv_ = time.tv_;
    good_ = time.good_;
  }
  return *this;
}

bool Time::operator<(const Time& time) const
{
  return util::difftv(time.tv_, tv_) > 0;
}

void Time::reset() {
  gettimeofday(&tv_, 0);
}

struct timeval Time::getCurrentTime() const {
  struct timeval now;
  gettimeofday(&now, 0);
  return now;
}

bool Time::elapsed(time_t sec) const {
  // Because of gettimeofday called from getCurrentTime() is slow, and most of
  // the time this function is called before specified time passes, we first do
  // simple test using time.
  // Then only when the further test is required, call gettimeofday.
  time_t now = time(0);
  if(tv_.tv_sec+sec < now) {
    return true;
  } else if(tv_.tv_sec+sec == now) {
    return
      util::difftv(getCurrentTime(), tv_) >= static_cast<int64_t>(sec)*1000000;
  } else {
    return false;
  }
}

bool Time::elapsedInMillis(int64_t millis) const {
  return util::difftv(getCurrentTime(), tv_)/1000 >= millis;
}

bool Time::isNewer(const Time& time) const {
  return util::difftv(tv_, time.tv_) > 0;
}

time_t Time::difference() const
{
  return util::difftv(getCurrentTime(), tv_)/1000000;
}

time_t Time::difference(const struct timeval& now) const
{
  return util::difftv(now, tv_)/1000000;
}

int64_t Time::differenceInMillis() const {
  return util::difftv(getCurrentTime(), tv_)/1000;
}

int64_t Time::differenceInMillis(const struct timeval& now) const
{
  return util::difftv(now, tv_)/1000;
}

bool Time::isZero() const
{
  return tv_.tv_sec == 0 && tv_.tv_usec == 0;
}

int64_t Time::getTimeInMicros() const
{
  return (int64_t)tv_.tv_sec*1000*1000+tv_.tv_usec;
}

int64_t Time::getTimeInMillis() const
{
  return (int64_t)tv_.tv_sec*1000+tv_.tv_usec/1000;
}

time_t Time::getTime() const
{
  return tv_.tv_sec;
}

void Time::setTimeInSec(time_t sec) {
  tv_.tv_sec = sec;
  tv_.tv_usec = 0;
}

void Time::advance(time_t sec)
{
  tv_.tv_sec += sec;
}

bool Time::good() const
{
  return good_;
}

bool Time::bad() const
{
  return !good_;
}

std::string Time::toHTTPDate() const
{
  char buf[32];
  time_t t = getTime();
  struct tm* tms = gmtime(&t); // returned struct is statically allocated.
  size_t r = strftime(buf, sizeof(buf), "%a, %d %b %Y %H:%M:%S GMT", tms);
  return std::string(&buf[0], &buf[r]);
}

Time Time::parse(const std::string& datetime, const std::string& format)
{
  struct tm tm;
  memset(&tm, 0, sizeof(tm));
  char* r = strptime(datetime.c_str(), format.c_str(), &tm);
  if(r != datetime.c_str()+datetime.size()) {
    return Time::null();
  }
  time_t thetime = timegm(&tm);
  if(thetime == -1) {
    if(tm.tm_year >= 2038-1900) {
      thetime = INT32_MAX;
    }
  }
  return Time(thetime);  
}

Time Time::parseRFC1123(const std::string& datetime)
{
  return parse(datetime, "%a, %d %b %Y %H:%M:%S GMT");
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
    &parseRFC1123,
    &parseRFC850,
    &parseAsctime,
    &parseRFC850Ext,
  };
  for(Time (**funcsp)(const std::string&) = &funcs[0];
      funcsp != vend(funcs); ++funcsp) {
    Time t = (*funcsp)(datetime);
    if(t.good()) {
      return t;
    }
  }
  return Time::null();
}

Time Time::null()
{
  Time t(0);
  t.good_ = false;
  return t;
}

} // namespace aria2
