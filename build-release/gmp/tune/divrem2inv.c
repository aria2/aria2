/* mpn/generic/divrem_2.c forced to use udiv_qrnnd_preinv. */

/*
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

#include "gmp.h"
#include "gmp-impl.h"

#ifdef DIVREM_2_THRESHOLD
#undef DIVREM_2_THRESHOLD
#endif
#define DIVREM_2_THRESHOLD  0
#define __gmpn_divrem_2     mpn_divrem_2_inv

#include "mpn/generic/divrem_2.c"
