/* mpf_sub -- Subtract two floats.

Copyright 1993, 1994, 1995, 1996, 1999, 2000, 2001, 2002, 2004, 2005, 2011 Free
Software Foundation, Inc.

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
mpf_sub (mpf_ptr r, mpf_srcptr u, mpf_srcptr v)
{
  mp_srcptr up, vp;
  mp_ptr rp, tp;
  mp_size_t usize, vsize, rsize;
  mp_size_t prec;
  mp_exp_t exp;
  mp_size_t ediff;
  int negate;
  TMP_DECL;

  usize = u->_mp_size;
  vsize = v->_mp_size;

  /* Handle special cases that don't work in generic code below.  */
  if (usize == 0)
    {
      mpf_neg (r, v);
      return;
    }
  if (vsize == 0)
    {
      if (r != u)
        mpf_set (r, u);
      return;
    }

  /* If signs of U and V are different, perform addition.  */
  if ((usize ^ vsize) < 0)
    {
      __mpf_struct v_negated;
      v_negated._mp_size = -vsize;
      v_negated._mp_exp = v->_mp_exp;
      v_negated._mp_d = v->_mp_d;
      mpf_add (r, u, &v_negated);
      return;
    }

  TMP_MARK;

  /* Signs are now known to be the same.  */
  negate = usize < 0;

  /* Make U be the operand with the largest exponent.  */
  if (u->_mp_exp < v->_mp_exp)
    {
      mpf_srcptr t;
      t = u; u = v; v = t;
      negate ^= 1;
      usize = u->_mp_size;
      vsize = v->_mp_size;
    }

  usize = ABS (usize);
  vsize = ABS (vsize);
  up = u->_mp_d;
  vp = v->_mp_d;
  rp = r->_mp_d;
  prec = r->_mp_prec + 1;
  exp = u->_mp_exp;
  ediff = u->_mp_exp - v->_mp_exp;

  /* If ediff is 0 or 1, we might have a situation where the operands are
     extremely close.  We need to scan the operands from the most significant
     end ignore the initial parts that are equal.  */
  if (ediff <= 1)
    {
      if (ediff == 0)
	{
	  /* Skip leading limbs in U and V that are equal.  */
	  if (up[usize - 1] == vp[vsize - 1])
	    {
	      /* This loop normally exits immediately.  Optimize for that.  */
	      do
		{
		  usize--;
		  vsize--;
		  exp--;

		  if (usize == 0)
		    {
                      /* u cancels high limbs of v, result is rest of v */
		      negate ^= 1;
                    cancellation:
                      /* strip high zeros before truncating to prec */
                      while (vsize != 0 && vp[vsize - 1] == 0)
                        {
                          vsize--;
                          exp--;
                        }
		      if (vsize > prec)
			{
			  vp += vsize - prec;
			  vsize = prec;
			}
                      MPN_COPY_INCR (rp, vp, vsize);
                      rsize = vsize;
                      goto done;
		    }
		  if (vsize == 0)
		    {
                      vp = up;
                      vsize = usize;
                      goto cancellation;
		    }
		}
	      while (up[usize - 1] == vp[vsize - 1]);
	    }

	  if (up[usize - 1] < vp[vsize - 1])
	    {
	      /* For simplicity, swap U and V.  Note that since the loop above
		 wouldn't have exited unless up[usize - 1] and vp[vsize - 1]
		 were non-equal, this if-statement catches all cases where U
		 is smaller than V.  */
	      MPN_SRCPTR_SWAP (up,usize, vp,vsize);
	      negate ^= 1;
	      /* negating ediff not necessary since it is 0.  */
	    }

	  /* Check for
	     x+1 00000000 ...
	      x  ffffffff ... */
	  if (up[usize - 1] != vp[vsize - 1] + 1)
	    goto general_case;
	  usize--;
	  vsize--;
	  exp--;
	}
      else /* ediff == 1 */
	{
	  /* Check for
	     1 00000000 ...
	     0 ffffffff ... */

	  if (up[usize - 1] != 1 || vp[vsize - 1] != GMP_NUMB_MAX
	      || (usize >= 2 && up[usize - 2] != 0))
	    goto general_case;

	  usize--;
	  exp--;
	}

      /* Skip sequences of 00000000/ffffffff */
      while (vsize != 0 && usize != 0 && up[usize - 1] == 0
	     && vp[vsize - 1] == GMP_NUMB_MAX)
	{
	  usize--;
	  vsize--;
	  exp--;
	}

      if (usize == 0)
	{
	  while (vsize != 0 && vp[vsize - 1] == GMP_NUMB_MAX)
	    {
	      vsize--;
	      exp--;
	    }
	}

      if (usize > prec - 1)
	{
	  up += usize - (prec - 1);
	  usize = prec - 1;
	}
      if (vsize > prec - 1)
	{
	  vp += vsize - (prec - 1);
	  vsize = prec - 1;
	}

      tp = TMP_ALLOC_LIMBS (prec);
      {
	mp_limb_t cy_limb;
	if (vsize == 0)
	  {
	    mp_size_t size, i;
	    size = usize;
	    for (i = 0; i < size; i++)
	      tp[i] = up[i];
	    tp[size] = 1;
	    rsize = size + 1;
	    exp++;
	    goto normalize;
	  }
	if (usize == 0)
	  {
	    mp_size_t size, i;
	    size = vsize;
	    for (i = 0; i < size; i++)
	      tp[i] = ~vp[i] & GMP_NUMB_MASK;
	    cy_limb = 1 - mpn_add_1 (tp, tp, vsize, (mp_limb_t) 1);
	    rsize = vsize;
	    if (cy_limb == 0)
	      {
		tp[rsize] = 1;
		rsize++;
		exp++;
	      }
	    goto normalize;
	  }
	if (usize >= vsize)
	  {
	    /* uuuu     */
	    /* vv       */
	    mp_size_t size;
	    size = usize - vsize;
	    MPN_COPY (tp, up, size);
	    cy_limb = mpn_sub_n (tp + size, up + size, vp, vsize);
	    rsize = usize;
	  }
	else /* (usize < vsize) */
	  {
	    /* uuuu     */
	    /* vvvvvvv  */
	    mp_size_t size, i;
	    size = vsize - usize;
	    for (i = 0; i < size; i++)
	      tp[i] = ~vp[i] & GMP_NUMB_MASK;
	    cy_limb = mpn_sub_n (tp + size, up, vp + size, usize);
	    cy_limb+= mpn_sub_1 (tp + size, tp + size, usize, (mp_limb_t) 1);
	    cy_limb-= mpn_add_1 (tp, tp, vsize, (mp_limb_t) 1);
	    rsize = vsize;
	  }
	if (cy_limb == 0)
	  {
	    tp[rsize] = 1;
	    rsize++;
	    exp++;
	  }
	goto normalize;
      }
    }

