/* mpz_rootrem(root, rem, u, nth) --  Set ROOT to trunc(U^(1/nth)) and
   set REM to the remainder.

Copyright 1999, 2000, 2001, 2002, 2003, 2005, 2012 Free Software Foundation, Inc.

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

#include <stdio.h>		/* for NULL */
#include "gmp.h"
#include "gmp-impl.h"

void
mpz_rootrem (mpz_ptr root, mpz_ptr rem, mpz_srcptr u, unsigned long int nth)
{
  mp_ptr rootp, up, remp;
  mp_size_t us, un, rootn, remn;
  TMP_DECL;

  us = SIZ(u);

  /* even roots of negatives provoke an exception */
  if (UNLIKELY (us < 0 && (nth & 1) == 0))
    SQRT_OF_NEGATIVE;

  /* root extraction interpreted as c^(1/nth) means a zeroth root should
     provoke a divide by zero, do this even if c==0 */
  if (UNLIKELY (nth == 0))
    DIVIDE_BY_ZERO;

  if (us == 0)
    {
      if (root != NULL)
	SIZ(root) = 0;
      SIZ(rem) = 0;
      return;
    }

  un = ABS (us);
  rootn = (un - 1) / nth + 1;

  TMP_MARK;

  /* FIXME: Perhaps disallow root == NULL */
  if (root != NULL && u != root)
    rootp = MPZ_REALLOC (root, rootn);
  else
    rootp = TMP_ALLOC_LIMBS (rootn);

  if (u != rem)
    remp = MPZ_REALLOC (rem, un);
  else
    remp = TMP_ALLOC_LIMBS (un);

  up = PTR(u);

  if (nth == 1)
    {
      MPN_COPY (rootp, up, un);
      remn = 0;
    }
  else
    {
      remn = mpn_rootrem (rootp, remp, up, un, (mp_limb_t) nth);
    }

  if (root != NULL)
    {
      SIZ(root) = us >= 0 ? rootn : -rootn;
      if (u == root)
	MPN_COPY (up, rootp, rootn);
    }

  if (u == rem)
    MPN_COPY (up, remp, remn);
  SIZ(rem) = us >= 0 ? remn : -remn;
  TMP_FREE;
}
