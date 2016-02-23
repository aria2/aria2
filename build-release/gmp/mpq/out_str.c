/* mpq_out_str(stream,base,integer) */

/*
Copyright 2000, 2001 Free Software Foundation, Inc.

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

#include <stdio.h>
#include "gmp.h"
#include "gmp-impl.h"


size_t
mpq_out_str (FILE *stream, int base, mpq_srcptr q)
{
  size_t  written;

  if (stream == NULL)
    stream = stdout;

  written = mpz_out_str (stream, base, mpq_numref (q));

  if (mpz_cmp_ui (mpq_denref (q), 1) != 0)
    {
      putc ('/', stream);
      written += 1 + mpz_out_str (stream, base, mpq_denref (q));
    }

  return ferror (stream) ? 0 : written;
}
