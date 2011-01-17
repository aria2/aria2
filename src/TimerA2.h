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
#ifndef D_TIMER_A2_H
#define D_TIMER_A2_H

#include "common.h"
#include "a2time.h"

namespace aria2 {

class Timer {
private:
  timeval tv_;

  time_t difference(const struct timeval& tv) const;

  int64_t differenceInMillis(const struct timeval& tv) const;
public:
  // The time value is initialized so that it represents the time at which
  // this object was created.
  Timer();
  Timer(const Timer& time);
  Timer(time_t sec);
  Timer(const struct timeval& tv);

  Timer& operator=(const Timer& timer);

  bool operator<(const Timer& timer) const;

  bool operator<=(const Timer& timer) const;

  bool operator>(const Timer& timer) const;

  void reset();

  void reset(time_t sec);

  bool elapsed(time_t sec) const;

  bool elapsedInMillis(int64_t millis) const;

  time_t difference() const;

  time_t difference(const Timer& timer) const
  {
    return difference(timer.tv_);
  }

  int64_t differenceInMillis() const;

  int64_t differenceInMillis(const Timer& timer) const
  {
    return differenceInMillis(timer.tv_);
  }

  // Returns true if this object's time value is zero.
  bool isZero() const;

  void advance(time_t sec);

  // Returns this object's time value in seconds.
  time_t getTime() const;

  int64_t getTimeInMicros() const;

  int64_t getTimeInMillis() const;

  // Returns true if this Timer is not affected by system time change.
  // Otherwise return false.
  static bool monotonicClock();
};

} // namespace aria2

#endif // D_TIMER_A2_H
