/* mpq_cmp(u,v) -- Compare U, V.  Return positive, zero, or negative
   based on if U > V, U == V, or U < V.

Copyright 1991, 1994, 1996, 2001, 2002, 2005 Free Software Foundation, Inc.

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
#include "longlong.h"

int
mpq_cmp (const MP_RAT *op1, const MP_RAT *op2)
{
  mp_size_t num1_size = SIZ(NUM(op1));
  mp_size_t den1_size = SIZ(DEN(op1));
  mp_size_t num2_size = SIZ(NUM(op2));
  mp_size_t den2_size = SIZ(DEN(op2));
  mp_size_t tmp1_size, tmp2_size;
  mp_ptr tmp1_ptr, tmp2_ptr;
  mp_size_t num1_sign;
  int cc;
  TMP_DECL;

  /* need canonical signs to get right result */
  ASSERT (den1_size > 0);
  ASSERT (den2_size > 0);

  if (num1_size == 0)
    return -num2_size;
  if (num2_size == 0)
    return num1_size;
  if ((num1_size ^ num2_size) < 0) /* I.e. are the signs different? */
    return num1_size;

  num1_sign = num1_size;
  num1_size = ABS (num1_size);
  num2_size = ABS (num2_size);

  tmp1_size = num1_size + den2_size;
  tmp2_size = num2_size + den1_size;

  /* 1. Check to see if we can tell which operand is larger by just looking at
     the number of limbs.  */

  /* NUM1 x DEN2 is either TMP1_SIZE limbs or TMP1_SIZE-1 limbs.
     Same for NUM1 x DEN1 with respect to TMP2_SIZE.  */
  if (tmp1_size > tmp2_size + 1)
    /* NUM1 x DEN2 is surely larger in magnitude than NUM2 x DEN1.  */
    return num1_sign;
  if (tmp2_size > tmp1_size + 1)
    /* NUM1 x DEN2 is surely smaller in magnitude than NUM2 x DEN1.  */
    return -num1_sign;

  /* 2. Same, but compare the number of significant bits.  */
  {
    int cnt1, cnt2;
    mp_bitcnt_t bits1, bits2;

    count_leading_zeros (cnt1, PTR(NUM(op1))[num1_size - 1]);
    count_leading_zeros (cnt2, PTR(DEN(op2))[den2_size - 1]);
    bits1 = tmp1_size * GMP_NUMB_BITS - cnt1 - cnt2 + 2 * GMP_NAIL_BITS;

    count_leading_zeros (cnt1, PTR(NUM(op2))[num2_size - 1]);
    count_leading_zeros (cnt2, PTR(DEN(op1))[den1_size - 1]);
    bits2 = tmp2_size * GMP_NUMB_BITS - cnt1 - cnt2 + 2 * GMP_NAIL_BITS;

    if (bits1 > bits2 + 1)
      return num1_sign;
    if (bits2 > bits1 + 1)
      return -num1_sign;
  }

  /* 3. Finally, cross multiply and compare.  */

  TMP_MARK;
  TMP_ALLOC_LIMBS_2 (tmp1_ptr,tmp1_size, tmp2_ptr,tmp2_size);

  if (num1_size >= den2_size)
    tmp1_size -= 0 == mpn_mul (tmp1_ptr,
			       PTR(NUM(op1)), num1_size,
			       PTR(DEN(op2)), den2_size);
  else
    tmp1_size -= 0 == mpn_mul (tmp1_ptr,
			       PTR(DEN(op2)), den2_size,
			       PTR(NUM(op1)), num1_size);

   if (num2_size >= den1_size)
     tmp2_size -= 0 == mpn_mul (tmp2_ptr,
				PTR(NUM(op2)), num2_size,
				PTR(DEN(op1)), den1_size);
   else
     tmp2_size -= 0 == mpn_mul (tmp2_ptr,
				PTR(DEN(op1)), den1_size,
				PTR(NUM(op2)), num2_size);


  cc = tmp1_size - tmp2_size != 0
    ? tmp1_size - tmp2_size : mpn_cmp (tmp1_ptr, tmp2_ptr, tmp1_size);
  TMP_FREE;
  return num1_sign < 0 ? -cc : cc;
}
