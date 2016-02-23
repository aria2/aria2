/* mpn_sqrtrem -- square root and remainder

   Contributed to the GNU project by Paul Zimmermann (most code) and
   Torbjorn Granlund (mpn_sqrtrem1).

   THE FUNCTIONS IN THIS FILE EXCEPT mpn_sqrtrem ARE INTERNAL WITH A
   MUTABLE INTERFACE.  IT IS ONLY SAFE TO REACH THEM THROUGH DOCUMENTED
   INTERFACES.  IN FACT, IT IS ALMOST GUARANTEED THAT THEY WILL CHANGE OR
   DISAPPEAR IN A FUTURE GMP RELEASE.

Copyright 1999, 2000, 2001, 2002, 2004, 2005, 2008, 2010, 2012 Free Software
Foundation, Inc.

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


/* See "Karatsuba Square Root", reference in gmp.texi.  */


#include <stdio.h>
#include <stdlib.h>

#include "gmp.h"
#include "gmp-impl.h"
#include "longlong.h"

static const unsigned char invsqrttab[384] = /* The common 0x100 was removed */
{
  0xff,0xfd,0xfb,0xf9,0xf7,0xf5,0xf3,0xf2, /* sqrt(1/80)..sqrt(1/87) */
  0xf0,0xee,0xec,0xea,0xe9,0xe7,0xe5,0xe4, /* sqrt(1/88)..sqrt(1/8f) */
  0xe2,0xe0,0xdf,0xdd,0xdb,0xda,0xd8,0xd7, /* sqrt(1/90)..sqrt(1/97) */
  0xd5,0xd4,0xd2,0xd1,0xcf,0xce,0xcc,0xcb, /* sqrt(1/98)..sqrt(1/9f) */
  0xc9,0xc8,0xc6,0xc5,0xc4,0xc2,0xc1,0xc0, /* sqrt(1/a0)..sqrt(1/a7) */
  0xbe,0xbd,0xbc,0xba,0xb9,0xb8,0xb7,0xb5, /* sqrt(1/a8)..sqrt(1/af) */
  0xb4,0xb3,0xb2,0xb0,0xaf,0xae,0xad,0xac, /* sqrt(1/b0)..sqrt(1/b7) */
  0xaa,0xa9,0xa8,0xa7,0xa6,0xa5,0xa4,0xa3, /* sqrt(1/b8)..sqrt(1/bf) */
  0xa2,0xa0,0x9f,0x9e,0x9d,0x9c,0x9b,0x9a, /* sqrt(1/c0)..sqrt(1/c7) */
  0x99,0x98,0x97,0x96,0x95,0x94,0x93,0x92, /* sqrt(1/c8)..sqrt(1/cf) */
  0x91,0x90,0x8f,0x8e,0x8d,0x8c,0x8c,0x8b, /* sqrt(1/d0)..sqrt(1/d7) */
  0x8a,0x89,0x88,0x87,0x86,0x85,0x84,0x83, /* sqrt(1/d8)..sqrt(1/df) */
  0x83,0x82,0x81,0x80,0x7f,0x7e,0x7e,0x7d, /* sqrt(1/e0)..sqrt(1/e7) */
  0x7c,0x7b,0x7a,0x79,0x79,0x78,0x77,0x76, /* sqrt(1/e8)..sqrt(1/ef) */
  0x76,0x75,0x74,0x73,0x72,0x72,0x71,0x70, /* sqrt(1/f0)..sqrt(1/f7) */
  0x6f,0x6f,0x6e,0x6d,0x6d,0x6c,0x6b,0x6a, /* sqrt(1/f8)..sqrt(1/ff) */
  0x6a,0x69,0x68,0x68,0x67,0x66,0x66,0x65, /* sqrt(1/100)..sqrt(1/107) */
  0x64,0x64,0x63,0x62,0x62,0x61,0x60,0x60, /* sqrt(1/108)..sqrt(1/10f) */
  0x5f,0x5e,0x5e,0x5d,0x5c,0x5c,0x5b,0x5a, /* sqrt(1/110)..sqrt(1/117) */
  0x5a,0x59,0x59,0x58,0x57,0x57,0x56,0x56, /* sqrt(1/118)..sqrt(1/11f) */
  0x55,0x54,0x54,0x53,0x53,0x52,0x52,0x51, /* sqrt(1/120)..sqrt(1/127) */
  0x50,0x50,0x4f,0x4f,0x4e,0x4e,0x4d,0x4d, /* sqrt(1/128)..sqrt(1/12f) */
  0x4c,0x4b,0x4b,0x4a,0x4a,0x49,0x49,0x48, /* sqrt(1/130)..sqrt(1/137) */
  0x48,0x47,0x47,0x46,0x46,0x45,0x45,0x44, /* sqrt(1/138)..sqrt(1/13f) */
  0x44,0x43,0x43,0x42,0x42,0x41,0x41,0x40, /* sqrt(1/140)..sqrt(1/147) */
  0x40,0x3f,0x3f,0x3e,0x3e,0x3d,0x3d,0x3c, /* sqrt(1/148)..sqrt(1/14f) */
  0x3c,0x3b,0x3b,0x3a,0x3a,0x39,0x39,0x39, /* sqrt(1/150)..sqrt(1/157) */
  0x38,0x38,0x37,0x37,0x36,0x36,0x35,0x35, /* sqrt(1/158)..sqrt(1/15f) */
  0x35,0x34,0x34,0x33,0x33,0x32,0x32,0x32, /* sqrt(1/160)..sqrt(1/167) */
  0x31,0x31,0x30,0x30,0x2f,0x2f,0x2f,0x2e, /* sqrt(1/168)..sqrt(1/16f) */
  0x2e,0x2d,0x2d,0x2d,0x2c,0x2c,0x2b,0x2b, /* sqrt(1/170)..sqrt(1/177) */
  0x2b,0x2a,0x2a,0x29,0x29,0x29,0x28,0x28, /* sqrt(1/178)..sqrt(1/17f) */
  0x27,0x27,0x27,0x26,0x26,0x26,0x25,0x25, /* sqrt(1/180)..sqrt(1/187) */
  0x24,0x24,0x24,0x23,0x23,0x23,0x22,0x22, /* sqrt(1/188)..sqrt(1/18f) */
  0x21,0x21,0x21,0x20,0x20,0x20,0x1f,0x1f, /* sqrt(1/190)..sqrt(1/197) */
  0x1f,0x1e,0x1e,0x1e,0x1d,0x1d,0x1d,0x1c, /* sqrt(1/198)..sqrt(1/19f) */
  0x1c,0x1b,0x1b,0x1b,0x1a,0x1a,0x1a,0x19, /* sqrt(1/1a0)..sqrt(1/1a7) */
  0x19,0x19,0x18,0x18,0x18,0x18,0x17,0x17, /* sqrt(1/1a8)..sqrt(1/1af) */
  0x17,0x16,0x16,0x16,0x15,0x15,0x15,0x14, /* sqrt(1/1b0)..sqrt(1/1b7) */
  0x14,0x14,0x13,0x13,0x13,0x12,0x12,0x12, /* sqrt(1/1b8)..sqrt(1/1bf) */
  0x12,0x11,0x11,0x11,0x10,0x10,0x10,0x0f, /* sqrt(1/1c0)..sqrt(1/1c7) */
  0x0f,0x0f,0x0f,0x0e,0x0e,0x0e,0x0d,0x0d, /* sqrt(1/1c8)..sqrt(1/1cf) */
  0x0d,0x0c,0x0c,0x0c,0x0c,0x0b,0x0b,0x0b, /* sqrt(1/1d0)..sqrt(1/1d7) */
  0x0a,0x0a,0x0a,0x0a,0x09,0x09,0x09,0x09, /* sqrt(1/1d8)..sqrt(1/1df) */
  0x08,0x08,0x08,0x07,0x07,0x07,0x07,0x06, /* sqrt(1/1e0)..sqrt(1/1e7) */
  0x06,0x06,0x06,0x05,0x05,0x05,0x04,0x04, /* sqrt(1/1e8)..sqrt(1/1ef) */
  0x04,0x04,0x03,0x03,0x03,0x03,0x02,0x02, /* sqrt(1/1f0)..sqrt(1/1f7) */
  0x02,0x02,0x01,0x01,0x01,0x01,0x00,0x00  /* sqrt(1/1f8)..sqrt(1/1ff) */
};

