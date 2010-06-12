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

SpeedCalc::SpeedCalc():_sw(0), _maxSpeed(0), _prevSpeed(0),
                       _accumulatedLength(0),
                       _nextInterval(CHANGE_INTERVAL_SEC)
{
  std::fill(&_lengthArray[0], &_lengthArray[2], 0);
}

void SpeedCalc::reset() {
  std::fill(&_lengthArray[0], &_lengthArray[2], 0);
  std::fill(&_cpArray[0], &_cpArray[2], global::wallclock);
  _sw = 0;
  _maxSpeed = 0;
  _prevSpeed = 0;
  _start = global::wallclock;
  _accumulatedLength = 0;
  _nextInterval = CHANGE_INTERVAL_SEC;
}

unsigned int SpeedCalc::calculateSpeed() {
  int64_t milliElapsed = _cpArray[_sw].differenceInMillis(global::wallclock);
  if(milliElapsed) {
    unsigned int speed = _lengthArray[_sw]*1000/milliElapsed;
    _prevSpeed = speed;
    _maxSpeed = std::max(speed, _maxSpeed);
    if(isIntervalOver(milliElapsed)) {
      changeSw();
    }
    return speed;
  } else {
    return _prevSpeed;
  }
}

void SpeedCalc::update(size_t bytes) {
  _accumulatedLength += bytes;
  std::transform(&_lengthArray[0], &_lengthArray[2], &_lengthArray[0],
                 std::bind1st(std::plus<uint64_t>(), (uint64_t)bytes));
  if(isIntervalOver()) {
    changeSw();
  }
}

bool SpeedCalc::isIntervalOver() const {
  return _nextInterval <= _cpArray[_sw].difference(global::wallclock);
}

bool SpeedCalc::isIntervalOver(int64_t milliElapsed) const
{
  return _nextInterval <= milliElapsed/1000;
}

void SpeedCalc::changeSw() {
  _lengthArray[_sw] = 0;
  _cpArray[_sw] = global::wallclock;
  _sw ^= 0x01;
  _nextInterval =
    _cpArray[_sw].difference(global::wallclock)+CHANGE_INTERVAL_SEC;
}

unsigned int SpeedCalc::calculateAvgSpeed() const {
  uint64_t milliElapsed = _start.differenceInMillis(global::wallclock);

  // if milliElapsed is too small, the average speed is rubish, better return 0
  if(milliElapsed > 4) {
    unsigned int speed = _accumulatedLength*1000/milliElapsed;
    return speed;
  } else {
    return 0;
  }
}

} // namespace aria2
