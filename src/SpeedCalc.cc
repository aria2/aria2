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

#define CHANGE_INTERVAL_SEC 15

SpeedCalc::SpeedCalc():sw_(0), maxSpeed_(0), prevSpeed_(0),
                       accumulatedLength_(0),
                       nextInterval_(CHANGE_INTERVAL_SEC)
{
  std::fill(&lengthArray_[0], &lengthArray_[2], 0);
}

void SpeedCalc::reset() {
  std::fill(&lengthArray_[0], &lengthArray_[2], 0);
  std::fill(&cpArray_[0], &cpArray_[2], global::wallclock());
  sw_ = 0;
  maxSpeed_ = 0;
  prevSpeed_ = 0;
  start_ = global::wallclock();
  accumulatedLength_ = 0;
  nextInterval_ = CHANGE_INTERVAL_SEC;
}

int SpeedCalc::calculateSpeed() {
  int64_t milliElapsed = cpArray_[sw_].differenceInMillis(global::wallclock());
  if(milliElapsed) {
    int speed = lengthArray_[sw_]*1000/milliElapsed;
    prevSpeed_ = speed;
    maxSpeed_ = std::max(speed, maxSpeed_);
    if(isIntervalOver(milliElapsed)) {
      changeSw();
    }
    return speed;
  } else {
    return prevSpeed_;
  }
}

void SpeedCalc::update(size_t bytes) {
  accumulatedLength_ += bytes;
  std::transform(&lengthArray_[0], &lengthArray_[2], &lengthArray_[0],
                 std::bind1st(std::plus<int64_t>(), (int64_t)bytes));
  if(isIntervalOver()) {
    changeSw();
  }
}

bool SpeedCalc::isIntervalOver() const {
  return nextInterval_ <= cpArray_[sw_].difference(global::wallclock());
}

bool SpeedCalc::isIntervalOver(int64_t milliElapsed) const
{
  return nextInterval_ <= milliElapsed/1000;
}

void SpeedCalc::changeSw() {
  lengthArray_[sw_] = 0;
  cpArray_[sw_] = global::wallclock();
  sw_ ^= 0x01u;
  nextInterval_ =
    cpArray_[sw_].difference(global::wallclock())+CHANGE_INTERVAL_SEC;
}

int SpeedCalc::calculateAvgSpeed() const {
  int64_t milliElapsed = start_.differenceInMillis(global::wallclock());

  // if milliElapsed is too small, the average speed is rubish, better return 0
  if(milliElapsed > 4) {
    int speed = accumulatedLength_*1000/milliElapsed;
    return speed;
  } else {
    return 0;
  }
}

} // namespace aria2
