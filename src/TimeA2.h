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
#ifndef D_TIME_H
#define D_TIME_H

#include "common.h"

#include <stdint.h>

#include <string>

#include "a2time.h"

namespace aria2 {

class Time {
public:
  using Clock = std::chrono::system_clock;

  // The time value is initialized so that it represents the time at which
  // this object was created.
  Time();
  Time(const Time& time) = default;
  Time(Time&& time) = default;
  Time(time_t sec);

  Time& operator=(const Time& time) = default;
  Time& operator=(Time&& time) = default;

  bool operator<(const Time& time) const { return tp_ < time.tp_; }
  bool operator>(const Time& time) const { return time < *this; }
  bool operator<=(const Time& time) const { return !(time < *this); }
  bool operator>=(const Time& time) const { return !(*this < time); }

  // Makes this object's time value up to date.
  void reset();

  Clock::duration difference() const;
  Clock::duration difference(const Time& now) const;

  const Clock::time_point& getTime() const { return tp_; }

  void setTimeFromEpoch(time_t sec);
  time_t getTimeFromEpoch() const { return Clock::to_time_t(tp_); }

  template <typename duration> void advance(const duration& t) { tp_ += t; }

  bool good() const { return good_; }
  bool bad() const { return !good_; }

  static Time null() { return Time(0, false); }

  std::string toHTTPDate() const;

  // Currently timezone is assumed as GMT.
  static Time parse(const std::string& datetime, const std::string& format);

  // Currently timezone is assumed to GMT.
  static Time parseRFC1123(const std::string& datetime);

  // Like parseRFC1123, but only accepts trailing "+0000" instead of
  // last 3 letters "GMT".
  static Time parseRFC1123Alt(const std::string& datetime);

  // Currently timezone is assumed to GMT.
  static Time parseRFC850(const std::string& datetime);

  // Currently timezone is assumed to GMT.  Basically the format is
  // RFC850, but year part is 4digit, eg 2008 This format appears in
  // original Netscape's PERSISTENT CLIENT STATE HTTP COOKIES
  // Specification. http://curl.haxx.se/rfc/cookie_spec.html
  static Time parseRFC850Ext(const std::string& datetime);

  // Currently timezone is assumed to GMT.
  // ANSI C's asctime() format
  static Time parseAsctime(const std::string& datetime);

  // Try parseRFC1123, parseRFC850, parseAsctime, parseRFC850Ext in
  // that order and returns the first "good" Time object returned by
  // these functions.
  static Time parseHTTPDate(const std::string& datetime);

private:
  Time(time_t t, bool good) : tp_(Clock::from_time_t(t)), good_(good) {}

  Clock::time_point tp_;
  bool good_;
};

} // namespace aria2

#endif // D_TIME_H
