/* Test mpf_set_ui and mpf_init_set_ui.

Copyright 2000, 2001, 2003 Free Software Foundation, Inc.

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

void
check_data (void)
{
  static const struct {
    unsigned long  x;
    mp_size_t      want_size;
    mp_limb_t      want_data[2];
  } data[] = {

    {  0L,  0 },
    {  1L,  1, { 1 } },

#if GMP_NUMB_BITS >= BITS_PER_ULONG
    { ULONG_MAX,     1, { ULONG_MAX, 0 } },
    { ULONG_HIGHBIT, 1, { ULONG_HIGHBIT, 0 } },
#else
    { ULONG_MAX,     2, { ULONG_MAX & GMP_NUMB_MASK,
                          ULONG_MAX >> GMP_NUMB_BITS } },
    { ULONG_HIGHBIT, 2, { 0,
                          ULONG_HIGHBIT >> GMP_NUMB_BITS } },
#endif
  };

  mpf_t  x;
  int    i;

  for (i = 0; i < numberof (data); i++)
    {
      mpf_init (x);
      mpf_set_ui (x, data[i].x);
      MPF_CHECK_FORMAT (x);
      if (x->_mp_size != data[i].want_size
          || refmpn_cmp_allowzero (x->_mp_d, data[i].want_data,
                                   ABS (data[i].want_size)) != 0
          || x->_mp_exp != ABS (data[i].want_size))
        {
          printf ("mpf_set_ui wrong on data[%d]\n", i);
          abort();
        }
      mpf_clear (x);

      mpf_init_set_ui (x, data[i].x);
      MPF_CHECK_FORMAT (x);
      if (x->_mp_size != data[i].want_size
          || refmpn_cmp_allowzero (x->_mp_d, data[i].want_data,
                                   ABS (data[i].want_size)) != 0
          || x->_mp_exp != ABS (data[i].want_size))
        {
          printf ("mpf_init_set_ui wrong on data[%d]\n", i);
          abort();
        }
      mpf_clear (x);
    }
}

int
main (void)
{
  tests_start ();

  check_data ();

  tests_end ();
  exit (0);
}
