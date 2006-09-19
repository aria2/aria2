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
#ifndef _D_SPEED_CALC_H_
#define _D_SPEED_CALC_H_

#include "common.h"
#include "TimeA2.h"

class SpeedCalc {
private:
  long long int lengthArray[2];
  int sw;
  Time cpArray[2];
  int maxSpeed;
  int prevSpeed;

  bool isIntervalOver() const;
  void changeSw();
public:
  SpeedCalc() {
    reset();
  }

  ~SpeedCalc() {}

  /**
   * Returns download/upload speed in byte per sec
   */
  int calculateSpeed();

  int getMaxSpeed() const {
    return maxSpeed;
  }

  void update(int bytes);

  void reset();
};

#endif // _D_SPEED_CALC_H_
