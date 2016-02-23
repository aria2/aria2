/* gmp_sprintf -- formatted output to an unrestricted string.

Copyright 2001 Free Software Foundation, Inc.

This file is part of the GNU MP Library.

The GNU MP Library is free software; you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation; either version 3 of the License, or (at your
option) any later version.

The GNU MP Library is distributed in the hope that it will be useful, but
WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public
License for more details.

You should have received a copy of the GNU Lesser General Public License
along with the GNU MP Library.  If not, see http://www.gnu.org/licenses/.  */

#include "config.h"

#if HAVE_STDARG
#include <stdarg.h>
#else
#include <varargs.h>
#endif

#include <string.h>    /* for strlen */

#include "gmp.h"
#include "gmp-impl.h"


int
#if HAVE_STDARG
gmp_sprintf (char *buf, const char *fmt, ...)
#else
gmp_sprintf (va_alist)
     va_dcl
#endif
{
#if WANT_ASSERT
  int      fmtlen = strlen(fmt);
#endif
  va_list  ap;
  int      ret;

#if HAVE_STDARG
  va_start (ap, fmt);
#else
  char       *buf;
  const char *fmt;
  va_start (ap);
  buf = va_arg (ap, char *);
  fmt = va_arg (ap, const char *);
#endif

  ret = __gmp_doprnt (&__gmp_sprintf_funs, &buf, fmt, ap);
  va_end (ap);

  ASSERT (! MEM_OVERLAP_P (buf, strlen(buf)+1, fmt, fmtlen+1));

  return ret;
}
