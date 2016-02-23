/* mpf_set_default_prec --

Copyright 1993, 1994, 1995, 2001 Free Software Foundation, Inc.

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

mp_size_t __gmp_default_fp_limb_precision = __GMPF_BITS_TO_PREC (53);

void
mpf_set_default_prec (mp_bitcnt_t prec_in_bits) __GMP_NOTHROW
{
  __gmp_default_fp_limb_precision = __GMPF_BITS_TO_PREC (prec_in_bits);
}
