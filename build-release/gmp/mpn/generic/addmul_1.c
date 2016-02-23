/* mpn_addmul_1 -- multiply the N long limb vector pointed to by UP by VL,
   add the N least significant limbs of the product to the limb vector
   pointed to by RP.  Return the most significant limb of the product,
   adjusted for carry-out from the addition.

Copyright 1992, 1993, 1994, 1996, 2000, 2002, 2004 Free Software Foundation,
Inc.

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
#include "longlong.h"


#if GMP_NAIL_BITS == 0

mp_limb_t
mpn_addmul_1 (mp_ptr rp, mp_srcptr up, mp_size_t n, mp_limb_t vl)
{
  mp_limb_t ul, cl, hpl, lpl, rl;

  ASSERT (n >= 1);
  ASSERT (MPN_SAME_OR_SEPARATE_P (rp, up, n));

  cl = 0;
  do
    {
      ul = *up++;
      umul_ppmm (hpl, lpl, ul, vl);

      lpl += cl;
      cl = (lpl < cl) + hpl;

      rl = *rp;
      lpl = rl + lpl;
      cl += lpl < rl;
      *rp++ = lpl;
    }
  while (--n != 0);

  return cl;
}

#endif

#if GMP_NAIL_BITS == 1

mp_limb_t
mpn_addmul_1 (mp_ptr rp, mp_srcptr up, mp_size_t n, mp_limb_t vl)
{
  mp_limb_t shifted_vl, ul, rl, lpl, hpl, prev_hpl, cl, xl, c1, c2, c3;

  ASSERT (n >= 1);
  ASSERT (MPN_SAME_OR_SEPARATE_P (rp, up, n));
  ASSERT_MPN (rp, n);
  ASSERT_MPN (up, n);
  ASSERT_LIMB (vl);

  shifted_vl = vl << GMP_NAIL_BITS;
  cl = 0;
  prev_hpl = 0;
  do
    {
      ul = *up++;
      rl = *rp;
      umul_ppmm (hpl, lpl, ul, shifted_vl);
      lpl >>= GMP_NAIL_BITS;
      ADDC_LIMB (c1, xl, prev_hpl, lpl);
      ADDC_LIMB (c2, xl, xl, rl);
      ADDC_LIMB (c3, xl, xl, cl);
      cl = c1 + c2 + c3;
      *rp++ = xl;
      prev_hpl = hpl;
    }
  while (--n != 0);

  return prev_hpl + cl;
}

#endif

#if GMP_NAIL_BITS >= 2

mp_limb_t
mpn_addmul_1 (mp_ptr rp, mp_srcptr up, mp_size_t n, mp_limb_t vl)
{
  mp_limb_t shifted_vl, ul, rl, lpl, hpl, prev_hpl, xw, cl, xl;

  ASSERT (n >= 1);
  ASSERT (MPN_SAME_OR_SEPARATE_P (rp, up, n));
  ASSERT_MPN (rp, n);
  ASSERT_MPN (up, n);
  ASSERT_LIMB (vl);

  shifted_vl = vl << GMP_NAIL_BITS;
  cl = 0;
  prev_hpl = 0;
  do
    {
      ul = *up++;
      rl = *rp;
      umul_ppmm (hpl, lpl, ul, shifted_vl);
      lpl >>= GMP_NAIL_BITS;
      xw = prev_hpl + lpl + rl + cl;
      cl = xw >> GMP_NUMB_BITS;
      xl = xw & GMP_NUMB_MASK;
      *rp++ = xl;
      prev_hpl = hpl;
    }
  while (--n != 0);

  return prev_hpl + cl;
}

#endif
