/* mpz_fdiv_r -- Division rounding the quotient towards -infinity.
   The remainder gets the same sign as the denominator.

Copyright 1994, 1995, 1996, 2001, 2005, 2012 Free Software Foundation, Inc.

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

#include "gmp.h"
#include "gmp-impl.h"

void
mpz_fdiv_r (mpz_ptr rem, mpz_srcptr dividend, mpz_srcptr divisor)
{
  mp_size_t divisor_size = SIZ (divisor);
  mpz_t temp_divisor;		/* N.B.: lives until function returns! */
  TMP_DECL;

  TMP_MARK;

  /* We need the original value of the divisor after the remainder has been
     preliminary calculated.  We have to copy it to temporary space if it's
     the same variable as REM.  */
  if (rem == divisor)
    {
      MPZ_TMP_INIT (temp_divisor, ABS (divisor_size));
      mpz_set (temp_divisor, divisor);
      divisor = temp_divisor;
    }

  mpz_tdiv_r (rem, dividend, divisor);

  if ((divisor_size ^ SIZ (dividend)) < 0 && SIZ (rem) != 0)
    mpz_add (rem, rem, divisor);

  TMP_FREE;
}
