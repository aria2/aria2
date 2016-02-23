/* mpq_swap (U, V) -- Swap U and V.

Copyright 1997, 1998, 2000, 2001 Free Software Foundation, Inc.

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

#include "gmp.h"
#include "gmp-impl.h"

void
mpq_swap (mpq_ptr u, mpq_ptr v) __GMP_NOTHROW
{
  mp_ptr up, vp;
  mp_size_t usize, vsize;
  mp_size_t ualloc, valloc;

  ualloc = ALLOC(NUM(u));
  valloc = ALLOC(NUM(v));
  ALLOC(NUM(v)) = ualloc;
  ALLOC(NUM(u)) = valloc;

  usize = SIZ(NUM(u));
  vsize = SIZ(NUM(v));
  SIZ(NUM(v)) = usize;
  SIZ(NUM(u)) = vsize;

  up = PTR(NUM(u));
  vp = PTR(NUM(v));
  PTR(NUM(v)) = up;
  PTR(NUM(u)) = vp;


  ualloc = ALLOC(DEN(u));
  valloc = ALLOC(DEN(v));
  ALLOC(DEN(v)) = ualloc;
  ALLOC(DEN(u)) = valloc;

  usize = SIZ(DEN(u));
  vsize = SIZ(DEN(v));
  SIZ(DEN(v)) = usize;
  SIZ(DEN(u)) = vsize;

  up = PTR(DEN(u));
  vp = PTR(DEN(v));
  PTR(DEN(v)) = up;
  PTR(DEN(u)) = vp;
}
