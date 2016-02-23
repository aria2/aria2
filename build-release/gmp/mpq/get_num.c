 /* mpq_get_num(num,rat_src) -- Set NUM to the numerator of RAT_SRC.

Copyright 1991, 1994, 1995, 2001, 2012 Free Software Foundation, Inc.

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
mpq_get_num (mpz_ptr num, mpq_srcptr src)
{
  mp_size_t size = SIZ(NUM(src));
  mp_size_t abs_size = ABS (size);
  mp_ptr dp;

  dp = MPZ_NEWALLOC (num, abs_size);
  SIZ(num) = size;

  MPN_COPY (dp, PTR(NUM(src)), abs_size);
}
