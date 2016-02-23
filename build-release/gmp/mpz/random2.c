/* mpz_random2 -- Generate a positive random mpz_t of specified size, with
   long runs of consecutive ones and zeros in the binary representation.
   Meant for testing of other MP routines.

Copyright 1991, 1993, 1994, 1996, 2001, 2012 Free Software Foundation, Inc.

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
mpz_random2 (mpz_ptr x, mp_size_t size)
{
  mp_size_t abs_size;
  mp_ptr xp;

  abs_size = ABS (size);
  if (abs_size != 0)
    {
      xp = MPZ_REALLOC (x, abs_size);

      mpn_random2 (xp, abs_size);
    }

  SIZ (x) = size;
}
