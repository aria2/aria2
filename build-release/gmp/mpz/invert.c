/* mpz_invert (inv, x, n).  Find multiplicative inverse of X in Z(N).
   If X has an inverse, return non-zero and store inverse in INVERSE,
   otherwise, return 0 and put garbage in INVERSE.

Copyright 1996, 1997, 1998, 1999, 2000, 2001, 2005, 2012 Free Software
Foundation, Inc.

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

int
mpz_invert (mpz_ptr inverse, mpz_srcptr x, mpz_srcptr n)
{
  mpz_t gcd, tmp;
  mp_size_t xsize, nsize, size;
  TMP_DECL;

  xsize = ABSIZ (x);
  nsize = ABSIZ (n);

  /* No inverse exists if the leftside operand is 0.  Likewise, no
     inverse exists if the mod operand is 1.  */
  if (xsize == 0 || (nsize == 1 && (PTR (n))[0] == 1))
    return 0;

  size = MAX (xsize, nsize) + 1;
  TMP_MARK;

  MPZ_TMP_INIT (gcd, size);
  MPZ_TMP_INIT (tmp, size);
  mpz_gcdext (gcd, tmp, (mpz_ptr) 0, x, n);

  /* If no inverse existed, return with an indication of that.  */
  if (!MPZ_EQUAL_1_P (gcd))
    {
      TMP_FREE;
      return 0;
    }

  /* Make sure we return a positive inverse.  */
  if (SIZ (tmp) < 0)
    {
      if (SIZ (n) < 0)
	mpz_sub (inverse, tmp, n);
      else
	mpz_add (inverse, tmp, n);
    }
  else
    mpz_set (inverse, tmp);

  TMP_FREE;
  return 1;
}
