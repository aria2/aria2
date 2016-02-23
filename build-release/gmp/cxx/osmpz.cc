/* operator<< -- mpz formatted output to an ostream.

Copyright 2001 Free Software Foundation, Inc.

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

#include <iostream>
#include <stdarg.h>    /* for va_list and hence doprnt_funs_t */
#include <string.h>

#include "gmp.h"
#include "gmp-impl.h"

using namespace std;


ostream&
operator<< (ostream &o, mpz_srcptr z)
{
  struct doprnt_params_t  param;
  __gmp_doprnt_params_from_ios (&param, o);
  return __gmp_doprnt_integer_ostream (o, &param,
                                       mpz_get_str (NULL, param.base, z));
}
