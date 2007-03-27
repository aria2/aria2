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
#include "TimeA2.h"
#include "Util.h"

Time::Time() {
  reset();
}

Time::Time(const Time& time) {
  tv = time.tv;
}

Time::Time(int sec) {
  setTimeInSec(sec);
}

Time::~Time() {}

void Time::reset() {
  gettimeofday(&tv, 0);
}

struct timeval Time::getCurrentTime() const {
  struct timeval now;
  gettimeofday(&now, 0);
  return now;
}

bool Time::elapsed(int sec) const {
  return Util::difftvsec(getCurrentTime(), tv) >= sec;
}

bool Time::elapsedInMillis(int millis) const {
  return Util::difftv(getCurrentTime(), tv)/1000 >= millis;
}

bool Time::isNewer(const Time& time) const {
  return Util::difftv(this->tv, time.tv) > 0;
}

int Time::difference() const {
  return Util::difftvsec(getCurrentTime(), tv);
}

long long int Time::differenceInMillis() const {
  return Util::difftv(getCurrentTime(), tv)/1000;
}

void Time::setTimeInSec(int sec) {
  tv.tv_sec = sec;
  tv.tv_usec = 0;
}
