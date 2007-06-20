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
#ifndef _D_SPEED_CALC_H_
#define _D_SPEED_CALC_H_

#include "common.h"
#include "TimeA2.h"

class SpeedCalc {
private:
  int64_t lengthArray[2];
  int32_t sw;
  Time cpArray[2];
  int32_t maxSpeed;
  int32_t prevSpeed;
  Time start;
  int64_t accumulatedLength;
  int32_t nextInterval;

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
  int32_t calculateSpeed();

  int32_t calculateSpeed(const struct timeval& now);

  int32_t getMaxSpeed() const {
    return maxSpeed;
  }

  int32_t getAvgSpeed() const;

  void update(int bytes);

  void reset();
};

#endif // _D_SPEED_CALC_H_
