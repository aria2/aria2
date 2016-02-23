/* mpz_fdiv_ui -- Division rounding the quotient towards -infinity.
   The remainder gets the same sign as the denominator.

Copyright 1994, 1995, 1996, 2001, 2002, 2004, 2005, 2012 Free Software
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

#include "gmp.h"
#include "gmp-impl.h"

unsigned long int
mpz_fdiv_ui (mpz_srcptr dividend, unsigned long int divisor)
{
  mp_size_t ns, nn;
  mp_ptr np;
  mp_limb_t rl;

  if (UNLIKELY (divisor == 0))
    DIVIDE_BY_ZERO;

  ns = SIZ(dividend);
  if (ns == 0)
    {
      return 0;
    }

  nn = ABS(ns);
  np = PTR(dividend);
#if BITS_PER_ULONG > GMP_NUMB_BITS  /* avoid warnings about shift amount */
  if (divisor > GMP_NUMB_MAX)
    {
      mp_limb_t dp[2], rp[2];
      mp_ptr qp;
      mp_size_t rn;
      TMP_DECL;

      if (nn == 1)		/* tdiv_qr requirements; tested above for 0 */
	{
	  rl = np[0];
	  rp[0] = rl;
	}
      else
	{
	  TMP_MARK;
	  dp[0] = divisor & GMP_NUMB_MASK;
	  dp[1] = divisor >> GMP_NUMB_BITS;
	  qp = TMP_ALLOC_LIMBS (nn - 2 + 1);
	  mpn_tdiv_qr (qp, rp, (mp_size_t) 0, np, nn, dp, (mp_size_t) 2);
	  TMP_FREE;
	  rl = rp[0] + (rp[1] << GMP_NUMB_BITS);
	}

      if (rl != 0 && ns < 0)
	{
	  rl = divisor - rl;
	  rp[0] = rl & GMP_NUMB_MASK;
	  rp[1] = rl >> GMP_NUMB_BITS;
	}

      rn = 1 + (rl > GMP_NUMB_MAX);  rn -= (rp[rn - 1] == 0);
    }
  else
#endif
    {
      rl = mpn_mod_1 (np, nn, (mp_limb_t) divisor);
      if (rl == 0)
	;
      else
	{
	  if (ns < 0)
	    rl = divisor - rl;
	}
    }

  return rl;
}
