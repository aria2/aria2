/* <!-- copyright */
/*
 * aria2 - a simple utility for downloading files faster
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
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
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
  for_each(&lengthArray[0], &lengthArray[2], Plus(bytes));
  if(isIntervalOver()) {
    changeSw();
  }
}

bool SpeedCalc::isIntervalOver() const {
  return CHANGE_INTERVAL_SEC <= cpArray[sw].difference();
}

void SpeedCalc::changeSw() {
  lengthArray[sw] = 0;
  cpArray[sw].reset();
  sw ^= 0x01;
}
