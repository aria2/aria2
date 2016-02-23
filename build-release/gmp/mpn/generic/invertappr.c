/* mpn_invertappr and helper functions.  Compute I such that
   floor((B^{2n}-1)/U - 1 <= I + B^n <= floor((B^{2n}-1)/U.

   Contributed to the GNU project by Marco Bodrato.

   The algorithm used here was inspired by ApproximateReciprocal from "Modern
   Computer Arithmetic", by Richard P. Brent and Paul Zimmermann.  Special
   thanks to Paul Zimmermann for his very valuable suggestions on all the
   theoretical aspects during the work on this code.

   THE FUNCTIONS IN THIS FILE ARE INTERNAL WITH MUTABLE INTERFACES.  IT IS ONLY
   SAFE TO REACH THEM THROUGH DOCUMENTED INTERFACES.  IN FACT, IT IS ALMOST
   GUARANTEED THAT THEY WILL CHANGE OR DISAPPEAR IN A FUTURE GMP RELEASE.

Copyright (C) 2007, 2009, 2010, 2012 Free Software Foundation, Inc.

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

/* FIXME: Remove NULL and TMP_*, as soon as all the callers properly
   allocate and pass the scratch to the function. */
#include <stdlib.h>		/* for NULL */

#include "gmp.h"
#include "gmp-impl.h"
#include "longlong.h"

/* FIXME: The iterative version splits the operand in two slighty unbalanced
   parts, the use of log_2 (or counting the bits) underestimate the maximum
   number of iterations.  */

#if TUNE_PROGRAM_BUILD
#define NPOWS \
 ((sizeof(mp_size_t) > 6 ? 48 : 8*sizeof(mp_size_t)))
#define MAYBE_dcpi1_divappr   1
#else
#define NPOWS \
 ((sizeof(mp_size_t) > 6 ? 48 : 8*sizeof(mp_size_t)) - LOG2C (INV_NEWTON_THRESHOLD))
#define MAYBE_dcpi1_divappr \
  (INV_NEWTON_THRESHOLD < DC_DIVAPPR_Q_THRESHOLD)
#if (INV_NEWTON_THRESHOLD > INV_MULMOD_BNM1_THRESHOLD) && \
    (INV_APPR_THRESHOLD > INV_MULMOD_BNM1_THRESHOLD)
#undef  INV_MULMOD_BNM1_THRESHOLD
#define INV_MULMOD_BNM1_THRESHOLD 0 /* always when Newton */
#endif
#endif

/* All the three functions mpn{,_bc,_ni}_invertappr (ip, dp, n, scratch), take
   the strictly normalised value {dp,n} (i.e., most significant bit must be set)
   as an input, and compute {ip,n}: the approximate reciprocal of {dp,n}.

   Let e = mpn*_invertappr (ip, dp, n, scratch) be the returned value; the
   following conditions are satisfied by the output:
     0 <= e <= 1;
     {dp,n}*(B^n+{ip,n}) < B^{2n} <= {dp,n}*(B^n+{ip,n}+1+e) .
   I.e. e=0 means that the result {ip,n} equals the one given by mpn_invert.
	e=1 means that the result _may_ be one less than expected.

   The _bc version returns e=1 most of the time.
   The _ni version should return e=0 most of the time; only about 1% of
   possible random input should give e=1.

   When the strict result is needed, i.e., e=0 in the relation above:
     {dp,n}*(B^n+{ip,n}) < B^{2n} <= {dp,n}*(B^n+{ip,n}+1) ;
   the function mpn_invert (ip, dp, n, scratch) should be used instead.  */

/* Maximum scratch needed by this branch (at tp): 3*n + 2 */
static mp_limb_t
mpn_bc_invertappr (mp_ptr ip, mp_srcptr dp, mp_size_t n, mp_ptr tp)
{
  mp_ptr xp;

  ASSERT (n > 0);
  ASSERT (dp[n-1] & GMP_NUMB_HIGHBIT);
  ASSERT (! MPN_OVERLAP_P (ip, n, dp, n));
  ASSERT (! MPN_OVERLAP_P (ip, n, tp, mpn_invertappr_itch(n)));
  ASSERT (! MPN_OVERLAP_P (dp, n, tp, mpn_invertappr_itch(n)));

  /* Compute a base value of r limbs. */
  if (n == 1)
    invert_limb (*ip, *dp);
  else {
    mp_size_t i;
    xp = tp + n + 2;				/* 2 * n limbs */

    for (i = n - 1; i >= 0; i--)
      xp[i] = GMP_NUMB_MAX;
    mpn_com (xp + n, dp, n);

    /* Now xp contains B^2n - {dp,n}*B^n - 1 */

    /* FIXME: if mpn_*pi1_divappr_q handles n==2, use it! */
    if (n == 2) {
      mpn_divrem_2 (ip, 0, xp, 4, dp);
    } else {
      gmp_pi1_t inv;
      invert_pi1 (inv, dp[n-1], dp[n-2]);
      if (! MAYBE_dcpi1_divappr
	  || BELOW_THRESHOLD (n, DC_DIVAPPR_Q_THRESHOLD))
	mpn_sbpi1_divappr_q (ip, xp, 2 * n, dp, n, inv.inv32);
      else
	mpn_dcpi1_divappr_q (ip, xp, 2 * n, dp, n, &inv);
      MPN_DECR_U(ip, n, 1);
      return 1;
    }
  }
  return 0;
}

