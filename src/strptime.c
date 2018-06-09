/*
 * Copyright (c) 1999 Kungliga Tekniska Högskolan
 * (Royal Institute of Technology, Stockholm, Sweden).
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * 3. Neither the name of KTH nor the names of its contributors may be
 *    used to endorse or promote products derived from this software without
 *    specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY KTH AND ITS CONTRIBUTORS ``AS IS'' AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL KTH OR ITS CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR
 * BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
 * ADVISED OF THE POSSIBILITY OF SUCH DAMAGE. */

#ifndef HAVE_CONFIG_H
#  include "config.h"
#endif // HAVE_CONFIG_H

#ifndef HAVE_LOCALTIME_R
#  include "localtime_r.h"
#endif // HAVE_LOCALTIME_R

#ifndef HAVE_TIMEGM
#  include "timegm.h"
#endif // HAVE_TIMEGM

#include <stddef.h>
#include <stdio.h>
#include <time.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>

#ifdef HAVE_ALLOCA_H
#  include <alloca.h>
#endif // HAVE_ALLOCA_H

#ifdef HAVE_MALLOC_H
#  include <malloc.h>
#endif // HAVE_MALLOC_H

#include "strptime.h"

static const char* abb_weekdays[] = {"Sun", "Mon", "Tue", "Wed",
                                     "Thu", "Fri", "Sat", NULL};

static const char* full_weekdays[] = {"Sunday",    "Monday",   "Tuesday",
                                      "Wednesday", "Thursday", "Friday",
                                      "Saturday",  NULL};

static const char* abb_month[] = {"Jan", "Feb", "Mar", "Apr", "May",
                                  "Jun", "Jul", "Aug", "Sep", "Oct",
                                  "Nov", "Dec", NULL};

static const char* full_month[] = {
    "January", "February",  "March",   "April",    "May",      "June", "July",
    "August",  "September", "October", "November", "December", NULL,
};

static const char* ampm[] = {"am", "pm", NULL};

/*
 * tm_year is relative this year
 */
const int tm_year_base = 1900;

/*
 * Return TRUE iff `year' was a leap year.
 * Needed for strptime.
 */
static int is_leap_year(int year)
{
  return (year % 4) == 0 && ((year % 100) != 0 || (year % 400) == 0);
}

/* Needed for strptime. */
static int match_string(const char** buf, const char** strs)
{
  int i = 0;

  for (i = 0; strs[i] != NULL; ++i) {
    int len = strlen(strs[i]);

    if (strncasecmp(*buf, strs[i], len) == 0) {
      *buf += len;
      return i;
    }
  }
  return -1;
}

/* Needed for strptime. */
static int first_day(int year)
{
  int ret = 4;

  for (; year > 1970; --year)
    ret = (ret + 365 + is_leap_year(year) ? 1 : 0) % 7;
  return ret;
}

/*
 * Set `timeptr' given `wnum' (week number [0, 53])
 * Needed for strptime
 */

static void set_week_number_sun(struct tm* timeptr, int wnum)
{
  int fday = first_day(timeptr->tm_year + tm_year_base);

  timeptr->tm_yday = wnum * 7 + timeptr->tm_wday - fday;
  if (timeptr->tm_yday < 0) {
    timeptr->tm_wday = fday;
    timeptr->tm_yday = 0;
  }
}

/*
 * Set `timeptr' given `wnum' (week number [0, 53])
 * Needed for strptime
 */

static void set_week_number_mon(struct tm* timeptr, int wnum)
{
  int fday = (first_day(timeptr->tm_year + tm_year_base) + 6) % 7;

  timeptr->tm_yday = wnum * 7 + (timeptr->tm_wday + 6) % 7 - fday;
  if (timeptr->tm_yday < 0) {
    timeptr->tm_wday = (fday + 1) % 7;
    timeptr->tm_yday = 0;
  }
}

/*
 * Set `timeptr' given `wnum' (week number [0, 53])
 * Needed for strptime
 */
