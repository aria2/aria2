/* mpq_init -- Make a new rational number with value 0/1.

Copyright 1991, 1994, 1995, 2000, 2001, 2002 Free Software Foundation, Inc.

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
mpq_init (MP_RAT *x)
{
  ALLOC(NUM(x)) = 1;
  PTR(NUM(x)) = (mp_ptr) (*__gmp_allocate_func) (BYTES_PER_MP_LIMB);
  SIZ(NUM(x)) = 0;
  ALLOC(DEN(x)) = 1;
  PTR(DEN(x)) = (mp_ptr) (*__gmp_allocate_func) (BYTES_PER_MP_LIMB);
  PTR(DEN(x))[0] = 1;
  SIZ(DEN(x)) = 1;

#ifdef __CHECKER__
  /* let the low limb look initialized, for the benefit of mpz_get_ui etc */
  PTR(NUM(x))[0] = 0;
#endif
}
