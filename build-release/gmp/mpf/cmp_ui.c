/* mpf_cmp_ui -- Compare a float with an unsigned integer.

Copyright 1993, 1994, 1995, 1999, 2001, 2002 Free Software Foundation, Inc.

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
mpf_cmp_ui (mpf_srcptr u, unsigned long int vval) __GMP_NOTHROW
{
  mp_srcptr up;
  mp_size_t usize;
  mp_exp_t uexp;
  mp_limb_t ulimb;

  uexp = u->_mp_exp;
  usize = u->_mp_size;

  /* 1. Is U negative?  */
  if (usize < 0)
    return -1;
  /* We rely on usize being non-negative in the code that follows.  */

  if (vval == 0)
    return usize != 0;

  /* 2. Are the exponents different (V's exponent == 1)?  */
#if GMP_NAIL_BITS != 0
  if (uexp > 1 + (vval > GMP_NUMB_MAX))
    return 1;
  if (uexp < 1 + (vval > GMP_NUMB_MAX))
    return -1;
#else
  if (uexp > 1)
    return 1;
  if (uexp < 1)
    return -1;
#endif

  up = u->_mp_d;

  ulimb = up[usize - 1];
#if GMP_NAIL_BITS != 0
  if (usize >= 2 && uexp == 2)
    {
      if ((ulimb >> GMP_NAIL_BITS) != 0)
	return 1;
      ulimb = (ulimb << GMP_NUMB_BITS) | up[usize - 2];
      usize--;
    }
#endif
  usize--;

  /* 3. Compare the most significant mantissa limb with V.  */
  if (ulimb > vval)
    return 1;
  else if (ulimb < vval)
    return -1;

  /* Ignore zeroes at the low end of U.  */
  while (*up == 0)
    {
      up++;
      usize--;
    }

  /* 4. Now, if the number of limbs are different, we have a difference
     since we have made sure the trailing limbs are not zero.  */
  if (usize > 0)
    return 1;

  /* Wow, we got zero even if we tried hard to avoid it.  */
  return 0;
}
