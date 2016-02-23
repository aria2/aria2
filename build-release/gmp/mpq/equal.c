/* mpq_equal(u,v) -- Compare U, V.  Return non-zero if they are equal, zero
   if they are non-equal.

Copyright 1996, 2001, 2002 Free Software Foundation, Inc.

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
mpq_equal (mpq_srcptr op1, mpq_srcptr op2) __GMP_NOTHROW
{
  mp_size_t  num1_size, num2_size, den1_size, den2_size, i;
  mp_srcptr  num1_ptr,  num2_ptr,  den1_ptr,  den2_ptr;

  /* need fully canonical for correct results */
  ASSERT_MPQ_CANONICAL (op1);
  ASSERT_MPQ_CANONICAL (op2);

  num1_size = SIZ(NUM(op1));
  num2_size = SIZ(NUM(op2));
  if (num1_size != num2_size)
    return 0;

  num1_ptr = PTR(NUM(op1));
  num2_ptr = PTR(NUM(op2));
  num1_size = ABS (num1_size);
  for (i = 0; i < num1_size; i++)
    if (num1_ptr[i] != num2_ptr[i])
      return 0;

  den1_size = SIZ(DEN(op1));
  den2_size = SIZ(DEN(op2));
  if (den1_size != den2_size)
    return 0;

  den1_ptr = PTR(DEN(op1));
  den2_ptr = PTR(DEN(op2));
  for (i = 0; i < den1_size; i++)
    if (den1_ptr[i] != den2_ptr[i])
      return 0;

  return 1;
}
