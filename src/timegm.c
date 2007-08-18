/* timegm.c - libc replacement function
 * Copyright (C) 2004 Free Software Foundation, Inc.
 *
 * This file is part of GnuPG.
 *
 * GnuPG is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * GnuPG is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301,
 * USA.
 */

/*
  timegm() is a GNU function that might not be available everywhere.
  It's basically the inverse of gmtime() - you give it a struct tm,
  and get back a time_t.  It differs from mktime() in that it handles
  the case where the struct tm is UTC and the local environment isn't.

  Some BSDs don't handle the putenv("foo") case properly, so we use
  unsetenv if the platform has it to remove environment variables.
*/

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif // HAVE_CONFIG_H

#include <time.h>
#include <stdlib.h>
#include <string.h>

time_t
timegm(struct tm *tm)
{
  time_t answer;
  char *zone;

  zone=getenv("TZ");
  putenv("TZ=UTC");
  tzset();
  answer=mktime(tm);
  if(zone)
    {
      char *old_zone;

      old_zone=malloc(3+strlen(zone)+1);
      if(old_zone)
	{
	  strcpy(old_zone,"TZ=");
	  strcat(old_zone,zone);
	  putenv(old_zone);
	}
    }
  else
#ifdef HAVE_UNSETENV
    unsetenv("TZ");
#else
    putenv("TZ=");
#endif

  tzset();
  return answer;
}
