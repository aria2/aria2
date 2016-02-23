/* mpq_get_den(den,rat_src) -- Set DEN to the denominator of RAT_SRC.

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
mpq_get_den (mpz_ptr den, mpq_srcptr src)
{
  mp_size_t size = SIZ(DEN(src));
  mp_ptr dp;

  dp = MPZ_NEWALLOC (den, size);
  SIZ(den) = size;
  MPN_COPY (dp, PTR(DEN(src)), size);
}
