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
#include "SpeedCalc.h"

#include <algorithm>
#include <functional>

#include "wallclock.h"

namespace aria2 {

#define WINDOW_MSEC 15000

SpeedCalc::SpeedCalc()
  : accumulatedLength_(0),
    bytesWindow_(0),
    maxSpeed_(0)
{}

void SpeedCalc::reset()
{
  timeSlots_.clear();
  start_ = global::wallclock();
  accumulatedLength_ = 0;
  bytesWindow_ = 0;
  maxSpeed_ = 0;
}

void SpeedCalc::removeStaleTimeSlot(int64_t now)
{
  while(!timeSlots_.empty()) {
    if(now - timeSlots_[0].first <= WINDOW_MSEC) {
      break;
    } else {
      bytesWindow_ -= timeSlots_[0].second;
      timeSlots_.pop_front();
    }
  }
}

int SpeedCalc::calculateSpeed()
{
  int64_t now = global::wallclock().getTimeInMillis();
  removeStaleTimeSlot(now);
  if(timeSlots_.empty()) {
    return 0;
  }
  int64_t elapsed = now - timeSlots_[0].first;
  if(elapsed <= 0) {
    elapsed = 1;
  }
  int speed = bytesWindow_*1000/elapsed;
  maxSpeed_ = std::max(speed, maxSpeed_);
  return speed;
}

void SpeedCalc::update(size_t bytes)
{
  int64_t now = global::wallclock().getTimeInMillis();
  removeStaleTimeSlot(now);
  if(timeSlots_.empty() || now/1000 != timeSlots_.back().first/1000) {
    timeSlots_.push_back(std::make_pair(now, bytes));
  } else {
    timeSlots_.back().second += bytes;
  }
  bytesWindow_ += bytes;
  accumulatedLength_ += bytes;
}

int SpeedCalc::calculateAvgSpeed() const
{
  int64_t milliElapsed = start_.differenceInMillis(global::wallclock());
  // if milliElapsed is too small, the average speed is rubish, better
  // return 0
  if(milliElapsed > 4) {
    int speed = accumulatedLength_*1000/milliElapsed;
    return speed;
  } else {
    return 0;
  }
}

} // namespace aria2
