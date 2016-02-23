/* mpf_integer_p -- test whether an mpf is an integer */

/*
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
mpf_integer_p (mpf_srcptr f) __GMP_NOTHROW
{
  mp_srcptr ptr;
  mp_exp_t exp;
  mp_size_t size, frac, i;

  size = SIZ (f);
  if (size == 0)
    return 1;  /* zero is an integer */

  exp = EXP (f);
  if (exp <= 0)
    return 0;  /* has only fraction limbs */

  /* any fraction limbs must be zero */
  frac = ABS (size) - exp;
  ptr = PTR (f);
  for (i = 0; i < frac; i++)
    if (ptr[i] != 0)
      return 0;

  return 1;
}
