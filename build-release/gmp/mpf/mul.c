/* mpf_mul -- Multiply two floats.

Copyright 1993, 1994, 1996, 2001, 2005 Free Software Foundation, Inc.

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
mpf_mul (mpf_ptr r, mpf_srcptr u, mpf_srcptr v)
{
  mp_srcptr up, vp;
  mp_size_t usize, vsize;
  mp_size_t sign_product;
  mp_size_t prec = r->_mp_prec;
  TMP_DECL;

  TMP_MARK;
  usize = u->_mp_size;
  vsize = v->_mp_size;
  sign_product = usize ^ vsize;

  usize = ABS (usize);
  vsize = ABS (vsize);

  up = u->_mp_d;
  vp = v->_mp_d;
  if (usize > prec)
    {
      up += usize - prec;
      usize = prec;
    }
  if (vsize > prec)
    {
      vp += vsize - prec;
      vsize = prec;
    }

  if (usize == 0 || vsize == 0)
    {
      r->_mp_size = 0;
      r->_mp_exp = 0;		/* ??? */
    }
  else
    {
      mp_size_t rsize;
      mp_limb_t cy_limb;
      mp_ptr rp, tp;
      mp_size_t adj;

      rsize = usize + vsize;
      tp = TMP_ALLOC_LIMBS (rsize);
      cy_limb = (usize >= vsize
		 ? mpn_mul (tp, up, usize, vp, vsize)
		 : mpn_mul (tp, vp, vsize, up, usize));

      adj = cy_limb == 0;
      rsize -= adj;
      prec++;
      if (rsize > prec)
	{
	  tp += rsize - prec;
	  rsize = prec;
	}
      rp = r->_mp_d;
      MPN_COPY (rp, tp, rsize);
      r->_mp_exp = u->_mp_exp + v->_mp_exp - adj;
      r->_mp_size = sign_product >= 0 ? rsize : -rsize;
    }
  TMP_FREE;
}
