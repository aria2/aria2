/* mpn_rootrem(rootp,remp,ap,an,nth) -- Compute the nth root of {ap,an}, and
   store the truncated integer part at rootp and the remainder at remp.

   Contributed by Paul Zimmermann (algorithm) and
   Paul Zimmermann and Torbjorn Granlund (implementation).

   THE FUNCTIONS IN THIS FILE ARE INTERNAL, AND HAVE MUTABLE INTERFACES.  IT'S
   ONLY SAFE TO REACH THEM THROUGH DOCUMENTED INTERFACES.  IN FACT, IT'S ALMOST
   GUARANTEED THAT THEY'LL CHANGE OR DISAPPEAR IN A FUTURE GNU MP RELEASE.

Copyright 2002, 2005, 2009, 2010, 2011, 2012 Free Software Foundation, Inc.

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

/* FIXME:
     This implementation is not optimal when remp == NULL, since the complexity
     is M(n), whereas it should be M(n/k) on average.
*/

#include <stdio.h>		/* for NULL */

#include "gmp.h"
#include "gmp-impl.h"
#include "longlong.h"

static mp_size_t mpn_rootrem_internal (mp_ptr, mp_ptr, mp_srcptr, mp_size_t,
				       mp_limb_t, int);

#define MPN_RSHIFT(cy,rp,up,un,cnt) \
  do {									\
    if ((cnt) != 0)							\
      cy = mpn_rshift (rp, up, un, cnt);				\
    else								\
      {									\
	MPN_COPY_INCR (rp, up, un);					\
	cy = 0;								\
      }									\
  } while (0)

#define MPN_LSHIFT(cy,rp,up,un,cnt) \
  do {									\
    if ((cnt) != 0)							\
      cy = mpn_lshift (rp, up, un, cnt);				\
    else								\
      {									\
	MPN_COPY_DECR (rp, up, un);					\
	cy = 0;								\
      }									\
  } while (0)


/* Put in {rootp, ceil(un/k)} the kth root of {up, un}, rounded toward zero.
   If remp <> NULL, put in {remp, un} the remainder.
   Return the size (in limbs) of the remainder if remp <> NULL,
	  or a non-zero value iff the remainder is non-zero when remp = NULL.
   Assumes:
   (a) up[un-1] is not zero
   (b) rootp has at least space for ceil(un/k) limbs
   (c) remp has at least space for un limbs (in case remp <> NULL)
   (d) the operands do not overlap.

   The auxiliary memory usage is 3*un+2 if remp = NULL,
   and 2*un+2 if remp <> NULL.  FIXME: This is an incorrect comment.
*/
mp_size_t
mpn_rootrem (mp_ptr rootp, mp_ptr remp,
	     mp_srcptr up, mp_size_t un, mp_limb_t k)
{
  mp_size_t m;
  ASSERT (un > 0);
  ASSERT (up[un - 1] != 0);
  ASSERT (k > 1);

  m = (un - 1) / k;		/* ceil(un/k) - 1 */
  if (remp == NULL && m > 2)
    /* Pad {up,un} with k zero limbs.  This will produce an approximate root
       with one more limb, allowing us to compute the exact integral result. */
    {
      mp_ptr sp, wp;
      mp_size_t rn, sn, wn;
      TMP_DECL;
      TMP_MARK;
      wn = un + k;
      wp = TMP_ALLOC_LIMBS (wn); /* will contain the padded input */
      sn = m + 2; /* ceil(un/k) + 1 */
      sp = TMP_ALLOC_LIMBS (sn); /* approximate root of padded input */
      MPN_COPY (wp + k, up, un);
      MPN_ZERO (wp, k);
      rn = mpn_rootrem_internal (sp, NULL, wp, wn, k, 1);
      /* The approximate root S = {sp,sn} is either the correct root of
	 {sp,sn}, or 1 too large.  Thus unless the least significant limb of
	 S is 0 or 1, we can deduce the root of {up,un} is S truncated by one
	 limb.  (In case sp[0]=1, we can deduce the root, but not decide
	 whether it is exact or not.) */
      MPN_COPY (rootp, sp + 1, sn - 1);
      TMP_FREE;
      return rn;
    }
  else
    {
      return mpn_rootrem_internal (rootp, remp, up, un, k, 0);
    }
}

