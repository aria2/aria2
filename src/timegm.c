/*
 * aria2 - The high speed download utility
 *
 * Copyright (C) 2012 Tatsuhiro Tsujikawa
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
#include "timegm.h"

#include <stdint.h>

/* Counter the number of leap year in the range [0, y). The |y| is the
   year, including century (e.g., 2012) */
static int count_leap_year(int y)
{
  y -= 1;
  return y / 4 - y / 100 + y / 400;
}

/* Returns nonzero if the |y| is the leap year. The |y| is the year,
   including century (e.g., 2012) */
static int is_leap_year(int y)
{
  return y % 4 == 0 && (y % 100 != 0 || y % 400 == 0);
}

/* The number of days before ith month begins */
static int daysum[] = {0, 31, 59, 90, 120, 151, 181, 212, 243, 273, 304, 334};

// Based on the algorithm of Python 2.7 calendar.timegm.
time_t timegm(struct tm* tm)
{
  int days;
  int num_leap_year;
  int64_t t;
  if (tm->tm_mon > 11) {
    return -1;
  }
  num_leap_year = count_leap_year(tm->tm_year + 1900) - count_leap_year(1970);
  days = (tm->tm_year - 70) * 365 + num_leap_year + daysum[tm->tm_mon] +
         tm->tm_mday - 1;
  if (tm->tm_mon >= 2 && is_leap_year(tm->tm_year + 1900)) {
    ++days;
  }
  t = ((int64_t)days * 24 + tm->tm_hour) * 3600 + tm->tm_min * 60 + tm->tm_sec;
  if (sizeof(time_t) == 4) {
    if (t < INT32_MIN || t > INT32_MAX) {
      return -1;
    }
  }
  return t;
}
