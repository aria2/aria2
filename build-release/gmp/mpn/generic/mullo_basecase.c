/* mpn_mullo_basecase -- Internal routine to multiply two natural
   numbers of length m and n and return the low part.

   THIS IS AN INTERNAL FUNCTION WITH A MUTABLE INTERFACE.  IT IS ONLY
   SAFE TO REACH THIS FUNCTION THROUGH DOCUMENTED INTERFACES.


Copyright (C) 2000, 2002, 2004 Free Software Foundation, Inc.

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

/*
  FIXME: Should use mpn_addmul_2 (and higher).
*/

void
mpn_mullo_basecase (mp_ptr rp, mp_srcptr up, mp_srcptr vp, mp_size_t n)
{
  mp_size_t i;

  mpn_mul_1 (rp, up, n, vp[0]);

  for (i = 1; i < n; i++)
    mpn_addmul_1 (rp + i, up, n - i, vp[i]);
}
