/* mpz_init_set_str(string, base) -- Convert the \0-terminated string
   STRING in base BASE to a multiple precision integer.  Return a MP_INT
   structure representing the integer.  Allow white space in the
   string.  If BASE == 0 determine the base in the C standard way,
   i.e.  0xhh...h means base 16, 0oo...o means base 8, otherwise
   assume base 10.

Copyright 1991, 1993, 1994, 1995, 2000, 2001, 2002, 2012 Free Software
Foundation, Inc.

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
mpz_init_set_str (mpz_ptr x, const char *str, int base)
{
  ALLOC (x) = 1;
  PTR (x) = (mp_ptr) (*__gmp_allocate_func) (BYTES_PER_MP_LIMB);

  /* if str has no digits mpz_set_str leaves x->_mp_size unset */
  SIZ (x) = 0;

#ifdef __CHECKER__
  /* let the low limb look initialized, for the benefit of mpz_get_ui etc */
  PTR (x)[0] = 0;
#endif

  return mpz_set_str (x, str, base);
}
