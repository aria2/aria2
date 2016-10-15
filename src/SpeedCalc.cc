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

namespace {
constexpr auto WINDOW_TIME = 10_s;
} // namespace

SpeedCalc::SpeedCalc() : accumulatedLength_(0), bytesWindow_(0), maxSpeed_(0) {}

void SpeedCalc::reset()
{
  timeSlots_.clear();
  start_ = global::wallclock();
  accumulatedLength_ = 0;
  bytesWindow_ = 0;
  maxSpeed_ = 0;
}

void SpeedCalc::removeStaleTimeSlot(const Timer& now)
{
  while (!timeSlots_.empty()) {
    if (timeSlots_[0].first.difference(now) <= WINDOW_TIME) {
      break;
    }
    bytesWindow_ -= timeSlots_[0].second;
    timeSlots_.pop_front();
  }
}

int SpeedCalc::calculateSpeed()
{
  const auto& now = global::wallclock();
  removeStaleTimeSlot(now);
  if (timeSlots_.empty()) {
    return 0;
  }
  auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(
                     timeSlots_[0].first.difference(now))
                     .count();
  if (elapsed <= 0) {
    elapsed = 1;
  }
  int speed = bytesWindow_ * 1000 / elapsed;
  maxSpeed_ = std::max(speed, maxSpeed_);
  return speed;
}

int SpeedCalc::calculateNewestSpeed(int seconds)
{
  const auto& now = global::wallclock();
  removeStaleTimeSlot(now);

  int64_t bytesCount(0);
  auto it = timeSlots_.rbegin();
  while (it != timeSlots_.rend()) {
    if (it->first.difference(now) > seconds * 1_s) {
      break;
    }
    bytesCount += (*it++).second;
  }
  if (it == timeSlots_.rbegin()) {
    return 0;
  }

  auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(
                     (*--it).first.difference(now))
                     .count();
  if (elapsed <= 0) {
    elapsed = 1;
  }
  return bytesCount * (1000. / elapsed);
}

void SpeedCalc::update(size_t bytes)
{
  const auto& now = global::wallclock();
  removeStaleTimeSlot(now);
  if (timeSlots_.empty() ||
      std::chrono::duration_cast<std::chrono::seconds>(
          timeSlots_.back().first.difference(now)) >= 1_s) {
    timeSlots_.push_back(std::make_pair(now, bytes));
  }
  else {
    timeSlots_.back().second += bytes;
  }
  bytesWindow_ += bytes;
  accumulatedLength_ += bytes;
}

int SpeedCalc::calculateAvgSpeed() const
{
  auto milliElapsed = std::chrono::duration_cast<std::chrono::milliseconds>(
                          start_.difference(global::wallclock()))
                          .count();
  // if milliElapsed is too small, the average speed is rubbish, better
  // return 0
  if (milliElapsed > 4) {
    int speed = accumulatedLength_ * 1000 / milliElapsed;
    return speed;
  }
  else {
    return 0;
  }
}

} // namespace aria2
