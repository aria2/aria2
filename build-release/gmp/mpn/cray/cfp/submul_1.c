/* mpn_submul_1 for Cray PVP.

Copyright 1996, 2000, 2001 Free Software Foundation, Inc.

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

mp_limb_t
mpn_submul_1 (mp_ptr rp, mp_srcptr up, mp_size_t n, mp_limb_t limb)
{
  mp_limb_t p0[n], p1[n], tp[n];
  mp_limb_t cy_limb;

  GMPN_MULWW (p1, p0, up, &n, &limb);
  cy_limb = mpn_sub_n (tp, rp, p0, n);
  rp[0] = tp[0];
  if (n != 1)
    cy_limb += mpn_sub_n (rp + 1, tp + 1, p1, n - 1);
  cy_limb += p1[n - 1];

  return cy_limb;
}
