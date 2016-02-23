/* mpf_init_set -- Initialize a float and assign it from another float.

Copyright 1993, 1994, 1995, 2000, 2001, 2004 Free Software Foundation, Inc.

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
mpf_init_set (mpf_ptr r, mpf_srcptr s)
{
  mp_ptr rp, sp;
  mp_size_t ssize, size;
  mp_size_t prec;

  prec = __gmp_default_fp_limb_precision;
  r->_mp_d = (mp_ptr) (*__gmp_allocate_func) ((prec + 1) * BYTES_PER_MP_LIMB);
  r->_mp_prec = prec;

  prec++;		/* lie not to lose precision in assignment */
  ssize = s->_mp_size;
  size = ABS (ssize);

  rp = r->_mp_d;
  sp = s->_mp_d;

  if (size > prec)
    {
      sp += size - prec;
      size = prec;
    }

  r->_mp_exp = s->_mp_exp;
  r->_mp_size = ssize >= 0 ? size : -size;

  MPN_COPY (rp, sp, size);
}
