/*
 * aria2 - The high speed download utility
 *
 * Copyright (C) 2010 Tatsuhiro Tsujikawa
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
#include "clock_gettime_osx.h"

#include <mach/mach.h>
#include <mach/mach_time.h>

int clock_gettime(int dummyid, struct timespec* tp)
{
  static uint64_t lasttime = mach_absolute_time();
  static struct timespec monotime = {2678400, 0}; // 1month offset(24*3600*31)
  uint64_t now = mach_absolute_time();
  static mach_timebase_info_data_t baseinfo;
  if(baseinfo.denom == 0) {
    mach_timebase_info(&baseinfo);
  }
  uint64_t elapsed = (now-lasttime)*baseinfo.numer/baseinfo.denom;
  monotime.tv_sec += elapsed/1000000000;
  monotime.tv_nsec += elapsed%1000000000;
  if(monotime.tv_nsec >= 1000000000) {
    monotime.tv_sec += monotime.tv_nsec/1000000000;
    monotime.tv_nsec %= 1000000000;
  }
  lasttime = now;
  *tp = monotime;
  return 0;
}
