/* Test conversion and I/O using mpq_out_str and mpq_inp_str.

Copyright 1993, 1994, 1996, 2000, 2001, 2012 Free Software Foundation, Inc.

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

#include "config.h"

#include <stdio.h>
#include <stdlib.h>
#if HAVE_UNISTD_H
#include <unistd.h>		/* for unlink */
#endif

#include "gmp.h"
#include "gmp-impl.h"
#include "tests.h"

#define FILENAME  "io.tmp"

void
debug_mp (mpq_t x, int base)
{
  mpq_out_str (stdout, base, x); fputc ('\n', stdout);
}

int
main (int argc, char **argv)
{
  mpq_t  op1, op2;
  mp_size_t size;
  int i;
  int reps = 10000;
  FILE *fp;
  int base;
  gmp_randstate_ptr rands;
  mpz_t bs;
  unsigned long bsi, size_range;
  size_t nread;

  tests_start ();
  rands = RANDS;

  mpz_init (bs);

  if (argc == 2)
    reps = atoi (argv[1]);

  mpq_init (op1);
  mpq_init (op2);

  fp = fopen (FILENAME, "w+");

  for (i = 0; i < reps; i++)
    {
      mpz_urandomb (bs, rands, 32);
      size_range = mpz_get_ui (bs) % 10 + 2;

      mpz_urandomb (bs, rands, size_range);
      size = mpz_get_ui (bs);
      mpz_errandomb (mpq_numref(op1), rands, 512L);
      mpz_errandomb_nonzero (mpq_denref(op1), rands, 512L);
      mpq_canonicalize (op1);

      mpz_urandomb (bs, rands, 1);
      bsi = mpz_get_ui (bs);
      if ((bsi & 1) != 0)
	mpq_neg (op1, op1);

      mpz_urandomb (bs, rands, 16);
      bsi = mpz_get_ui (bs);
      base = bsi % 36 + 1;
      if (base == 1)
	base = 0;

      rewind (fp);
      if (mpq_out_str (fp, base, op1) == 0
	  || putc (' ', fp) == EOF
	  || fflush (fp) != 0)
	{
	  printf ("mpq_out_str write error\n");
	  abort ();
	}

      rewind (fp);
      nread = mpq_inp_str (op2, fp, base);
      if (nread == 0)
	{
	  if (ferror (fp))
	    printf ("mpq_inp_str stream read error\n");
	  else
	    printf ("mpq_inp_str data conversion error\n");
	  abort ();
	}

      if (nread != ftell(fp))
	{
	  printf ("mpq_inp_str nread doesn't match ftell\n");
	  printf ("  nread  %lu\n", (unsigned long) nread);
	  printf ("  ftell  %ld\n", ftell(fp));
	  abort ();
	}

      if (mpq_cmp (op1, op2))
	{
	  printf ("ERROR\n");
	  printf ("op1  = "); debug_mp (op1, -16);
	  printf ("op2  = "); debug_mp (op2, -16);
	  printf ("base = %d\n", base);
	  abort ();
	}
    }

  fclose (fp);

  unlink (FILENAME);

  mpz_clear (bs);
  mpq_clear (op1);
  mpq_clear (op2);

  tests_end ();
  exit (0);
}
