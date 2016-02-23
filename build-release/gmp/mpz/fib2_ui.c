/* mpz_fib2_ui -- calculate Fibonacci numbers.

Copyright 2001, 2012 Free Software Foundation, Inc.

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

#include <stdio.h>
#include "gmp.h"
#include "gmp-impl.h"


void
mpz_fib2_ui (mpz_ptr fn, mpz_ptr fnsub1, unsigned long n)
{
  mp_ptr     fp, f1p;
  mp_size_t  size;

  size = MPN_FIB2_SIZE (n);
  fp =  MPZ_REALLOC (fn,     size);
  f1p = MPZ_REALLOC (fnsub1, size);

  size = mpn_fib2_ui (fp, f1p, n);

  SIZ(fn)     = size - (n == 0);
  SIZ(fnsub1) = size - (f1p[size-1] == 0);
}
