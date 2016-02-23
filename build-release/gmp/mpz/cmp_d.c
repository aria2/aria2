/* mpz_cmp_d -- compare absolute values of mpz and double.

Copyright 2001, 2002, 2003 Free Software Foundation, Inc.

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

#if HAVE_FLOAT_H
#include <float.h>  /* for DBL_MAX */
#endif

#include "gmp.h"
#include "gmp-impl.h"


#define RETURN_CMP(zl, dl)                      \
  do {                                          \
    zlimb = (zl);                               \
    dlimb = (dl);                               \
    if (zlimb != dlimb)                         \
      return (zlimb >= dlimb ? ret : -ret);     \
  } while (0)

#define RETURN_NONZERO(ptr, size, val)          \
  do {                                          \
    mp_size_t __i;                              \
    for (__i = (size)-1; __i >= 0; __i--)       \
      if ((ptr)[__i] != 0)                      \
        return val;                             \
    return 0;                                   \
  } while (0)


int
mpz_cmp_d (mpz_srcptr z, double d)
{
  mp_limb_t  darray[LIMBS_PER_DOUBLE], zlimb, dlimb;
  mp_srcptr  zp;
  mp_size_t  zsize;
  int        dexp, ret;

  /* d=NaN is an invalid operation, there's no sensible return value.
     d=Inf or -Inf is always bigger than z.  */
  DOUBLE_NAN_INF_ACTION (d, __gmp_invalid_operation (), goto z_zero);

  /* 1. Either operand zero. */
  zsize = SIZ(z);
  if (d == 0.0)
    return zsize;
  if (zsize == 0)
    {
    z_zero:
      return (d < 0.0 ? 1 : -1);
    }

  /* 2. Opposite signs. */
  if (zsize >= 0)
    {
      if (d < 0.0)
	return 1;    /* >=0 cmp <0 */
      ret = 1;
    }
  else
    {
      if (d >= 0.0)
	return -1;   /* <0 cmp >=0 */
      ret = -1;
      d = -d;
      zsize = -zsize;
    }

  /* 3. Small d, knowing abs(z) >= 1. */
  if (d < 1.0)
    return ret;

  dexp = __gmp_extract_double (darray, d);
  ASSERT (dexp >= 1);

  /* 4. Check for different high limb positions. */
  if (zsize != dexp)
    return (zsize >= dexp ? ret : -ret);

  /* 5. Limb data. */
  zp = PTR(z);

#if LIMBS_PER_DOUBLE == 2
  RETURN_CMP (zp[zsize-1], darray[1]);
  if (zsize == 1)
    return (darray[0] != 0 ? -ret : 0);

  RETURN_CMP (zp[zsize-2], darray[0]);
  RETURN_NONZERO (zp, zsize-2, ret);
#endif

#if LIMBS_PER_DOUBLE == 3
  RETURN_CMP (zp[zsize-1], darray[2]);
  if (zsize == 1)
    return ((darray[0] | darray[1]) != 0 ? -ret : 0);

  RETURN_CMP (zp[zsize-2], darray[1]);
  if (zsize == 2)
    return (darray[0] != 0 ? -ret : 0);

  RETURN_CMP (zp[zsize-3], darray[0]);
  RETURN_NONZERO (zp, zsize-3, ret);
#endif

#if LIMBS_PER_DOUBLE >= 4
  {
    int i;
    for (i = 1; i <= LIMBS_PER_DOUBLE; i++)
      {
	RETURN_CMP (zp[zsize-i], darray[LIMBS_PER_DOUBLE-i]);
	if (i >= zsize)
	  RETURN_NONZERO (darray, LIMBS_PER_DOUBLE-i, -ret);
      }
    RETURN_NONZERO (zp, zsize-LIMBS_PER_DOUBLE, ret);
  }
#endif
}
