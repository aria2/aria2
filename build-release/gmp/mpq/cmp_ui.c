/* mpq_cmp_ui(u,vn,vd) -- Compare U with Vn/Vd.  Return positive, zero, or
   negative based on if U > V, U == V, or U < V.  Vn and Vd may have
   common factors.

Copyright 1993, 1994, 1996, 2000, 2001, 2002, 2003, 2005 Free Software
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
_mpq_cmp_ui (const MP_RAT *op1, unsigned long int num2, unsigned long int den2)
{
  mp_size_t num1_size = SIZ(NUM(op1));
  mp_size_t den1_size = SIZ(DEN(op1));
  mp_size_t tmp1_size, tmp2_size;
  mp_ptr tmp1_ptr, tmp2_ptr;
  mp_limb_t cy_limb;
  int cc;
  TMP_DECL;

#if GMP_NAIL_BITS != 0
  if ((num2 | den2) > GMP_NUMB_MAX)
    {
      mpq_t op2;
      mpq_init (op2);
      mpz_set_ui (mpq_numref (op2), num2);
      mpz_set_ui (mpq_denref (op2), den2);
      cc = mpq_cmp (op1, op2);
      mpq_clear (op2);
      return cc;
    }
#endif

  /* need canonical sign to get right result */
  ASSERT (den1_size > 0);

  if (UNLIKELY (den2 == 0))
    DIVIDE_BY_ZERO;

  if (num1_size == 0)
    return -(num2 != 0);
  if (num1_size < 0)
    return num1_size;
  if (num2 == 0)
    return num1_size;

  /* NUM1 x DEN2 is either TMP1_SIZE limbs or TMP1_SIZE-1 limbs.
     Same for NUM1 x DEN1 with respect to TMP2_SIZE.  */
  if (num1_size > den1_size + 1)
    /* NUM1 x DEN2 is surely larger in magnitude than NUM2 x DEN1.  */
    return num1_size;
  if (den1_size > num1_size + 1)
    /* NUM1 x DEN2 is surely smaller in magnitude than NUM2 x DEN1.  */
    return -num1_size;

  TMP_MARK;
  tmp1_ptr = TMP_ALLOC_LIMBS (num1_size + 1);
  tmp2_ptr = TMP_ALLOC_LIMBS (den1_size + 1);

  cy_limb = mpn_mul_1 (tmp1_ptr, PTR(NUM(op1)), num1_size,
                       (mp_limb_t) den2);
  tmp1_ptr[num1_size] = cy_limb;
  tmp1_size = num1_size + (cy_limb != 0);

  cy_limb = mpn_mul_1 (tmp2_ptr, PTR(DEN(op1)), den1_size,
                       (mp_limb_t) num2);
  tmp2_ptr[den1_size] = cy_limb;
  tmp2_size = den1_size + (cy_limb != 0);

  cc = tmp1_size - tmp2_size != 0
    ? tmp1_size - tmp2_size : mpn_cmp (tmp1_ptr, tmp2_ptr, tmp1_size);
  TMP_FREE;
  return cc;
}
