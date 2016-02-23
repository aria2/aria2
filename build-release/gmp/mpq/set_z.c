/* mpq_set_z (dest,src) -- Set DEST to SRC.

Copyright 1996, 2001, 2012 Free Software Foundation, Inc.

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
mpq_set_z (mpq_ptr dest, mpz_srcptr src)
{
  mp_size_t num_size;
  mp_size_t abs_num_size;
  mp_ptr dp;

  num_size = SIZ (src);
  abs_num_size = ABS (num_size);
  dp = MPZ_NEWALLOC (NUM(dest), abs_num_size);
  SIZ(NUM(dest)) = num_size;
  MPN_COPY (dp, PTR(src), abs_num_size);

  PTR(DEN(dest))[0] = 1;
  SIZ(DEN(dest)) = 1;
}
