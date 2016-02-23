/* mpq_div -- divide two rational numbers.

Copyright 1991, 1994, 1995, 1996, 2000, 2001 Free Software Foundation, Inc.

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
mpq_div (mpq_ptr quot, mpq_srcptr op1, mpq_srcptr op2)
{
  mpz_t gcd1, gcd2;
  mpz_t tmp1, tmp2;
  mpz_t numtmp;
  mp_size_t op1_num_size;
  mp_size_t op1_den_size;
  mp_size_t op2_num_size;
  mp_size_t op2_den_size;
  mp_size_t alloc;
  TMP_DECL;

  op2_num_size = ABSIZ(NUM(op2));

  if (UNLIKELY (op2_num_size == 0))
    DIVIDE_BY_ZERO;

  op1_num_size = ABSIZ(NUM(op1));

  if (op1_num_size == 0)
    {
      /* We special case this to simplify allocation logic; gcd(0,x) = x
	 is a singular case for the allocations.  */
      SIZ(NUM(quot)) = 0;
      PTR(DEN(quot))[0] = 1;
      SIZ(DEN(quot)) = 1;
      return;
    }

  op2_den_size =   SIZ(DEN(op2));
  op1_den_size =   SIZ(DEN(op1));

  TMP_MARK;

  alloc = MIN (op1_num_size, op2_num_size);
  MPZ_TMP_INIT (gcd1, alloc);

  alloc = MIN (op1_den_size, op2_den_size);
  MPZ_TMP_INIT (gcd2, alloc);

  alloc = MAX (op1_num_size, op2_num_size);
  MPZ_TMP_INIT (tmp1, alloc);

  alloc = MAX (op1_den_size, op2_den_size);
  MPZ_TMP_INIT (tmp2, alloc);

  alloc = op1_num_size + op2_den_size;
  MPZ_TMP_INIT (numtmp, alloc);

  /* QUOT might be identical to either operand, so don't store the result there
     until we are finished with the input operands.  We can overwrite the
     numerator of QUOT when we are finished with the numerators of OP1 and
     OP2.  */

  mpz_gcd (gcd1, NUM(op1), NUM(op2));
  mpz_gcd (gcd2, DEN(op2), DEN(op1));

  mpz_divexact_gcd (tmp1, NUM(op1), gcd1);
  mpz_divexact_gcd (tmp2, DEN(op2), gcd2);

  mpz_mul (numtmp, tmp1, tmp2);

  mpz_divexact_gcd (tmp1, NUM(op2), gcd1);
  mpz_divexact_gcd (tmp2, DEN(op1), gcd2);

  mpz_mul (DEN(quot), tmp1, tmp2);

  /* We needed to go via NUMTMP to take care of QUOT being the same as OP2.
     Now move NUMTMP to QUOT->_mp_num.  */
  mpz_set (NUM(quot), numtmp);

  /* Keep the denominator positive.  */
  if (SIZ(DEN(quot)) < 0)
    {
      SIZ(DEN(quot)) = -SIZ(DEN(quot));
      SIZ(NUM(quot)) = -SIZ(NUM(quot));
    }

  TMP_FREE;
}