general_case:
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

  if (ediff >= prec)
    {
      /* V completely cancelled.  */
      if (rp != up)
	MPN_COPY (rp, up, usize);
      rsize = usize;
    }
  else
    {
      /* Allocate temp space for the result.  Allocate
	 just vsize + ediff later???  */
      tp = TMP_ALLOC_LIMBS (prec);

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
	      if (usize >= vsize)
		{
		  /* uuuu     */
		  /* vv       */
		  mp_size_t size;
		  size = usize - vsize;
		  MPN_COPY (tp, up, size);
		  mpn_sub_n (tp + size, up + size, vp, vsize);
		  rsize = usize;
		}
	      else /* (usize < vsize) */
		{
		  /* uuuu     */
		  /* vvvvvvv  */
		  mp_size_t size, i;
		  size = vsize - usize;
		  tp[0] = -vp[0] & GMP_NUMB_MASK;
		  for (i = 1; i < size; i++)
		    tp[i] = ~vp[i] & GMP_NUMB_MASK;
		  mpn_sub_n (tp + size, up, vp + size, usize);
		  mpn_sub_1 (tp + size, tp + size, usize, (mp_limb_t) 1);
		  rsize = vsize;
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

    normalize:
      /* Full normalize.  Optimize later.  */
      while (rsize != 0 && tp[rsize - 1] == 0)
	{
	  rsize--;
	  exp--;
	}
      MPN_COPY (rp, tp, rsize);
    }

 done:
  r->_mp_size = negate ? -rsize : rsize;
  if (rsize == 0)
    exp = 0;
  r->_mp_exp = exp;
  TMP_FREE;
}
