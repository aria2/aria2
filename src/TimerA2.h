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
#ifndef D_TIMER_A2_H
#define D_TIMER_A2_H

#include "common.h"

#include <chrono>

#include "a2time.h"
#include "a2functional.h"

namespace aria2 {

class Timer {
public:
  using Clock = std::chrono::steady_clock;

  // The time value is initialized so that it represents the time at which
  // this object was created.
  Timer();
  Timer(const Timer& time) = default;
  Timer(Timer&& time) = default;

  template <typename duration>
  constexpr explicit Timer(const duration& t) : tp_(t)
  {
  }

  explicit Timer(const Clock::time_point& tp);

  Timer& operator=(Timer&& timer) = default;
  Timer& operator=(const Timer& timer) = default;

  bool operator<(const Timer& timer) const { return tp_ < timer.tp_; }
  bool operator>(const Timer& timer) const { return timer < *this; }
  bool operator<=(const Timer& timer) const { return !(timer < *this); }
  bool operator>=(const Timer& timer) const { return !(*this < timer); }

  void reset();

  template <typename duration> void reset(const duration& t)
  {
    tp_ = Clock::time_point(t);
  }

  Clock::duration difference() const;

  Clock::duration difference(const Timer& timer) const;

  // Returns true if this object's time value is zero.
  bool isZero() const;

  template <typename duration> void advance(const duration& t) { tp_ += t; }

  template <typename duration> void sub(const duration& t) { tp_ -= t; }

  const Clock::time_point& getTime() const { return tp_; }

  static Timer zero() { return Timer(0_s); }

private:
  Clock::time_point tp_;
};

} // namespace aria2

#endif // D_TIMER_A2_H
