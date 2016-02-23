/* mpz_pow_ui -- mpz raised to ulong.

Copyright 2001, 2008 Free Software Foundation, Inc.

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
mpz_pow_ui (mpz_ptr r, mpz_srcptr b, unsigned long int e)
{
  /* We test some small exponents here, mainly to avoid the overhead of
     mpz_n_pow_ui for small bases and exponents.  */
  switch (e)
    {
    case 0:
      mpz_set_ui (r, 1);
      break;
    case 1:
      mpz_set (r, b);
      break;
    case 2:
      mpz_mul (r, b, b);
      break;
    default:
      mpz_n_pow_ui (r, PTR(b), (mp_size_t) SIZ(b), e);
    }
}
