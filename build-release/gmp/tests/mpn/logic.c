/* Test mpn_and, mpn_ior, mpn_xor, mpn_andn, mpn_iorn, mpn_xnor, mpn_nand, and
   mpn_nior.

Copyright 2011, 2012, 2013 Free Software Foundation, Inc.

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


#include <stdlib.h>
#include <stdio.h>

/* Fake native prevalence of the tested operations, so that we actually test
   the compiled functions, i.e., the ones which users will reach.  The inlined
   variants will be tested through tests/mpz/logic.c.  */
#define HAVE_NATIVE_mpn_com    1
#define HAVE_NATIVE_mpn_and_n  1
#define HAVE_NATIVE_mpn_andn_n 1
#define HAVE_NATIVE_mpn_nand_n 1
#define HAVE_NATIVE_mpn_ior_n  1
#define HAVE_NATIVE_mpn_iorn_n 1
#define HAVE_NATIVE_mpn_nior_n 1
#define HAVE_NATIVE_mpn_xor_n  1
#define HAVE_NATIVE_mpn_xnor_n 1

#include "gmp.h"
#include "gmp-impl.h"
#include "tests.h"


void
check_one (mp_srcptr refp, mp_srcptr rp, mp_srcptr ap, mp_srcptr bp, mp_size_t n, char *funcname)
{
  if (mpn_cmp (refp, rp, n))
    {
      printf ("ERROR in mpn_%s\n", funcname);
      printf ("a: "); mpn_dump (ap, n);
      printf ("b: "); mpn_dump (bp, n);
      printf ("r:   "); mpn_dump (rp, n);
      printf ("ref: "); mpn_dump (refp, n);
      abort();
    }
}

int
main (int argc, char **argv)
{
  mp_ptr ap, bp, rp, refp;
  mp_size_t max_n, n;
  gmp_randstate_ptr rands;
  long test, reps = 1000;
  TMP_SDECL;
  TMP_SMARK;

  tests_start ();
  TESTS_REPS (reps, argv, argc);

  rands = RANDS;

  max_n = 32;

  ap = TMP_SALLOC_LIMBS (max_n);
  bp = TMP_SALLOC_LIMBS (max_n);
  rp = TMP_SALLOC_LIMBS (max_n);
  refp = TMP_SALLOC_LIMBS (max_n);

  for (test = 0; test < reps; test++)
    {
      for (n = 1; n <= max_n; n++)
	{
	  mpn_random2 (ap, n);
	  mpn_random2 (bp, n);

	  refmpn_and_n (refp, ap, bp, n);
	  mpn_and_n (rp, ap, bp, n);
	  check_one (refp, rp, ap, bp, n, "and_n");

	  refmpn_ior_n (refp, ap, bp, n);
	  mpn_ior_n (rp, ap, bp, n);
	  check_one (refp, rp, ap, bp, n, "ior_n");

	  refmpn_xor_n (refp, ap, bp, n);
	  mpn_xor_n (rp, ap, bp, n);
	  check_one (refp, rp, ap, bp, n, "xor_n");

	  refmpn_andn_n (refp, ap, bp, n);
	  mpn_andn_n (rp, ap, bp, n);
	  check_one (refp, rp, ap, bp, n, "andn_n");

	  refmpn_iorn_n (refp, ap, bp, n);
	  mpn_iorn_n (rp, ap, bp, n);
	  check_one (refp, rp, ap, bp, n, "iorn_n");

	  refmpn_nand_n (refp, ap, bp, n);
	  mpn_nand_n (rp, ap, bp, n);
	  check_one (refp, rp, ap, bp, n, "nand_n");

	  refmpn_nior_n (refp, ap, bp, n);
	  mpn_nior_n (rp, ap, bp, n);
	  check_one (refp, rp, ap, bp, n, "nior_n");

	  refmpn_xnor_n (refp, ap, bp, n);
	  mpn_xnor_n (rp, ap, bp, n);
	  check_one (refp, rp, ap, bp, n, "xnor_n");

	  refmpn_com (refp, ap, n);
	  mpn_com (rp, ap, n);
	  check_one (refp, rp, ap, bp, n, "com");
	}
    }

  TMP_SFREE;
  tests_end ();
  return 0;
}
