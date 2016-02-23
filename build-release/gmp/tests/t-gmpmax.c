/* Check the values of __GMP_UINT_MAX etc.

Copyright 2001 Free Software Foundation, Inc.

This file is part of the GNU MP Library test suite.

The GNU MP Library test suite is free software; you can redistribute it
and/or modify it under the terms of the GNU General Public License as
published by the Free Software Foundation; either version 3 of the License,
or (at your option) any later version.

The GNU MP Library test suite is distributed in the hope that it will be
useful, but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General
Public License for more details.

You should have received a copy of the GNU General Public License along with
the GNU MP Library test suite.  If not, see http://www.gnu.org/licenses/.  */

#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include "gmp.h"


/* __GMP_UINT_MAX etc are generated with expressions in gmp.h since we don't
   want to demand <limits.h> or forcibly include it.  Check the expressions
   come out the same as <limits.h>.  */

int
main (int argc, char *argv[])
{
  int  error = 0;

#ifdef UINT_MAX
  if (__GMP_UINT_MAX != UINT_MAX)
    {
      printf ("__GMP_UINT_MAX incorrect\n");
      printf ("  __GMP_UINT_MAX  %u  0x%X\n", __GMP_UINT_MAX, __GMP_UINT_MAX);
      printf ("  UINT_MAX        %u  0x%X\n", UINT_MAX, UINT_MAX);
      error = 1;
    }
#endif

  /* gcc 2.95.2 limits.h on solaris 2.5.1 incorrectly selects a 64-bit
     LONG_MAX, leading to some integer overflow in ULONG_MAX and a spurious
     __GMP_ULONG_MAX != ULONG_MAX.  Casting ULONG_MAX to unsigned long is a
     workaround.  */
#ifdef ULONG_MAX
  if (__GMP_ULONG_MAX != (unsigned long) ULONG_MAX)
    {
      printf ("__GMP_ULONG_MAX incorrect\n");
      printf ("  __GMP_ULONG_MAX  %lu  0x%lX\n", __GMP_ULONG_MAX, __GMP_ULONG_MAX);
      printf ("  ULONG_MAX        %lu  0x%lX\n", ULONG_MAX, ULONG_MAX);
      error = 1;
    }
#endif

#ifdef USHRT_MAX
  if (__GMP_USHRT_MAX != USHRT_MAX)
    {
      printf ("__GMP_USHRT_MAX incorrect\n");
      printf ("  __GMP_USHRT_MAX  %hu  0x%hX\n", __GMP_USHRT_MAX, __GMP_USHRT_MAX);
      printf ("  USHRT_MAX        %hu  0x%hX\n", USHRT_MAX, USHRT_MAX);
      error = 1;
    }
#endif

  if (error)
    abort ();

  exit (0);
}
