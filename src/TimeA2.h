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
#ifndef _D_TIME_H_
#define _D_TIME_H_

#include "common.h"
#include <sys/time.h>

class Time {
private:
  struct timeval tv;

  struct timeval getCurrentTime() const;
public:
  // The time value is initialized so that it represents the time at which
  // this object was created.
  Time();
  Time(const Time& time);
  Time(int sec);

  Time& operator=(const Time& time) {
    if(this != &time) {
      tv = time.tv;
    }
    return *this;
  }

  ~Time();

  // Makes this object's time value up to date.
  void reset();

  bool elapsed(int sec) const;

  bool elapsedInMillis(int millis) const;

  int difference() const;
  long long int differenceInMillis() const;

  // Returns true if this object's time value is zero.
  bool isZero() const { return tv.tv_sec == 0 && tv.tv_usec == 0; }

  long long int getTimeInMicros() const {
    return (long long int)tv.tv_sec*1000*1000+tv.tv_usec;
  }

  long long int getTimeInMillis() const {
    return (long long int)tv.tv_sec*1000+tv.tv_usec/1000;
  }

  // Returns this object's time value in seconds.
  int getTime() const {
    return tv.tv_sec;
  }

  void setTimeInSec(int sec);

  bool isNewer(const Time& time) const;
};

#endif // _D_TIME_H_
