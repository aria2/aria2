/* mpq_inits() -- Initialize multiple mpq_t variables and set them to 0.

Copyright 2009 Free Software Foundation, Inc.

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

#include <stdio.h>		/* for NULL */
#include "gmp.h"
#include "gmp-impl.h"

void
#if HAVE_STDARG
mpq_inits (mpq_ptr x, ...)
#else
mpq_inits (va_alist)
     va_dcl
#endif
{
  va_list  ap;

#if HAVE_STDARG
  va_start (ap, x);
#else
  mpq_ptr x;
  va_start (ap);
  x = va_arg (ap, mpq_ptr);
#endif

  while (x != NULL)
    {
      mpq_init (x);
      x = va_arg (ap, mpq_ptr);
    }
  va_end (ap);
}
