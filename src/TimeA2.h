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
#ifndef D_TIME_H
#define D_TIME_H

#include "common.h"

#include <stdint.h>

#include <string>

#include "a2time.h"

namespace aria2 {

class Time {
private:
  struct timeval tv_;

  bool good_;

  struct timeval getCurrentTime() const;
public:
  // The time value is initialized so that it represents the time at which
  // this object was created.
  Time();
  Time(const Time& time);
  Time(time_t sec);
  Time(const struct timeval& tv);

  ~Time();

  Time& operator=(const Time& time);

  bool operator<(const Time& time) const;

  // Makes this object's time value up to date.
  void reset();

  bool elapsed(time_t sec) const;

  bool elapsedInMillis(int64_t millis) const;

  time_t difference() const;

  time_t difference(const struct timeval& now) const;

  time_t difference(const Time& now) const
  {
    return difference(now.tv_);
  }

  int64_t differenceInMillis() const;

  int64_t differenceInMillis(const struct timeval& now) const;

  int64_t differenceInMillis(const Time& now) const
  {
    return differenceInMillis(now.tv_);
  }

  // Returns true if this object's time value is zero.
  bool isZero() const;

  int64_t getTimeInMicros() const;

  int64_t getTimeInMillis() const;

  // Returns this object's time value in seconds.
  time_t getTime() const;

  void setTimeInSec(time_t sec);

  bool isNewer(const Time& time) const;

  void advance(time_t sec);

  bool good() const;

  // Returns !good()
  bool bad() const;

  std::string toHTTPDate() const;

  // Currently timezone is assumed as GMT.
  static Time parse(const std::string& datetime, const std::string& format);

  // Currently timezone is assumed to GMT.
  static Time parseRFC1123(const std::string& datetime);

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

  static Time null();
};

} // namespace aria2

#endif // D_TIME_H
