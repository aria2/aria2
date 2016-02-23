/* mpf_ui_sub -- Subtract a float from an unsigned long int.

Copyright 1993, 1994, 1995, 1996, 2001, 2002, 2005 Free Software Foundation,
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

void
mpf_ui_sub (mpf_ptr r, unsigned long int u, mpf_srcptr v)
{
  mp_srcptr up, vp;
  mp_ptr rp, tp;
  mp_size_t usize, vsize, rsize;
  mp_size_t prec;
  mp_exp_t uexp;
  mp_size_t ediff;
  int negate;
  mp_limb_t ulimb;
  TMP_DECL;

  vsize = v->_mp_size;

  /* Handle special cases that don't work in generic code below.  */
  if (u == 0)
    {
      mpf_neg (r, v);
      return;
    }
  if (vsize == 0)
    {
      mpf_set_ui (r, u);
      return;
    }

  /* If signs of U and V are different, perform addition.  */
  if (vsize < 0)
    {
      __mpf_struct v_negated;
      v_negated._mp_size = -vsize;
      v_negated._mp_exp = v->_mp_exp;
      v_negated._mp_d = v->_mp_d;
      mpf_add_ui (r, &v_negated, u);
      return;
    }

  TMP_MARK;

  /* Signs are now known to be the same.  */

  ulimb = u;
  /* Make U be the operand with the largest exponent.  */
  if (1 < v->_mp_exp)
    {
      negate = 1;
      usize = ABS (vsize);
      vsize = 1;
      up = v->_mp_d;
      vp = &ulimb;
      rp = r->_mp_d;
      prec = r->_mp_prec + 1;
      uexp = v->_mp_exp;
      ediff = uexp - 1;
    }
  else
    {
      negate = 0;
      usize = 1;
      vsize = ABS (vsize);
      up = &ulimb;
      vp = v->_mp_d;
      rp = r->_mp_d;
      prec = r->_mp_prec;
      uexp = 1;
      ediff = 1 - v->_mp_exp;
    }

  /* Ignore leading limbs in U and V that are equal.  Doing
     this helps increase the precision of the result.  */
  if (ediff == 0)
    {
      /* This loop normally exits immediately.  Optimize for that.  */
      for (;;)
	{
	  usize--;
	  vsize--;
	  if (up[usize] != vp[vsize])
	    break;
	  uexp--;
	  if (usize == 0)
	    goto Lu0;
	  if (vsize == 0)
	    goto Lv0;
	}
      usize++;
      vsize++;
      /* Note that either operand (but not both operands) might now have
	 leading zero limbs.  It matters only that U is unnormalized if
	 vsize is now zero, and vice versa.  And it is only in that case
	 that we have to adjust uexp.  */
      if (vsize == 0)
      Lv0:
	while (usize != 0 && up[usize - 1] == 0)
	  usize--, uexp--;
      if (usize == 0)
      Lu0:
	while (vsize != 0 && vp[vsize - 1] == 0)
	  vsize--, uexp--;
    }

  /* If U extends beyond PREC, ignore the part that does.  */
  if (usize > prec)
    {
      up += usize - prec;
      usize = prec;
    }

  /* If V extends beyond PREC, ignore the part that does.
     Note that this may make vsize negative.  */
  if (vsize + ediff > prec)
    {
      vp += vsize + ediff - prec;
      vsize = prec - ediff;
    }

  /* Allocate temp space for the result.  Allocate
     just vsize + ediff later???  */
  tp = TMP_ALLOC_LIMBS (prec);

  if (ediff >= prec)
    {
      /* V completely cancelled.  */
      if (tp != up)
	MPN_COPY (rp, up, usize);
      rsize = usize;
    }
  else
    {
      /* Locate the least significant non-zero limb in (the needed
	 parts of) U and V, to simplify the code below.  */
      for (;;)
	{
	  if (vsize == 0)
	    {
	      MPN_COPY (rp, up, usize);
	      rsize = usize;
	      goto done;
	    }
	  if (vp[0] != 0)
	    break;
	  vp++, vsize--;
	}
      for (;;)
	{
	  if (usize == 0)
	    {
	      MPN_COPY (rp, vp, vsize);
	      rsize = vsize;
	      negate ^= 1;
	      goto done;
	    }
	  if (up[0] != 0)
	    break;
	  up++, usize--;
	}

      /* uuuu     |  uuuu     |  uuuu     |  uuuu     |  uuuu    */
      /* vvvvvvv  |  vv       |    vvvvv  |    v      |       vv */

      if (usize > ediff)
	{
	  /* U and V partially overlaps.  */
	  if (ediff == 0)
	    {
	      /* Have to compare the leading limbs of u and v
		 to determine whether to compute u - v or v - u.  */
	      if (usize > vsize)
		{
		  /* uuuu     */
		  /* vv       */
		  int cmp;
		  cmp = mpn_cmp (up + usize - vsize, vp, vsize);
		  if (cmp >= 0)
		    {
		      mp_size_t size;
		      size = usize - vsize;
		      MPN_COPY (tp, up, size);
		      mpn_sub_n (tp + size, up + size, vp, vsize);
		      rsize = usize;
		    }
		  else
		    {
		      /* vv       */  /* Swap U and V. */
		      /* uuuu     */
		      mp_size_t size, i;
		      size = usize - vsize;
		      tp[0] = -up[0] & GMP_NUMB_MASK;
		      for (i = 1; i < size; i++)
			tp[i] = ~up[i] & GMP_NUMB_MASK;
		      mpn_sub_n (tp + size, vp, up + size, vsize);
		      mpn_sub_1 (tp + size, tp + size, vsize, (mp_limb_t) 1);
		      negate ^= 1;
		      rsize = usize;
		    }
		}
	      else if (usize < vsize)
		{
		  /* uuuu     */
		  /* vvvvvvv  */
		  int cmp;
		  cmp = mpn_cmp (up, vp + vsize - usize, usize);
		  if (cmp > 0)
		    {
		      mp_size_t size, i;
		      size = vsize - usize;
		      tp[0] = -vp[0] & GMP_NUMB_MASK;
		      for (i = 1; i < size; i++)
			tp[i] = ~vp[i] & GMP_NUMB_MASK;
		      mpn_sub_n (tp + size, up, vp + size, usize);
		      mpn_sub_1 (tp + size, tp + size, usize, (mp_limb_t) 1);
		      rsize = vsize;
		    }
		  else
		    {
		      /* vvvvvvv  */  /* Swap U and V. */
		      /* uuuu     */
		      /* This is the only place we can get 0.0.  */
		      mp_size_t size;
		      size = vsize - usize;
		      MPN_COPY (tp, vp, size);
		      mpn_sub_n (tp + size, vp + size, up, usize);
		      negate ^= 1;
		      rsize = vsize;
		    }
		}
	      else
		{
		  /* uuuu     */
		  /* vvvv     */
		  int cmp;
		  cmp = mpn_cmp (up, vp + vsize - usize, usize);
		  if (cmp > 0)
		    {
		      mpn_sub_n (tp, up, vp, usize);
		      rsize = usize;
		    }
		  else
		    {
		      mpn_sub_n (tp, vp, up, usize);
		      negate ^= 1;
		      rsize = usize;
		      /* can give zero */
		    }
		}
	    }
	  else
	    {
	      if (vsize + ediff <= usize)
		{
		  /* uuuu     */
		  /*   v      */
		  mp_size_t size;
		  size = usize - ediff - vsize;
		  MPN_COPY (tp, up, size);
		  mpn_sub (tp + size, up + size, usize - size, vp, vsize);
		  rsize = usize;
		}
	      else
		{
		  /* uuuu     */
		  /*   vvvvv  */
		  mp_size_t size, i;
		  size = vsize + ediff - usize;
		  tp[0] = -vp[0] & GMP_NUMB_MASK;
		  for (i = 1; i < size; i++)
		    tp[i] = ~vp[i] & GMP_NUMB_MASK;
		  mpn_sub (tp + size, up, usize, vp + size, usize - ediff);
		  mpn_sub_1 (tp + size, tp + size, usize, (mp_limb_t) 1);
		  rsize = vsize + ediff;
		}
	    }
	}
      else
	{
	  /* uuuu     */
	  /*      vv  */
	  mp_size_t size, i;
	  size = vsize + ediff - usize;
	  tp[0] = -vp[0] & GMP_NUMB_MASK;
	  for (i = 1; i < vsize; i++)
	    tp[i] = ~vp[i] & GMP_NUMB_MASK;
	  for (i = vsize; i < size; i++)
	    tp[i] = GMP_NUMB_MAX;
	  mpn_sub_1 (tp + size, up, usize, (mp_limb_t) 1);
	  rsize = size + usize;
	}

      /* Full normalize.  Optimize later.  */
      while (rsize != 0 && tp[rsize - 1] == 0)
	{
	  rsize--;
	  uexp--;
	}
      MPN_COPY (rp, tp, rsize);
    }

 done:
  r->_mp_size = negate ? -rsize : rsize;
  r->_mp_exp = uexp;
  TMP_FREE;
}