/* Compute s = floor(sqrt(a0)), and *rp = a0 - s^2.  */

#if GMP_NUMB_BITS > 32
#define MAGIC CNST_LIMB(0x10000000000)	/* 0xffe7debbfc < MAGIC < 0x232b1850f410 */
#else
#define MAGIC CNST_LIMB(0x100000)		/* 0xfee6f < MAGIC < 0x29cbc8 */
#endif

static mp_limb_t
mpn_sqrtrem1 (mp_ptr rp, mp_limb_t a0)
{
#if GMP_NUMB_BITS > 32
  mp_limb_t a1;
#endif
  mp_limb_t x0, t2, t, x2;
  unsigned abits;

  ASSERT_ALWAYS (GMP_NAIL_BITS == 0);
  ASSERT_ALWAYS (GMP_LIMB_BITS == 32 || GMP_LIMB_BITS == 64);
  ASSERT (a0 >= GMP_NUMB_HIGHBIT / 2);

  /* Use Newton iterations for approximating 1/sqrt(a) instead of sqrt(a),
     since we can do the former without division.  As part of the last
     iteration convert from 1/sqrt(a) to sqrt(a).  */

  abits = a0 >> (GMP_LIMB_BITS - 1 - 8);	/* extract bits for table lookup */
  x0 = 0x100 | invsqrttab[abits - 0x80];	/* initial 1/sqrt(a) */

  /* x0 is now an 8 bits approximation of 1/sqrt(a0) */

#if GMP_NUMB_BITS > 32
  a1 = a0 >> (GMP_LIMB_BITS - 1 - 32);
  t = (mp_limb_signed_t) (CNST_LIMB(0x2000000000000) - 0x30000  - a1 * x0 * x0) >> 16;
  x0 = (x0 << 16) + ((mp_limb_signed_t) (x0 * t) >> (16+2));

  /* x0 is now a 16 bits approximation of 1/sqrt(a0) */

  t2 = x0 * (a0 >> (32-8));
  t = t2 >> 25;
  t = ((mp_limb_signed_t) ((a0 << 14) - t * t - MAGIC) >> (32-8));
  x0 = t2 + ((mp_limb_signed_t) (x0 * t) >> 15);
  x0 >>= 32;
#else
  t2 = x0 * (a0 >> (16-8));
  t = t2 >> 13;
  t = ((mp_limb_signed_t) ((a0 << 6) - t * t - MAGIC) >> (16-8));
  x0 = t2 + ((mp_limb_signed_t) (x0 * t) >> 7);
  x0 >>= 16;
#endif

  /* x0 is now a full limb approximation of sqrt(a0) */

  x2 = x0 * x0;
  if (x2 + 2*x0 <= a0 - 1)
    {
      x2 += 2*x0 + 1;
      x0++;
    }

  *rp = a0 - x2;
  return x0;
}


