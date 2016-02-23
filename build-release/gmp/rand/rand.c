/* gmp_randinit (state, algorithm, ...) -- Initialize a random state.

Copyright 1999, 2000, 2001, 2002 Free Software Foundation, Inc.

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

#include <stdio.h> /* for NULL */

#if HAVE_STDARG
#include <stdarg.h>
#else
#include <varargs.h>
#endif

#include "gmp.h"
#include "gmp-impl.h"

void
#if HAVE_STDARG
gmp_randinit (gmp_randstate_t rstate,
	      gmp_randalg_t alg,
	      ...)
#else
gmp_randinit (va_alist)
     va_dcl
#endif
{
  va_list ap;
#if HAVE_STDARG
  va_start (ap, alg);
#else
  __gmp_randstate_struct *rstate;
  gmp_randalg_t alg;
  va_start (ap);
  rstate = va_arg (ap, __gmp_randstate_struct *);
  alg = va_arg (ap, gmp_randalg_t);
#endif

  switch (alg) {
  case GMP_RAND_ALG_LC:
    if (! gmp_randinit_lc_2exp_size (rstate, va_arg (ap, unsigned long)))
      gmp_errno |= GMP_ERROR_INVALID_ARGUMENT;
    break;
  default:
    gmp_errno |= GMP_ERROR_UNSUPPORTED_ARGUMENT;
    break;
  }
  va_end (ap);
}
