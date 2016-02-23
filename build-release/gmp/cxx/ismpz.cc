/* operator>> -- C++-style input of mpz_t.

Copyright 2001, 2003 Free Software Foundation, Inc.

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

#include <cctype>
#include <iostream>
#include <string>
#include "gmp.h"
#include "gmp-impl.h"

using namespace std;


// For g++ libstdc++ parsing see num_get<chartype,initer>::_M_extract_int in
// include/bits/locale_facets.tcc.

istream &
operator>> (istream &i, mpz_ptr z)
{
  char c = 0;
  i.get(c); // start reading

  if (i.flags() & ios::skipws) // skip initial whitespace
    {
#if HAVE_STD__LOCALE
      const ctype<char>& ct = use_facet< ctype<char> >(i.getloc());
#define cxx_isspace(c)  (ct.is(ctype_base::space,(c)))
#else
#define cxx_isspace(c)  isspace(c)
#endif

      while (cxx_isspace(c) && i.get(c))
        ;
    }

  return __gmpz_operator_in_nowhite (i, z, c);
}
