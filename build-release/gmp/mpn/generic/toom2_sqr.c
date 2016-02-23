/* mpn_toom2_sqr -- Square {ap,an}.

   Contributed to the GNU project by Torbjorn Granlund.

   THE FUNCTION IN THIS FILE IS INTERNAL WITH A MUTABLE INTERFACE.  IT IS ONLY
   SAFE TO REACH IT THROUGH DOCUMENTED INTERFACES.  IN FACT, IT IS ALMOST
   GUARANTEED THAT IT WILL CHANGE OR DISAPPEAR IN A FUTURE GNU MP RELEASE.

Copyright 2006, 2007, 2008, 2009, 2010, 2012 Free Software Foundation, Inc.

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

/* Evaluate in: -1, 0, +inf

  <-s--><--n-->
   ____ ______
  |_a1_|___a0_|

  v0  =  a0     ^2  #   A(0)^2
  vm1 = (a0- a1)^2  #  A(-1)^2
  vinf=      a1 ^2  # A(inf)^2
*/

#if TUNE_PROGRAM_BUILD || WANT_FAT_BINARY
#define MAYBE_sqr_toom2   1
#else
#define MAYBE_sqr_toom2							\
  (SQR_TOOM3_THRESHOLD >= 2 * SQR_TOOM2_THRESHOLD)
#endif

#define TOOM2_SQR_REC(p, a, n, ws)					\
  do {									\
    if (! MAYBE_sqr_toom2						\
	|| BELOW_THRESHOLD (n, SQR_TOOM2_THRESHOLD))			\
      mpn_sqr_basecase (p, a, n);					\
    else								\
      mpn_toom2_sqr (p, a, n, ws);					\
  } while (0)

void
mpn_toom2_sqr (mp_ptr pp,
	       mp_srcptr ap, mp_size_t an,
	       mp_ptr scratch)
{
  const int __gmpn_cpuvec_initialized = 1;
  mp_size_t n, s;
  mp_limb_t cy, cy2;
  mp_ptr asm1;

#define a0  ap
#define a1  (ap + n)

  s = an >> 1;
  n = an - s;

  ASSERT (0 < s && s <= n);

  asm1 = pp;

  /* Compute asm1.  */
  if (s == n)
    {
      if (mpn_cmp (a0, a1, n) < 0)
	{
	  mpn_sub_n (asm1, a1, a0, n);
	}
      else
	{
	  mpn_sub_n (asm1, a0, a1, n);
	}
    }
  else
    {
      if (mpn_zero_p (a0 + s, n - s) && mpn_cmp (a0, a1, s) < 0)
	{
	  mpn_sub_n (asm1, a1, a0, s);
	  MPN_ZERO (asm1 + s, n - s);
	}
      else
	{
	  mpn_sub (asm1, a0, n, a1, s);
	}
    }

#define v0	pp				/* 2n */
#define vinf	(pp + 2 * n)			/* s+s */
#define vm1	scratch				/* 2n */
#define scratch_out	scratch + 2 * n

  /* vm1, 2n limbs */
  TOOM2_SQR_REC (vm1, asm1, n, scratch_out);

  /* vinf, s+s limbs */
  TOOM2_SQR_REC (vinf, a1, s, scratch_out);

  /* v0, 2n limbs */
  TOOM2_SQR_REC (v0, ap, n, scratch_out);

  /* H(v0) + L(vinf) */
  cy = mpn_add_n (pp + 2 * n, v0 + n, vinf, n);

  /* L(v0) + H(v0) */
  cy2 = cy + mpn_add_n (pp + n, pp + 2 * n, v0, n);

  /* L(vinf) + H(vinf) */
  cy += mpn_add (pp + 2 * n, pp + 2 * n, n, vinf + n, s + s - n);

  cy -= mpn_sub_n (pp + n, pp + n, vm1, 2 * n);

  ASSERT (cy + 1  <= 3);
  ASSERT (cy2 <= 2);

  mpn_incr_u (pp + 2 * n, cy2);
  if (LIKELY (cy <= 2))
    mpn_incr_u (pp + 3 * n, cy);
  else
    mpn_decr_u (pp + 3 * n, 1);
}
