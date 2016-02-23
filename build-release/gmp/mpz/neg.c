/* mpz_neg(mpz_ptr dst, mpz_ptr src) -- Assign the negated value of SRC to DST.

Copyright 1991, 1993, 1994, 1995, 2001, 2012 Free Software Foundation, Inc.

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

#define __GMP_FORCE_mpz_neg 1

#include "gmp.h"
#include "gmp-impl.h"

void
mpz_neg (mpz_ptr w, mpz_srcptr u)
{
  mp_ptr wp;
  mp_srcptr up;
  mp_size_t usize, size;

  usize = SIZ (u);

  if (u != w)
    {
      size = ABS (usize);

      wp = MPZ_NEWALLOC (w, size);

      up = PTR (u);

      MPN_COPY (wp, up, size);
    }

  SIZ (w) = -usize;
}
