/* Cray mpn_popcount -- population count.

Copyright 2000 Free Software Foundation, Inc.

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

#include <intrinsics.h>
#include "gmp.h"
#include "gmp-impl.h"

unsigned long int
mpn_popcount (mp_srcptr p, mp_size_t n)
{
  unsigned long int result = 0;
  mp_size_t i;
  for (i = 0; i < n; i++)
    result += _popcnt (p[i]);
  return result;
}
