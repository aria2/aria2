/* gmp_obstack_vprintf -- formatted output to an obstack.

Copyright 2001, 2002 Free Software Foundation, Inc.

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

#if HAVE_OBSTACK_VPRINTF

#if HAVE_STDARG
#include <stdarg.h>
#else
#include <varargs.h>
#endif

#include <obstack.h>
#include <string.h>

#include "gmp.h"
#include "gmp-impl.h"


int
gmp_obstack_vprintf (struct obstack *ob, const char *fmt, va_list ap)
{
  ASSERT (! MEM_OVERLAP_P (obstack_base(ob), obstack_object_size(ob),
                           fmt, strlen(fmt)+1));

  return __gmp_doprnt (&__gmp_obstack_printf_funs, ob, fmt, ap);
}

#endif /* HAVE_OBSTACK_VPRINTF */