/* mpn_ni_invertappr: computes the approximate reciprocal using Newton's
   iterations (at least one).

   Inspired by Algorithm "ApproximateReciprocal", published in "Modern Computer
   Arithmetic" by Richard P. Brent and Paul Zimmermann, algorithm 3.5, page 121
   in version 0.4 of the book.

   Some adaptations were introduced, to allow product mod B^m-1 and return the
   value e.

   USE_MUL_N = 1 (default) introduces a correction in such a way that "the
   value of B^{n+h}-T computed at step 8 cannot exceed B^n-1" (the book reads
   "2B^n-1").  This correction should not require to modify the proof.

   We use a wrapped product modulo B^m-1.  NOTE: is there any normalisation
   problem for the [0] class?  It shouldn't: we compute 2*|A*X_h - B^{n+h}| <
   B^m-1.  We may get [0] if and only if we get AX_h = B^{n+h}.  This can
   happen only if A=B^{n}/2, but this implies X_h = B^{h}*2-1 i.e., AX_h =
   B^{n+h} - A, then we get into the "negative" branch, where X_h is not
   incremented (because A < B^n).

   FIXME: the scratch for mulmod_bnm1 does not currently fit in the scratch, it
   is allocated apart.  */

#define USE_MUL_N 1

mp_limb_t
mpn_ni_invertappr (mp_ptr ip, mp_srcptr dp, mp_size_t n, mp_ptr scratch)
{
  mp_limb_t cy;
  mp_ptr xp;
  mp_size_t rn, mn;
  mp_size_t sizes[NPOWS], *sizp;
  mp_ptr tp;
  TMP_DECL;
#define rp scratch

  ASSERT (n > 2);
  ASSERT (dp[n-1] & GMP_NUMB_HIGHBIT);
  ASSERT (! MPN_OVERLAP_P (ip, n, dp, n));
  ASSERT (! MPN_OVERLAP_P (ip, n, scratch, mpn_invertappr_itch(n)));
  ASSERT (! MPN_OVERLAP_P (dp, n, scratch, mpn_invertappr_itch(n)));

  /* Compute the computation precisions from highest to lowest, leaving the
     base case size in 'rn'.  */
  sizp = sizes;
  rn = n;
  do {
    *sizp = rn;
    rn = ((rn) >> 1) + 1;
    sizp ++;
  } while (ABOVE_THRESHOLD (rn, INV_NEWTON_THRESHOLD));

  /* We search the inverse of 0.{dp,n}, we compute it as 1.{ip,n} */
  dp += n;
  ip += n;

  /* Compute a base value of rn limbs. */
  mpn_bc_invertappr (ip - rn, dp - rn, rn, scratch);

  TMP_MARK;

  if (ABOVE_THRESHOLD (n, INV_MULMOD_BNM1_THRESHOLD))
    {
      mn = mpn_mulmod_bnm1_next_size (n + 1);
      tp = TMP_ALLOC_LIMBS (mpn_mulmod_bnm1_itch (mn, n, (n >> 1) + 1));
    }
  /* Use Newton's iterations to get the desired precision.*/

  /* define rp scratch; 2rn + 1 limbs <= 2(n>>1 + 1) + 1 <= n + 3  limbs */
  /* Maximum scratch needed by this branch <= 3*n + 2 */
  xp = scratch + n + 3;				/*  n + rn limbs */
  while (1) {
    mp_limb_t method;

    n = *--sizp;
    /*
      v    n  v
      +----+--+
      ^ rn ^
    */

    /* Compute i_jd . */
    if (BELOW_THRESHOLD (n, INV_MULMOD_BNM1_THRESHOLD)
	|| ((mn = mpn_mulmod_bnm1_next_size (n + 1)) > (n + rn))) {
      /* FIXME: We do only need {xp,n+1}*/
      mpn_mul (xp, dp - n, n, ip - rn, rn);
      mpn_add_n (xp + rn, xp + rn, dp - n, n - rn + 1);
      method = 1; /* Remember we used (truncated) product */
      /* We computed cy.{xp,rn+n} <- 1.{ip,rn} * 0.{dp,n} */
    } else { /* Use B^n-1 wraparound */
      mpn_mulmod_bnm1 (xp, mn, dp - n, n, ip - rn, rn, tp);
      /* We computed {xp,mn} <- {ip,rn} * {dp,n} mod (B^mn-1) */
      /* We know that 2*|ip*dp + dp*B^rn - B^{rn+n}| < B^mn-1 */
      /* Add dp*B^rn mod (B^mn-1) */
      ASSERT (n >= mn - rn);
      xp[mn] = 1 + mpn_add_n (xp + rn, xp + rn, dp - n, mn - rn);
      cy = mpn_add_n (xp, xp, dp - (n - (mn - rn)), n - (mn - rn));
      MPN_INCR_U (xp + n - (mn - rn), mn + 1 - n + (mn - rn), cy);
      ASSERT (n + rn >=  mn);
      /* Subtract B^{rn+n} */
      MPN_DECR_U (xp + rn + n - mn, 2*mn + 1 - rn - n, 1);
      if (xp[mn])
	MPN_INCR_U (xp, mn, xp[mn] - 1);
      else
	MPN_DECR_U (xp, mn, 1);
      method = 0; /* Remember we are working Mod B^m-1 */
    }

    if (xp[n] < 2) { /* "positive" residue class */
      cy = 1;
      while (xp[n] || mpn_cmp (xp, dp - n, n)>0) {
	xp[n] -= mpn_sub_n (xp, xp, dp - n, n);
	cy ++;
      }
      MPN_DECR_U(ip - rn, rn, cy);
      ASSERT (cy <= 4); /* at most 3 cycles for the while above */
      ASSERT_NOCARRY (mpn_sub_n (xp, dp - n, xp, n));
      ASSERT (xp[n] == 0);
    } else { /* "negative" residue class */
      mpn_com (xp, xp, n + 1);
      MPN_INCR_U(xp, n + 1, method);
      ASSERT (xp[n] <= 1);
#if USE_MUL_N
      if (xp[n]) {
	MPN_INCR_U(ip - rn, rn, 1);
	ASSERT_CARRY (mpn_sub_n (xp, xp, dp - n, n));
      }
#endif
    }

    /* Compute x_ju_j. FIXME:We need {rp+rn,rn}, mulhi? */
#if USE_MUL_N
    mpn_mul_n (rp, xp + n - rn, ip - rn, rn);
#else
    rp[2*rn] = 0;
    mpn_mul (rp, xp + n - rn, rn + xp[n], ip - rn, rn);
#endif
    /* We need _only_ the carry from the next addition  */
    /* Anyway 2rn-n <= 2... we don't need to optimise.  */
    cy = mpn_add_n (rp + rn, rp + rn, xp + n - rn, 2*rn - n);
    cy = mpn_add_nc (ip - n, rp + 3*rn - n, xp + rn, n - rn, cy);
    MPN_INCR_U (ip - rn, rn, cy + (1-USE_MUL_N)*(rp[2*rn] + xp[n]));
    if (sizp == sizes) { /* Get out of the cycle */
      /* Check for possible carry propagation from below. */
      cy = rp[3*rn - n - 1] > GMP_NUMB_MAX - 7; /* Be conservative. */
/*    cy = mpn_add_1 (rp + rn, rp + rn, 2*rn - n, 4); */
      break;
    }
    rn = n;
  }
  TMP_FREE;

  return cy;
#undef rp
}

mp_limb_t
mpn_invertappr (mp_ptr ip, mp_srcptr dp, mp_size_t n, mp_ptr scratch)
{
  mp_limb_t res;
  TMP_DECL;

  TMP_MARK;

  if (scratch == NULL)
    scratch = TMP_ALLOC_LIMBS (mpn_invertappr_itch (n));

  ASSERT (n > 0);
  ASSERT (dp[n-1] & GMP_NUMB_HIGHBIT);
  ASSERT (! MPN_OVERLAP_P (ip, n, dp, n));
  ASSERT (! MPN_OVERLAP_P (ip, n, scratch, mpn_invertappr_itch(n)));
  ASSERT (! MPN_OVERLAP_P (dp, n, scratch, mpn_invertappr_itch(n)));

  if (BELOW_THRESHOLD (n, INV_NEWTON_THRESHOLD))
    res = mpn_bc_invertappr (ip, dp, n, scratch);
  else
    res = mpn_ni_invertappr (ip, dp, n, scratch);

  TMP_FREE;
  return res;
}
