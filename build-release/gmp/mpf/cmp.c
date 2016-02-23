/* mpf_cmp -- Compare two floats.

Copyright 1993, 1994, 1996, 2001 Free Software Foundation, Inc.

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
mpf_cmp (mpf_srcptr u, mpf_srcptr v) __GMP_NOTHROW
{
  mp_srcptr up, vp;
  mp_size_t usize, vsize;
  mp_exp_t uexp, vexp;
  int cmp;
  int usign;

  uexp = u->_mp_exp;
  vexp = v->_mp_exp;

  usize = u->_mp_size;
  vsize = v->_mp_size;

  /* 1. Are the signs different?  */
  if ((usize ^ vsize) >= 0)
    {
      /* U and V are both non-negative or both negative.  */
      if (usize == 0)
	/* vsize >= 0 */
	return -(vsize != 0);
      if (vsize == 0)
	/* usize >= 0 */
	return usize != 0;
      /* Fall out.  */
    }
  else
    {
      /* Either U or V is negative, but not both.  */
      return usize >= 0 ? 1 : -1;
    }

  /* U and V have the same sign and are both non-zero.  */

  usign = usize >= 0 ? 1 : -1;

  /* 2. Are the exponents different?  */
  if (uexp > vexp)
    return usign;
  if (uexp < vexp)
    return -usign;

  usize = ABS (usize);
  vsize = ABS (vsize);

  up = u->_mp_d;
  vp = v->_mp_d;

#define STRICT_MPF_NORMALIZATION 0
#if ! STRICT_MPF_NORMALIZATION
  /* Ignore zeroes at the low end of U and V.  */
  while (up[0] == 0)
    {
      up++;
      usize--;
    }
  while (vp[0] == 0)
    {
      vp++;
      vsize--;
    }
#endif

  if (usize > vsize)
    {
      cmp = mpn_cmp (up + usize - vsize, vp, vsize);
      if (cmp == 0)
	return usign;
    }
  else if (vsize > usize)
    {
      cmp = mpn_cmp (up, vp + vsize - usize, usize);
      if (cmp == 0)
	return -usign;
    }
  else
    {
      cmp = mpn_cmp (up, vp, usize);
      if (cmp == 0)
	return 0;
    }
  return cmp > 0 ? usign : -usign;
}
