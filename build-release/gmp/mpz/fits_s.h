/* int mpz_fits_X_p (mpz_t z) -- test whether z fits signed type X.

Copyright 1997, 2000, 2001, 2002 Free Software Foundation, Inc.

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
FUNCTION (mpz_srcptr z) __GMP_NOTHROW
{
  mp_size_t n = SIZ(z);
  mp_ptr p = PTR(z);
  mp_limb_t limb = p[0];

  if (n == 0)
    return 1;
  if (n == 1)
    return limb <= MAXIMUM;
  if (n == -1)
    return limb <= - (mp_limb_t) MINIMUM;
#if GMP_NAIL_BITS != 0
  {
    if ((p[1] >> GMP_NAIL_BITS) == 0)
      {
	limb += p[1] << GMP_NUMB_BITS;
	if (n == 2)
	  return limb <= MAXIMUM;
	if (n == -2)
	  return limb <= - (mp_limb_t) MINIMUM;
      }
  }
#endif
  return 0;
}
