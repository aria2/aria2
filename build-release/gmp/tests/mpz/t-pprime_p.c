/* Exercise mpz_probab_prime_p.

Copyright 2002 Free Software Foundation, Inc.

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
#include "gmp.h"
#include "gmp-impl.h"
#include "tests.h"


/* Enhancements:

   - Test some big primes don't come back claimed to be composite.
   - Test some big composites don't come back claimed to be certainly prime.
   - Test some big composites with small factors are identified as certainly
     composite.  */


/* return 1 if prime, 0 if composite */
int
isprime (long n)
{
  long  i;

  n = ABS(n);

  if (n < 2)
    return 0;
  if (n == 2)
    return 1;
  if ((n & 1) == 0)
    return 0;

  for (i = 3; i < n; i++)
    if ((n % i) == 0)
      return 0;

  return 1;
}

void
check_one (mpz_srcptr n, int want)
{
  int  got;

  got = mpz_probab_prime_p (n, 25);

  /* "definitely prime" is fine if we only wanted "probably prime" */
  if (got == 2 && want == 1)
    want = 2;

  if (got != want)
    {
      printf ("mpz_probab_prime_p\n");
      mpz_trace ("  n    ", n);
      printf    ("  got =%d", got);
      printf    ("  want=%d", want);
      abort ();
    }
}

void
check_pn (mpz_ptr n, int want)
{
  check_one (n, want);
  mpz_neg (n, n);
  check_one (n, want);
}

/* expect certainty for small n */
void
check_small (void)
{
  mpz_t  n;
  long   i;

  mpz_init (n);

  for (i = 0; i < 300; i++)
    {
      mpz_set_si (n, i);
      check_pn (n, isprime (i));
    }

  mpz_clear (n);
}

int
main (void)
{
  tests_start ();

  check_small ();

  tests_end ();
  exit (0);
}
