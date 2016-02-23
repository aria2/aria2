/* mpz_divisible_p -- mpz by mpz divisibility test

Copyright 2001, 2002 Free Software Foundation, Inc.

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

int
mpz_divisible_p (mpz_srcptr a, mpz_srcptr d)
{
  mp_size_t dsize = SIZ(d);
  mp_size_t asize = SIZ(a);

  if (UNLIKELY (dsize == 0))
    return (asize == 0);

  return mpn_divisible_p (PTR(a), ABS(asize), PTR(d), ABS(dsize));
}