#define Prec (GMP_NUMB_BITS >> 1)

/* same as mpn_sqrtrem, but for size=2 and {np, 2} normalized
   return cc such that {np, 2} = sp[0]^2 + cc*2^GMP_NUMB_BITS + rp[0] */
static mp_limb_t
mpn_sqrtrem2 (mp_ptr sp, mp_ptr rp, mp_srcptr np)
{
  mp_limb_t qhl, q, u, np0, sp0, rp0, q2;
  int cc;

  ASSERT (np[1] >= GMP_NUMB_HIGHBIT / 2);

  np0 = np[0];
  sp0 = mpn_sqrtrem1 (rp, np[1]);
  qhl = 0;
  rp0 = rp[0];
  while (rp0 >= sp0)
    {
      qhl++;
      rp0 -= sp0;
    }
  /* now rp0 < sp0 < 2^Prec */
  rp0 = (rp0 << Prec) + (np0 >> Prec);
  u = 2 * sp0;
  q = rp0 / u;
  u = rp0 - q * u;
  q += (qhl & 1) << (Prec - 1);
  qhl >>= 1; /* if qhl=1, necessary q=0 as qhl*2^Prec + q <= 2^Prec */
  /* now we have (initial rp0)<<Prec + np0>>Prec = (qhl<<Prec + q) * (2sp0) + u */
  sp0 = ((sp0 + qhl) << Prec) + q;
  cc = u >> Prec;
  rp0 = ((u << Prec) & GMP_NUMB_MASK) + (np0 & (((mp_limb_t) 1 << Prec) - 1));
  /* subtract q * q or qhl*2^(2*Prec) from rp */
  q2 = q * q;
  cc -= (rp0 < q2) + qhl;
  rp0 -= q2;
  /* now subtract 2*q*2^Prec + 2^(2*Prec) if qhl is set */
  if (cc < 0)
    {
      if (sp0 != 0)
	{
	  rp0 += sp0;
	  cc += rp0 < sp0;
	}
      else
	cc++;
      --sp0;
      rp0 += sp0;
      cc += rp0 < sp0;
    }

  rp[0] = rp0;
  sp[0] = sp0;
  return cc;
}

/* writes in {sp, n} the square root (rounded towards zero) of {np, 2n},
   and in {np, n} the low n limbs of the remainder, returns the high
   limb of the remainder (which is 0 or 1).
   Assumes {np, 2n} is normalized, i.e. np[2n-1] >= B/4
   where B=2^GMP_NUMB_BITS.  */
