/* mpn/generic/mod_1_1.c method 1.

Copyright 2011 Free Software Foundation, Inc.

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

#undef MOD_1_1P_METHOD
#define MOD_1_1P_METHOD 1
#undef mpn_mod_1_1p
#undef mpn_mod_1_1p_cps
#define mpn_mod_1_1p mpn_mod_1_1p_1
#define mpn_mod_1_1p_cps mpn_mod_1_1p_cps_1

#include "mpn/generic/mod_1_1.c"