static void set_week_number_mon4(struct tm* timeptr, int wnum)
{
  int fday = (first_day(timeptr->tm_year + tm_year_base) + 6) % 7;
  int offset = 0;

  if (fday < 4)
    offset += 7;

  timeptr->tm_yday = offset + (wnum - 1) * 7 + timeptr->tm_wday - fday;
  if (timeptr->tm_yday < 0) {
    timeptr->tm_wday = fday;
    timeptr->tm_yday = 0;
  }
}

/* strptime: roken */
// extern "C"
static char* _strptime(const char* buf, const char* format, struct tm* timeptr,
                       int* gmt)
{
  char c;

  for (; (c = *format) != '\0'; ++format) {
    char* s;
    int ret;

    if (isspace(c)) {
      while (isspace(*buf))
        ++buf;
    }
    else if (c == '%' && format[1] != '\0') {
      c = *++format;
      if (c == 'E' || c == 'O')
        c = *++format;
      switch (c) {
      case 'A':
      case 'a':
        ret = match_string(&buf, full_weekdays);
        if (ret < 0)
          ret = match_string(&buf, abb_weekdays);
        if (ret < 0)
          return NULL;
        timeptr->tm_wday = ret;
        break;
      case 'B':
      case 'b':
      case 'h':
        ret = match_string(&buf, full_month);
        if (ret < 0)
          ret = match_string(&buf, abb_month);
        if (ret < 0)
          return NULL;
        timeptr->tm_mon = ret;
        break;
      case 'C':
        ret = strtol(buf, &s, 10);
        if (s == buf)
          return NULL;
        timeptr->tm_year = (ret * 100) - tm_year_base;
        buf = s;
        break;
      case 'c': /* %a %b %e %H:%M:%S %Y */
        s = strptime(buf, "%a %b %e %H:%M:%S %Y", timeptr);
        if (s == NULL)
          return NULL;
        buf = s;
        break;
      case 'D': /* %m/%d/%y */
        s = strptime(buf, "%m/%d/%y", timeptr);
        if (s == NULL)
          return NULL;
        buf = s;
        break;
      case 'd':
      case 'e':
        ret = strtol(buf, &s, 10);
        if (s == buf)
          return NULL;
        timeptr->tm_mday = ret;
        buf = s;
        break;
      case 'H':
      case 'k':
        ret = strtol(buf, &s, 10);
        if (s == buf)
          return NULL;
        timeptr->tm_hour = ret;
        buf = s;
        break;
      case 'I':
      case 'l':
        ret = strtol(buf, &s, 10);
        if (s == buf)
          return NULL;
        if (ret == 12)
          timeptr->tm_hour = 0;
        else
          timeptr->tm_hour = ret;
        buf = s;
        break;
      case 'j':
        ret = strtol(buf, &s, 10);
        if (s == buf)
          return NULL;
        timeptr->tm_yday = ret - 1;
        buf = s;
        break;
      case 'm':
        ret = strtol(buf, &s, 10);
        if (s == buf)
          return NULL;
        timeptr->tm_mon = ret - 1;
        buf = s;
        break;
      case 'M':
        ret = strtol(buf, &s, 10);
        if (s == buf)
          return NULL;
        timeptr->tm_min = ret;
        buf = s;
        break;
      case 'n':
        if (*buf == '\n')
          ++buf;
        else
          return NULL;
        break;
      case 'p':
        ret = match_string(&buf, ampm);
        if (ret < 0)
          return NULL;
        if (timeptr->tm_hour == 0) {
          if (ret == 1)
            timeptr->tm_hour = 12;
        }
        else
          timeptr->tm_hour += 12;
        break;
      case 'r': /* %I:%M:%S %p */
        s = strptime(buf, "%I:%M:%S %p", timeptr);
        if (s == NULL)
          return NULL;
        buf = s;
        break;
      case 'R': /* %H:%M */
        s = strptime(buf, "%H:%M", timeptr);
        if (s == NULL)
          return NULL;
        buf = s;
        break;
      case 'S':
        ret = strtol(buf, &s, 10);
        if (s == buf)
          return NULL;
        timeptr->tm_sec = ret;
        buf = s;
        break;
      case 't':
        if (*buf == '\t')
          ++buf;
        else
          return NULL;
        break;
      case 'T': /* %H:%M:%S */
      case 'X':
        s = strptime(buf, "%H:%M:%S", timeptr);
        if (s == NULL)
          return NULL;
        buf = s;
        break;
      case 'u':
        ret = strtol(buf, &s, 10);
        if (s == buf)
          return NULL;
        timeptr->tm_wday = ret - 1;
        buf = s;
        break;
      case 'w':
        ret = strtol(buf, &s, 10);
        if (s == buf)
          return NULL;
        timeptr->tm_wday = ret;
        buf = s;
        break;
      case 'U':
        ret = strtol(buf, &s, 10);
        if (s == buf)
          return NULL;
        set_week_number_sun(timeptr, ret);
        buf = s;
        break;
      case 'V':
        ret = strtol(buf, &s, 10);
        if (s == buf)
          return NULL;
        set_week_number_mon4(timeptr, ret);
        buf = s;
        break;
      case 'W':
        ret = strtol(buf, &s, 10);
        if (s == buf)
          return NULL;
        set_week_number_mon(timeptr, ret);
        buf = s;
        break;
      case 'x':
        s = strptime(buf, "%Y:%m:%d", timeptr);
        if (s == NULL)
          return NULL;
        buf = s;
        break;
      case 'y':
        ret = strtol(buf, &s, 10);
        if (s == buf)
          return NULL;
        /*
         * y represents stricty 2 digits, raise error if more than 3
         * digits are parsed.
         */
        if (ret > 99) {
          return NULL;
        }
        /*
         * The value in the range 69-99 refer to years in 20th century.
         * The value in the range 00-68 refer to years in 21st century.
         */
        if (ret < 69)
          timeptr->tm_year = 100 + ret;
        else
          timeptr->tm_year = ret;
        buf = s;
        break;
      case 'Y':
        ret = strtol(buf, &s, 10);
        if (s == buf)
          return NULL;
        timeptr->tm_year = ret - tm_year_base;
        buf = s;
        break;
      case 'Z':
        /* source:
         * cygwin-1.5.24-2-src/cygwin-1.5.24-2/winsup/cygwin/libc/strptime.cc */
        {
          const char* cp;
          char* zonestr;

          for (cp = buf; *cp && isupper((unsigned char)*cp); ++cp) { /*empty*/
          }
          if (cp - buf) {
            zonestr = (char*)alloca(cp - buf + 1);
            strncpy(zonestr, buf, cp - buf);
            zonestr[cp - buf] = '\0';
            tzset();
            if (0 == strcmp(zonestr, "GMT")) {
              *gmt = 1;
            }
            else if (0 == strcmp(zonestr, tzname[0])) {
              timeptr->tm_isdst = 0;
            }
            else if (0 == strcmp(zonestr, tzname[1])) {
              timeptr->tm_isdst = 1;
            }
            else {
              return 0;
            }
            buf += cp - buf;
          }
        }
        break;
      case '\0':
        --format;
      /* FALLTHROUGH */
      case '%':
        if (*buf == '%')
          ++buf;
        else
          return NULL;
        break;
      default:
        if (*buf == '%' || *++buf == c)
          ++buf;
        else
          return NULL;
        break;
      }
    }
    else {
      if (*buf == c)
        ++buf;
      else
        return NULL;
    }
  }
  return (char*)buf;
}

char* strptime(const char* buf, const char* format, struct tm* timeptr)
{
  char* ret;
  int gmt;

  gmt = 0;
  ret = _strptime(buf, format, timeptr, &gmt);
  return (ret);
}
