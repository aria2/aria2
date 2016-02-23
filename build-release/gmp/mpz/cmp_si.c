/* mpz_cmp_si(u,v) -- Compare an integer U with a single-word int V.
   Return positive, zero, or negative based on if U > V, U == V, or U < V.

Copyright 1991, 1993, 1994, 1995, 1996, 2000, 2001, 2002, 2012, 2013 Free
Software Foundation, Inc.

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
_mpz_cmp_si (mpz_srcptr u, signed long int v_digit) __GMP_NOTHROW
{
  mp_size_t usize = SIZ (u);
  mp_size_t vsize;
  mp_limb_t u_digit;
  unsigned long int absv_digit;

#if GMP_NAIL_BITS != 0
  /* FIXME.  This isn't very pretty.  */
  mpz_t tmp;
  mp_limb_t tt[2];
  PTR(tmp) = tt;
  ALLOC(tmp) = 2;
  mpz_set_si (tmp, v_digit);
  return mpz_cmp (u, tmp);
#endif

  vsize = 0;
  if (v_digit > 0)
    vsize = 1;
  else if (v_digit < 0)
    {
      vsize = -1;
    }
  absv_digit = ABS_CAST (unsigned long int, v_digit);

  if (usize != vsize)
    return usize - vsize;

  if (usize == 0)
    return 0;

  u_digit = PTR (u)[0];

  if (u_digit == (mp_limb_t) absv_digit)
    return 0;

  if (u_digit > (mp_limb_t) absv_digit)
    return usize;
  else
    return -usize;
}
