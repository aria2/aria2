/* mpq_set_str -- string to mpq conversion.

Copyright 2001, 2002 Free Software Foundation, Inc.

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
#include <string.h>
#include "gmp.h"
#include "gmp-impl.h"


/* FIXME: Would like an mpz_set_mem (or similar) accepting a pointer and
   length so we wouldn't have to copy the numerator just to null-terminate
   it.  */

int
mpq_set_str (mpq_ptr q, const char *str, int base)
{
  const char  *slash;
  char        *num;
  size_t      numlen;
  int         ret;

  slash = strchr (str, '/');
  if (slash == NULL)
    {
      SIZ(DEN(q)) = 1;
      PTR(DEN(q))[0] = 1;

      return mpz_set_str (mpq_numref(q), str, base);
    }

  numlen = slash - str;
  num = __GMP_ALLOCATE_FUNC_TYPE (numlen+1, char);
  memcpy (num, str, numlen);
  num[numlen] = '\0';
  ret = mpz_set_str (mpq_numref(q), num, base);
  (*__gmp_free_func) (num, numlen+1);

  if (ret != 0)
    return ret;

  return mpz_set_str (mpq_denref(q), slash+1, base);
}
