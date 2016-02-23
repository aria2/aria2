/* gmp_snprintf -- formatted output to an fixed size buffer.

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
gmp_snprintf (char *buf, size_t size, const char *fmt, ...)
#else
gmp_snprintf (va_alist)
     va_dcl
#endif
{
  struct gmp_snprintf_t d;
  va_list  ap;
  int      ret;

#if HAVE_STDARG
  va_start (ap, fmt);
  d.buf = buf;
  d.size = size;

#else
  const char *fmt;
  va_start (ap);
  d.buf = va_arg (ap, char *);
  d.size = va_arg (ap, size_t);
  fmt = va_arg (ap, const char *);
#endif

  ASSERT (! MEM_OVERLAP_P (buf, size, fmt, strlen(fmt)+1));

  ret = __gmp_doprnt (&__gmp_snprintf_funs, &d, fmt, ap);
  va_end (ap);
  return ret;
}