/* if approx is non-zero, does not compute the final remainder */
static mp_size_t
mpn_rootrem_internal (mp_ptr rootp, mp_ptr remp, mp_srcptr up, mp_size_t un,
		      mp_limb_t k, int approx)
{
  mp_ptr qp, rp, sp, wp, scratch;
  mp_size_t qn, rn, sn, wn, nl, bn;
  mp_limb_t save, save2, cy;
  unsigned long int unb; /* number of significant bits of {up,un} */
  unsigned long int xnb; /* number of significant bits of the result */
  unsigned long b, kk;
  unsigned long sizes[GMP_NUMB_BITS + 1];
  int ni, i;
  int c;
  int logk;
  TMP_DECL;

  TMP_MARK;

  if (remp == NULL)
    {
      rp = TMP_ALLOC_LIMBS (un + 1);     /* will contain the remainder */
      scratch = rp;			 /* used by mpn_div_q */
    }
  else
    {
      scratch = TMP_ALLOC_LIMBS (un + 1); /* used by mpn_div_q */
      rp = remp;
    }
  sp = rootp;

  MPN_SIZEINBASE_2EXP(unb, up, un, 1);
  /* unb is the number of bits of the input U */

  xnb = (unb - 1) / k + 1;	/* ceil (unb / k) */
  /* xnb is the number of bits of the root R */

  if (xnb == 1) /* root is 1 */
    {
      if (remp == NULL)
	remp = rp;
      mpn_sub_1 (remp, up, un, (mp_limb_t) 1);
      MPN_NORMALIZE (remp, un);	/* There should be at most one zero limb,
				   if we demand u to be normalized  */
      rootp[0] = 1;
      TMP_FREE;
      return un;
    }

  /* We initialize the algorithm with a 1-bit approximation to zero: since we
     know the root has exactly xnb bits, we write r0 = 2^(xnb-1), so that
     r0^k = 2^(k*(xnb-1)), that we subtract to the input. */
  kk = k * (xnb - 1);		/* number of truncated bits in the input */
  rn = un - kk / GMP_NUMB_BITS; /* number of limbs of the non-truncated part */
  MPN_RSHIFT (cy, rp, up + kk / GMP_NUMB_BITS, rn, kk % GMP_NUMB_BITS);
  mpn_sub_1 (rp, rp, rn, 1);	/* subtract the initial approximation: since
				   the non-truncated part is less than 2^k, it
				   is <= k bits: rn <= ceil(k/GMP_NUMB_BITS) */
  sp[0] = 1;			/* initial approximation */
  sn = 1;			/* it has one limb */

  for (logk = 1; ((k - 1) >> logk) != 0; logk++)
    ;
  /* logk = ceil(log(k)/log(2)) */

  b = xnb - 1; /* number of remaining bits to determine in the kth root */
  ni = 0;
  while (b != 0)
    {
      /* invariant: here we want b+1 total bits for the kth root */
      sizes[ni] = b;
      /* if c is the new value of b, this means that we'll go from a root
	 of c+1 bits (say s') to a root of b+1 bits.
	 It is proved in the book "Modern Computer Arithmetic" from Brent
	 and Zimmermann, Chapter 1, that
	 if s' >= k*beta, then at most one correction is necessary.
	 Here beta = 2^(b-c), and s' >= 2^c, thus it suffices that
	 c >= ceil((b + log2(k))/2). */
      b = (b + logk + 1) / 2;
      if (b >= sizes[ni])
	b = sizes[ni] - 1;	/* add just one bit at a time */
      ni++;
    }
  sizes[ni] = 0;
  ASSERT_ALWAYS (ni < GMP_NUMB_BITS + 1);
  /* We have sizes[0] = b > sizes[1] > ... > sizes[ni] = 0 with
     sizes[i] <= 2 * sizes[i+1].
     Newton iteration will first compute sizes[ni-1] extra bits,
     then sizes[ni-2], ..., then sizes[0] = b. */

  /* qp and wp need enough space to store S'^k where S' is an approximate
     root. Since S' can be as large as S+2, the worst case is when S=2 and
     S'=4. But then since we know the number of bits of S in advance, S'
     can only be 3 at most. Similarly for S=4, then S' can be 6 at most.
     So the worst case is S'/S=3/2, thus S'^k <= (3/2)^k * S^k. Since S^k
     fits in un limbs, the number of extra limbs needed is bounded by
     ceil(k*log2(3/2)/GMP_NUMB_BITS). */
#define EXTRA 2 + (mp_size_t) (0.585 * (double) k / (double) GMP_NUMB_BITS)
  qp = TMP_ALLOC_LIMBS (un + EXTRA); /* will contain quotient and remainder
					of R/(k*S^(k-1)), and S^k */
  wp = TMP_ALLOC_LIMBS (un + EXTRA); /* will contain S^(k-1), k*S^(k-1),
					and temporary for mpn_pow_1 */

  wp[0] = 1; /* {sp,sn}^(k-1) = 1 */
  wn = 1;
  for (i = ni; i != 0; i--)
    {
      /* 1: loop invariant:
	 {sp, sn} is the current approximation of the root, which has
		  exactly 1 + sizes[ni] bits.
	 {rp, rn} is the current remainder
	 {wp, wn} = {sp, sn}^(k-1)
	 kk = number of truncated bits of the input
      */
      b = sizes[i - 1] - sizes[i]; /* number of bits to compute in that
				      iteration */

      /* Reinsert a low zero limb if we normalized away the entire remainder */
      if (rn == 0)
	{
	  rp[0] = 0;
	  rn = 1;
	}

      /* first multiply the remainder by 2^b */
      MPN_LSHIFT (cy, rp + b / GMP_NUMB_BITS, rp, rn, b % GMP_NUMB_BITS);
      rn = rn + b / GMP_NUMB_BITS;
      if (cy != 0)
	{
	  rp[rn] = cy;
	  rn++;
	}

      kk = kk - b;

      /* 2: current buffers: {sp,sn}, {rp,rn}, {wp,wn} */

      /* Now insert bits [kk,kk+b-1] from the input U */
      bn = b / GMP_NUMB_BITS; /* lowest limb from high part of rp[] */
      save = rp[bn];
      /* nl is the number of limbs in U which contain bits [kk,kk+b-1] */
      nl = 1 + (kk + b - 1) / GMP_NUMB_BITS - (kk / GMP_NUMB_BITS);
      /* nl  = 1 + floor((kk + b - 1) / GMP_NUMB_BITS)
		 - floor(kk / GMP_NUMB_BITS)
	     <= 1 + (kk + b - 1) / GMP_NUMB_BITS
		  - (kk - GMP_NUMB_BITS + 1) / GMP_NUMB_BITS
	     = 2 + (b - 2) / GMP_NUMB_BITS
	 thus since nl is an integer:
	 nl <= 2 + floor(b/GMP_NUMB_BITS) <= 2 + bn. */
      /* we have to save rp[bn] up to rp[nl-1], i.e. 1 or 2 limbs */
      if (nl - 1 > bn)
	save2 = rp[bn + 1];
      MPN_RSHIFT (cy, rp, up + kk / GMP_NUMB_BITS, nl, kk % GMP_NUMB_BITS);
      /* set to zero high bits of rp[bn] */
      rp[bn] &= ((mp_limb_t) 1 << (b % GMP_NUMB_BITS)) - 1;
      /* restore corresponding bits */
      rp[bn] |= save;
      if (nl - 1 > bn)
	rp[bn + 1] = save2; /* the low b bits go in rp[0..bn] only, since
			       they start by bit 0 in rp[0], so they use
			       at most ceil(b/GMP_NUMB_BITS) limbs */

      /* 3: current buffers: {sp,sn}, {rp,rn}, {wp,wn} */

      /* compute {wp, wn} = k * {sp, sn}^(k-1) */
      cy = mpn_mul_1 (wp, wp, wn, k);
      wp[wn] = cy;
      wn += cy != 0;

      /* 4: current buffers: {sp,sn}, {rp,rn}, {wp,wn} */

      /* now divide {rp, rn} by {wp, wn} to get the low part of the root */
      if (rn < wn)
	{
	  qn = 0;
	}
      else
	{
	  qn = rn - wn; /* expected quotient size */
	  mpn_div_q (qp, rp, rn, wp, wn, scratch);
	  qn += qp[qn] != 0;
	}

      /* 5: current buffers: {sp,sn}, {qp,qn}.
	 Note: {rp,rn} is not needed any more since we'll compute it from
	 scratch at the end of the loop.
       */

      /* Number of limbs used by b bits, when least significant bit is
	 aligned to least limb */
      bn = (b - 1) / GMP_NUMB_BITS + 1;

      /* the quotient should be smaller than 2^b, since the previous
	 approximation was correctly rounded toward zero */
      if (qn > bn || (qn == bn && (b % GMP_NUMB_BITS != 0) &&
		      qp[qn - 1] >= ((mp_limb_t) 1 << (b % GMP_NUMB_BITS))))
	{
	  qn = b / GMP_NUMB_BITS + 1; /* b+1 bits */
	  MPN_ZERO (qp, qn);
	  qp[qn - 1] = (mp_limb_t) 1 << (b % GMP_NUMB_BITS);
	  MPN_DECR_U (qp, qn, 1);
	  qn -= qp[qn - 1] == 0;
	}

      /* 6: current buffers: {sp,sn}, {qp,qn} */

      /* multiply the root approximation by 2^b */
      MPN_LSHIFT (cy, sp + b / GMP_NUMB_BITS, sp, sn, b % GMP_NUMB_BITS);
      sn = sn + b / GMP_NUMB_BITS;
      if (cy != 0)
	{
	  sp[sn] = cy;
	  sn++;
	}

      /* 7: current buffers: {sp,sn}, {qp,qn} */

      ASSERT_ALWAYS (bn >= qn); /* this is ok since in the case qn > bn
				   above, q is set to 2^b-1, which has
				   exactly bn limbs */

      /* Combine sB and q to form sB + q.  */
      save = sp[b / GMP_NUMB_BITS];
      MPN_COPY (sp, qp, qn);
      MPN_ZERO (sp + qn, bn - qn);
      sp[b / GMP_NUMB_BITS] |= save;

      /* 8: current buffer: {sp,sn} */

      /* Since each iteration treats b bits from the root and thus k*b bits
	 from the input, and we already considered b bits from the input,
	 we now have to take another (k-1)*b bits from the input. */
      kk -= (k - 1) * b; /* remaining input bits */
      /* {rp, rn} = floor({up, un} / 2^kk) */
      MPN_RSHIFT (cy, rp, up + kk / GMP_NUMB_BITS, un - kk / GMP_NUMB_BITS, kk % GMP_NUMB_BITS);
      rn = un - kk / GMP_NUMB_BITS;
      rn -= rp[rn - 1] == 0;

      /* 9: current buffers: {sp,sn}, {rp,rn} */

     for (c = 0;; c++)
	{
	  /* Compute S^k in {qp,qn}. */
	  if (i == 1)
	    {
	      /* Last iteration: we don't need W anymore. */
	      /* mpn_pow_1 requires that both qp and wp have enough space to
		 store the result {sp,sn}^k + 1 limb */
	      approx = approx && (sp[0] > 1);
	      qn = (approx == 0) ? mpn_pow_1 (qp, sp, sn, k, wp) : 0;
	    }
	  else
	    {
	      /* W <- S^(k-1) for the next iteration,
		 and S^k = W * S. */
	      wn = mpn_pow_1 (wp, sp, sn, k - 1, qp);
	      mpn_mul (qp, wp, wn, sp, sn);
	      qn = wn + sn;
	      qn -= qp[qn - 1] == 0;
	    }

	  /* if S^k > floor(U/2^kk), the root approximation was too large */
	  if (qn > rn || (qn == rn && mpn_cmp (qp, rp, rn) > 0))
	    MPN_DECR_U (sp, sn, 1);
	  else
	    break;
	}

      /* 10: current buffers: {sp,sn}, {rp,rn}, {qp,qn}, {wp,wn} */

      ASSERT_ALWAYS (c <= 1);
      ASSERT_ALWAYS (rn >= qn);

      /* R = R - Q = floor(U/2^kk) - S^k */
      if (i > 1 || approx == 0)
	{
	  mpn_sub (rp, rp, rn, qp, qn);
	  MPN_NORMALIZE (rp, rn);
	}
      /* otherwise we have rn > 0, thus the return value is ok */

      /* 11: current buffers: {sp,sn}, {rp,rn}, {wp,wn} */
    }

  TMP_FREE;
  return rn;
}
