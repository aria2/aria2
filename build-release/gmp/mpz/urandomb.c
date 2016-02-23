/* mpz_urandomb (rop, state, n) -- Generate a uniform pseudorandom
   integer in the range 0 to 2^N - 1, inclusive, using STATE as the
   random state previously initialized by a call to gmp_randinit().

Copyright 1999, 2000, 2002 Free Software Foundation, Inc.

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
mpz_urandomb (mpz_ptr rop, gmp_randstate_t rstate, mp_bitcnt_t nbits)
{
  mp_ptr rp;
  mp_size_t size;

  size = BITS_TO_LIMBS (nbits);
  rp = MPZ_REALLOC (rop, size);

  _gmp_rand (rp, rstate, nbits);
  MPN_NORMALIZE (rp, size);
  SIZ (rop) = size;
}
