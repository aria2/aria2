/* mpn_set_str_basecase -- mpn_set_str forced to its basecase.

Copyright 2002 Free Software Foundation, Inc.

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

#define __gmpn_set_str mpn_set_str_basecase
#define __gmpn_bc_set_str mpn_bc_set_str_basecase
#define __gmpn_dc_set_str mpn_dc_set_str_basecase
#define __gmpn_set_str_compute_powtab mpn_set_str_compute_powtab_basecase

#include "gmp.h"
#include "gmp-impl.h"

#ifndef SIZE_T_MAX
#define SIZE_T_MAX  ((size_t) ULONG_MAX)
#endif

#undef SET_STR_DC_THRESHOLD
#define SET_STR_DC_THRESHOLD           SIZE_T_MAX /* always */
#undef SET_STR_PRECOMPUTE_THRESHOLD
#define SET_STR_PRECOMPUTE_THRESHOLD   SIZE_T_MAX /* always */

#include "mpn/generic/set_str.c"
