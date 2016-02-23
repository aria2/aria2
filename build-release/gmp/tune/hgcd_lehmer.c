/* mpn/generic/hgcd.c forced to use Lehmer's quadratic algorithm. */

/*
Copyright 2010 Free Software Foundation, Inc.

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

#undef  HGCD_THRESHOLD
#define HGCD_THRESHOLD MP_SIZE_T_MAX
#define __gmpn_hgcd  mpn_hgcd_lehmer
#define __gmpn_hgcd_itch mpn_hgcd_lehmer_itch

#include "../mpn/generic/hgcd.c"
