/* mpn_sb_div_qr_sec, mpn_sb_div_r_sec -- Compute Q = floor(U / V), U = U mod
   V.  Side-channel silent under the assumption that the used instructions are
   side-channel silent.

   Contributed to the GNU project by Torbjorn Granlund.

   THE FUNCTIONS IN THIS FILE ARE INTERNAL WITH MUTABLE INTERFACES.  IT IS ONLY
   SAFE TO REACH THEM THROUGH DOCUMENTED INTERFACES.  IN FACT, IT IS ALMOST
   GUARANTEED THAT THEY WILL CHANGE OR DISAPPEAR IN A FUTURE GNU MP RELEASE.

Copyright 2011, 2012 Free Software Foundation, Inc.

This file is part of the GNU MP Library.

The GNU MP Library is free software; you can redistribute it and/or modify it
under the terms of the GNU Lesser General Public License as published by the
Free Software Foundation; either version 3 of the License, or (at your option)
any later version.

The GNU MP Library is distributed in the hope that it will be useful, but
WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License
for more details.

You should have received a copy of the GNU Lesser General Public License along
with the GNU MP Library.  If not, see http://www.gnu.org/licenses/.  */

#include "gmp.h"
#include "gmp-impl.h"
#include "longlong.h"

#if OPERATION_sb_div_qr_sec
/* Needs (nn + dn + 1) + mpn_sbpi1_div_qr_sec's needs of (2nn' - dn + 1) for a
   total of 3nn + 4 limbs at tp.  Note that mpn_sbpi1_div_qr_sec's nn is one
   greater than ours, therefore +4 and not just +2.  */
#define FNAME mpn_sb_div_qr_sec
#define Q(q) q,
#endif
#if OPERATION_sb_div_r_sec
/* Needs (nn + dn + 1) + mpn_sbpi1_div_r_sec's needs of (dn + 1) for a total of
   nn + 2dn + 2 limbs at tp.  */
#define FNAME mpn_sb_div_r_sec
#define Q(q)
#endif

void
FNAME (Q(mp_ptr qp)
       mp_ptr np, mp_size_t nn,
       mp_srcptr dp, mp_size_t dn,
       mp_ptr tp)
{
  mp_limb_t d1, d0, qh;
  unsigned int cnt;
  mp_ptr np2, dp2;
  gmp_pi1_t dinv;
  mp_limb_t inv32;
  mp_limb_t cy;

  ASSERT (dn >= 1);
  ASSERT (nn >= dn);
  ASSERT (dp[dn - 1] != 0);

  d1 = dp[dn - 1];
  count_leading_zeros (cnt, d1);

  if (cnt != 0)
    {
      dp2 = tp;					/* dn limbs */
      mpn_lshift (dp2, dp, dn, cnt);

      np2 = tp + dn;				/* (nn + 1) limbs */
      cy = mpn_lshift (np2, np, nn, cnt);
      np2[nn++] = cy;
    }
  else
    {
      /* FIXME: Consider copying np->np2 here, adding a 0-limb at the top.
	 That would simplify the underlying sbpi1 function, since then it
	 could assume nn > dn.  */
      dp2 = (mp_ptr) dp;
      np2 = np;
    }

  d0 = dp2[dn - 1];
  d0 += (~d0 != 0);
  invert_limb (inv32, d0);

  /* We add nn + dn to tp here, not nn + 1 + dn, as expected.  This is since nn
     here will have been incremented.  */
#if OPERATION_sb_div_qr_sec
  qh = mpn_sbpi1_div_qr_sec (qp, np2, nn, dp2, dn, inv32, tp + nn + dn);
#else
  mpn_sbpi1_div_r_sec (np2, nn, dp2, dn, inv32, tp + nn + dn);
#endif

  if (cnt == 0)
    ;				/* we have np = np2 here. */
  else
    mpn_rshift (np, np2, dn, cnt);

#if OPERATION_sb_div_qr_sec
  if (cnt == 0)
    qp[nn - dn] = qh;
#endif
}
