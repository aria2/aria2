/* Test mpf_sub.

Copyright 1996, 2001, 2004 Free Software Foundation, Inc.

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

#ifndef SIZE
#define SIZE 16
#endif

void
check_rand (int argc, char **argv)
{
  mp_size_t size;
  mp_exp_t exp;
  int reps = 20000;
  int i;
  mpf_t u, v, w, wref;
  mp_size_t bprec = 100;
  mpf_t rerr, max_rerr, limit_rerr;

  if (argc > 1)
    {
      reps = strtol (argv[1], 0, 0);
      if (argc > 2)
	bprec = strtol (argv[2], 0, 0);
    }

  mpf_set_default_prec (bprec);

  mpf_init_set_ui (limit_rerr, 1);
  mpf_div_2exp (limit_rerr, limit_rerr, bprec);
#if VERBOSE
  mpf_dump (limit_rerr);
#endif
  mpf_init (rerr);
  mpf_init_set_ui (max_rerr, 0);

  mpf_init (u);
  mpf_init (v);
  mpf_init (w);
  mpf_init (wref);
  for (i = 0; i < reps; i++)
    {
      size = urandom () % (2 * SIZE) - SIZE;
      exp = urandom () % SIZE;
      mpf_random2 (u, size, exp);

      size = urandom () % (2 * SIZE) - SIZE;
      exp = urandom () % SIZE;
      mpf_random2 (v, size, exp);

      if ((urandom () & 1) != 0)
	mpf_add_ui (u, v, 1);
      else if ((urandom () & 1) != 0)
	mpf_sub_ui (u, v, 1);

      mpf_sub (w, u, v);
      refmpf_sub (wref, u, v);

      mpf_reldiff (rerr, w, wref);
      if (mpf_cmp (rerr, max_rerr) > 0)
	{
	  mpf_set (max_rerr, rerr);
#if VERBOSE
	  mpf_dump (max_rerr);
#endif
	  if (mpf_cmp (rerr, limit_rerr) > 0)
	    {
	      printf ("ERROR after %d tests\n", i);
	      printf ("   u = "); mpf_dump (u);
	      printf ("   v = "); mpf_dump (v);
	      printf ("wref = "); mpf_dump (wref);
	      printf ("   w = "); mpf_dump (w);
	      abort ();
	    }
	}
    }

  mpf_clear (limit_rerr);
  mpf_clear (rerr);
  mpf_clear (max_rerr);

  mpf_clear (u);
  mpf_clear (v);
  mpf_clear (w);
  mpf_clear (wref);
}


void
check_data (void)
{
  static const struct {
    struct {
      int        exp, size;
      mp_limb_t  d[10];
    } x, y, want;

  } data[] = {
    { { 123, 2, { 8, 9 } },             { 123, 1, { 9 } }, { 122, 1, { 8 } } },

    /* f - f == 0, various sizes.
       These exercise a past problem (gmp 4.1.3 and earlier) where the
       result exponent was not zeroed on a zero result like this.  */
    { { 0, 0 }, { 0, 0 }, { 0, 0 } },
    { { 99, 1, { 1 } },             { 99, 1, { 1 } },             { 0, 0 } },
    { { 99, 2, { 123, 456 } },      { 99, 2, { 123, 456 } },      { 0, 0 } },
    { { 99, 3, { 123, 456, 789 } }, { 99, 3, { 123, 456, 789 } }, { 0, 0 } },

    /* High limbs cancel, leaving just the low limbs of the longer operand.
       This exercises a past problem (gmp 4.1.3 and earlier) where high zero
       limbs on the remainder were not stripped before truncating to the
       destination, causing loss of precision.  */
    { { 123, 2, { 8, 9 } },             { 123, 1, { 9 } }, { 122, 1, { 8 } } },
    { { 123, 3, { 8, 0, 9 } },          { 123, 1, { 9 } }, { 121, 1, { 8 } } },
    { { 123, 4, { 8, 0, 0, 9 } },       { 123, 1, { 9 } }, { 120, 1, { 8 } } },
    { { 123, 5, { 8, 0, 0, 0, 9 } },    { 123, 1, { 9 } }, { 119, 1, { 8 } } },
    { { 123, 6, { 8, 0, 0, 0, 0, 9 } }, { 123, 1, { 9 } }, { 118, 1, { 8 } } },

  };

  mpf_t  x, y, got, want;
  int  i, swap;

  mp_trace_base = 16;
  mpf_init (got);

  for (i = 0; i < numberof (data); i++)
    {
      for (swap = 0; swap <= 1; swap++)
        {
          PTR(x) = (mp_ptr) data[i].x.d;
          SIZ(x) = data[i].x.size;
          EXP(x) = data[i].x.exp;
          PREC(x) = numberof (data[i].x.d);
          MPF_CHECK_FORMAT (x);

          PTR(y) = (mp_ptr) data[i].y.d;
          SIZ(y) = data[i].y.size;
          EXP(y) = data[i].y.exp;
          PREC(y) = numberof (data[i].y.d);
          MPF_CHECK_FORMAT (y);

          PTR(want) = (mp_ptr) data[i].want.d;
          SIZ(want) = data[i].want.size;
          EXP(want) = data[i].want.exp;
          PREC(want) = numberof (data[i].want.d);
          MPF_CHECK_FORMAT (want);

          if (swap)
            {
              mpf_swap (x, y);
              SIZ(want) = - SIZ(want);
            }

          mpf_sub (got, x, y);
/*           MPF_CHECK_FORMAT (got); */

          if (mpf_cmp (got, want) != 0)
            {
              printf ("check_data() wrong reault at data[%d] (operands%s swapped)\n", i, swap ? "" : " not");
              mpf_trace ("x   ", x);
              mpf_trace ("y   ", y);
              mpf_trace ("got ", got);
              mpf_trace ("want", want);
              abort ();
            }
        }
    }

  mpf_clear (got);
}


int
main (int argc, char **argv)
{
  tests_start ();

  check_data ();
  check_rand (argc, argv);

  tests_end ();
  exit (0);
}
