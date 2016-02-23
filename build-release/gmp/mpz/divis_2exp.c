/* mpz_divisible_2exp_p -- mpz by 2^n divisibility test

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

#include "gmp.h"
#include "gmp-impl.h"


int
mpz_divisible_2exp_p (mpz_srcptr a, mp_bitcnt_t d) __GMP_NOTHROW
{
  mp_size_t      i, dlimbs;
  unsigned       dbits;
  mp_ptr         ap;
  mp_limb_t      dmask;
  mp_size_t      asize;

  asize = ABSIZ(a);
  dlimbs = d / GMP_NUMB_BITS;

  /* if d covers the whole of a, then only a==0 is divisible */
  if (asize <= dlimbs)
    return asize == 0;

  /* whole limbs must be zero */
  ap = PTR(a);
  for (i = 0; i < dlimbs; i++)
    if (ap[i] != 0)
      return 0;

  /* left over bits must be zero */
  dbits = d % GMP_NUMB_BITS;
  dmask = (CNST_LIMB(1) << dbits) - 1;
  return (ap[dlimbs] & dmask) == 0;
}