static mp_limb_t
mpn_dc_sqrtrem (mp_ptr sp, mp_ptr np, mp_size_t n)
{
  mp_limb_t q;			/* carry out of {sp, n} */
  int c, b;			/* carry out of remainder */
  mp_size_t l, h;

  ASSERT (np[2 * n - 1] >= GMP_NUMB_HIGHBIT / 2);

  if (n == 1)
    c = mpn_sqrtrem2 (sp, np, np);
  else
    {
      l = n / 2;
      h = n - l;
      q = mpn_dc_sqrtrem (sp + l, np + 2 * l, h);
      if (q != 0)
	mpn_sub_n (np + 2 * l, np + 2 * l, sp + l, h);
      q += mpn_divrem (sp, 0, np + l, n, sp + l, h);
      c = sp[0] & 1;
      mpn_rshift (sp, sp, l, 1);
      sp[l - 1] |= (q << (GMP_NUMB_BITS - 1)) & GMP_NUMB_MASK;
      q >>= 1;
      if (c != 0)
	c = mpn_add_n (np + l, np + l, sp + l, h);
      mpn_sqr (np + n, sp, l);
      b = q + mpn_sub_n (np, np, np + n, 2 * l);
      c -= (l == h) ? b : mpn_sub_1 (np + 2 * l, np + 2 * l, 1, (mp_limb_t) b);
      q = mpn_add_1 (sp + l, sp + l, h, q);

      if (c < 0)
	{
#if HAVE_NATIVE_mpn_addlsh1_n
	  c += mpn_addlsh1_n (np, np, sp, n) + 2 * q;
#else
	  c += mpn_addmul_1 (np, sp, n, CNST_LIMB(2)) + 2 * q;
#endif
	  c -= mpn_sub_1 (np, np, n, CNST_LIMB(1));
	  q -= mpn_sub_1 (sp, sp, n, CNST_LIMB(1));
	}
    }

  return c;
}


mp_size_t
mpn_sqrtrem (mp_ptr sp, mp_ptr rp, mp_srcptr np, mp_size_t nn)
{
  mp_limb_t *tp, s0[1], cc, high, rl;
  int c;
  mp_size_t rn, tn;
  TMP_DECL;

  ASSERT (nn >= 0);
  ASSERT_MPN (np, nn);

  /* If OP is zero, both results are zero.  */
  if (nn == 0)
    return 0;

  ASSERT (np[nn - 1] != 0);
  ASSERT (rp == NULL || MPN_SAME_OR_SEPARATE_P (np, rp, nn));
  ASSERT (rp == NULL || ! MPN_OVERLAP_P (sp, (nn + 1) / 2, rp, nn));
  ASSERT (! MPN_OVERLAP_P (sp, (nn + 1) / 2, np, nn));

  high = np[nn - 1];
  if (nn == 1 && (high & GMP_NUMB_HIGHBIT))
    {
      mp_limb_t r;
      sp[0] = mpn_sqrtrem1 (&r, high);
      if (rp != NULL)
	rp[0] = r;
      return r != 0;
    }
  count_leading_zeros (c, high);
  c -= GMP_NAIL_BITS;

  c = c / 2; /* we have to shift left by 2c bits to normalize {np, nn} */
  tn = (nn + 1) / 2; /* 2*tn is the smallest even integer >= nn */

  TMP_MARK;
  if (nn % 2 != 0 || c > 0)
    {
      tp = TMP_ALLOC_LIMBS (2 * tn);
      tp[0] = 0;	     /* needed only when 2*tn > nn, but saves a test */
      if (c != 0)
	mpn_lshift (tp + 2 * tn - nn, np, nn, 2 * c);
      else
	MPN_COPY (tp + 2 * tn - nn, np, nn);
      rl = mpn_dc_sqrtrem (sp, tp, tn);
      /* We have 2^(2k)*N = S^2 + R where k = c + (2tn-nn)*GMP_NUMB_BITS/2,
	 thus 2^(2k)*N = (S-s0)^2 + 2*S*s0 - s0^2 + R where s0=S mod 2^k */
      c += (nn % 2) * GMP_NUMB_BITS / 2;		/* c now represents k */
      s0[0] = sp[0] & (((mp_limb_t) 1 << c) - 1);	/* S mod 2^k */
      rl += mpn_addmul_1 (tp, sp, tn, 2 * s0[0]);	/* R = R + 2*s0*S */
      cc = mpn_submul_1 (tp, s0, 1, s0[0]);
      rl -= (tn > 1) ? mpn_sub_1 (tp + 1, tp + 1, tn - 1, cc) : cc;
      mpn_rshift (sp, sp, tn, c);
      tp[tn] = rl;
      if (rp == NULL)
	rp = tp;
      c = c << 1;
      if (c < GMP_NUMB_BITS)
	tn++;
      else
	{
	  tp++;
	  c -= GMP_NUMB_BITS;
	}
      if (c != 0)
	mpn_rshift (rp, tp, tn, c);
      else
	MPN_COPY_INCR (rp, tp, tn);
      rn = tn;
    }
  else
    {
      if (rp == NULL)
	rp = TMP_ALLOC_LIMBS (nn);
      if (rp != np)
	MPN_COPY (rp, np, nn);
      rn = tn + (rp[tn] = mpn_dc_sqrtrem (sp, rp, tn));
    }

  MPN_NORMALIZE (rp, rn);

  TMP_FREE;
  return rn;
}
