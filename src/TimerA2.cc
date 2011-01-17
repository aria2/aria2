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

#include "TimerA2.h"

#include <cassert>

#include "util.h"

namespace aria2 {

Timer::Timer()
{
  reset();
}

Timer::Timer(const Timer& timer):tv_(timer.tv_) {}

Timer::Timer(time_t sec)
{
  reset(sec);
}

Timer::Timer(const struct timeval& tv):tv_(tv) {}

static bool useClockGettime()
{
  static timespec ts;
  static int r = clock_gettime(CLOCK_MONOTONIC, &ts);
  return r == 0;
}

Timer& Timer::operator=(const Timer& timer)
{
  if(this != &timer) {
    tv_ = timer.tv_;
  }
  return *this;
}

bool Timer::operator<(const Timer& timer) const
{
  return util::difftv(timer.tv_, tv_) > 0;
}

bool Timer::operator<=(const Timer& timer) const
{
  return util::difftv(timer.tv_, tv_) >= 0;
}

bool Timer::operator>(const Timer& timer) const
{
  return util::difftv(tv_, timer.tv_) > 0;
}

static timeval getCurrentTime()
{
  timeval tv;
  if(useClockGettime()) {
    timespec ts;
    int r = clock_gettime(CLOCK_MONOTONIC, &ts);
    assert(r == 0);
    tv.tv_sec = ts.tv_sec+2678400; // 1month offset(24*3600*31)
    tv.tv_usec = ts.tv_nsec/1000;
  } else {
    gettimeofday(&tv, 0);
  }
  return tv;
}

void Timer::reset()
{
  tv_ = getCurrentTime();
}

void Timer::reset(time_t sec)
{
  tv_.tv_sec = sec;
  tv_.tv_usec = 0;
}

bool Timer::elapsed(time_t sec) const
{
  return
    util::difftv(getCurrentTime(), tv_) >= static_cast<int64_t>(sec)*1000000;
}

bool Timer::elapsedInMillis(int64_t millis) const
{
  return util::difftv(getCurrentTime(), tv_)/1000 >= millis;
}

time_t Timer::difference() const
{
  return util::difftv(getCurrentTime(), tv_)/1000000;
}

time_t Timer::difference(const timeval& tv) const
{
  return util::difftv(tv, tv_)/1000000;
}

int64_t Timer::differenceInMillis() const
{
  return util::difftv(getCurrentTime(), tv_)/1000;
}

int64_t Timer::differenceInMillis(const timeval& tv) const
{
  return util::difftv(tv, tv_)/1000;
}

bool Timer::isZero() const
{
  return tv_.tv_sec == 0 && tv_.tv_usec == 0;
}

int64_t Timer::getTimeInMicros() const
{
  return (int64_t)tv_.tv_sec*1000*1000+tv_.tv_usec;
}

int64_t Timer::getTimeInMillis() const
{
  return (int64_t)tv_.tv_sec*1000+tv_.tv_usec/1000;
}

time_t Timer::getTime() const
{
  return tv_.tv_sec;
}

void Timer::advance(time_t sec)
{
  tv_.tv_sec += sec;
}

bool Timer::monotonicClock()
{
  return useClockGettime();
}

} // namespace aria2
