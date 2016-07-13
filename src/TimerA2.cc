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

#include "TimerA2.h"

namespace aria2 {

// Add this offset to Timer::Clock::now() so that we can treat 0 value
// as special case, and normal timeout always applies.
constexpr auto OFFSET = 24_h;

namespace {
Timer::Clock::time_point getNow() { return Timer::Clock::now() + OFFSET; }
} // namespace

Timer::Timer() : tp_(getNow()) { reset(); }

Timer::Timer(const Clock::time_point& tp) : tp_(tp) {}

void Timer::reset() { tp_ = getNow(); }

Timer::Clock::duration Timer::difference() const
{
  auto now = getNow();
  if (now < tp_) {
    return Timer::Clock::duration(0_s);
  }

  return now - tp_;
}

Timer::Clock::duration Timer::difference(const Timer& timer) const
{
  if (timer.tp_ < tp_) {
    return Timer::Clock::duration(0_s);
  }

  return timer.tp_ - tp_;
}

bool Timer::isZero() const
{
  return tp_.time_since_epoch() == Clock::duration::zero();
}

} // namespace aria2
