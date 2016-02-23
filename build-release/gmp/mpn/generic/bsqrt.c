/* mpn_bsqrt, a^{1/2} (mod 2^n).

Copyright 2009, 2010, 2012 Free Software Foundation, Inc.

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
mpn_bsqrt (mp_ptr rp, mp_srcptr ap, mp_bitcnt_t nb, mp_ptr tp)
{
  mp_ptr sp;
  mp_size_t n;

  ASSERT (nb > 0);

  n = nb / GMP_NUMB_BITS;
  sp = tp + n;

  mpn_bsqrtinv (sp, ap, nb, tp);
  mpn_mullo_n (rp, sp, ap, n);
}
