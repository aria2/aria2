/* mpz_init2 -- initialize mpz, with requested size in bits.

Copyright 2001, 2002, 2008 Free Software Foundation, Inc.

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

#include <stdlib.h>
#include <stdio.h>
#include "gmp.h"
#include "gmp-impl.h"

void
mpz_init2 (mpz_ptr x, mp_bitcnt_t bits)
{
  mp_size_t  new_alloc;

  bits -= (bits != 0);		/* Round down, except if 0 */
  new_alloc = 1 + bits / GMP_NUMB_BITS;

  if (sizeof (unsigned long) > sizeof (int)) /* param vs _mp_size field */
    {
      if (UNLIKELY (new_alloc > INT_MAX))
	{
	  fprintf (stderr, "gmp: overflow in mpz type\n");
	  abort ();
	}
    }

  PTR(x) = __GMP_ALLOCATE_FUNC_LIMBS (new_alloc);
  ALLOC(x) = new_alloc;
  SIZ(x) = 0;

#ifdef __CHECKER__
  /* let the low limb look initialized, for the benefit of mpz_get_ui etc */
  PTR(x)[0] = 0;
#endif
}
