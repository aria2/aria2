/* mpn/generic/mod_1.c forced to use plain udiv_qrnnd.

Copyright 2000, 2003 Free Software Foundation, Inc.

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

#define OPERATION_mod_1

#include "gmp.h"
#include "gmp-impl.h"

#undef MOD_1_NORM_THRESHOLD
#undef MOD_1_UNNORM_THRESHOLD
#undef MOD_1N_TO_MOD_1_1_THRESHOLD
#undef MOD_1U_TO_MOD_1_1_THRESHOLD
#define MOD_1_NORM_THRESHOLD    MP_SIZE_T_MAX
#define MOD_1_UNNORM_THRESHOLD  MP_SIZE_T_MAX
#define MOD_1N_TO_MOD_1_1_THRESHOLD MP_SIZE_T_MAX
#define MOD_1U_TO_MOD_1_1_THRESHOLD MP_SIZE_T_MAX
#define __gmpn_mod_1  mpn_mod_1_div

#include "mpn/generic/mod_1.c"
