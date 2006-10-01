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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
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
#include <ostream>
#include <iterator>

#define CHANGE_INTERVAL_SEC 15

class Reset {
public:
  void operator()(Time& tm) {
    tm.reset();
  }
};

void SpeedCalc::reset() {
  fill(&lengthArray[0], &lengthArray[2], 0);
  for_each(&cpArray[0], &cpArray[2], Reset());
  sw = 0;
  maxSpeed = 0;
  prevSpeed = 0;
  start.reset();
  accumulatedLength = 0;
  nextInterval = CHANGE_INTERVAL_SEC;
}

int SpeedCalc::calculateSpeed() {
  int milliElapsed = cpArray[sw].differenceInMillis();
  if(milliElapsed) {
    int speed = lengthArray[sw]*1000/milliElapsed;
    prevSpeed = speed;
    maxSpeed = max<int>(speed, maxSpeed);
    return speed;
  } else {
    return prevSpeed;
  }
}

class Plus {
private:
  int d;
public:
  Plus(int d):d(d) {}

  void operator()(long long int& length) {
    length += d;
  }
};

void SpeedCalc::update(int bytes) {
  accumulatedLength += bytes;
  for_each(&lengthArray[0], &lengthArray[2], Plus(bytes));
  if(isIntervalOver()) {
    changeSw();
  }
}

bool SpeedCalc::isIntervalOver() const {
  return nextInterval <= cpArray[sw].difference();
}

void SpeedCalc::changeSw() {
  lengthArray[sw] = 0;
  cpArray[sw].reset();
  sw ^= 0x01;
  nextInterval = cpArray[sw].difference()+CHANGE_INTERVAL_SEC;
}

int SpeedCalc::getAvgSpeed() const {
  int milliElapsed = start.differenceInMillis();
  if(milliElapsed) {
    int speed = accumulatedLength*1000/milliElapsed;
    return speed;
  } else {
    return 0;
  }
}
