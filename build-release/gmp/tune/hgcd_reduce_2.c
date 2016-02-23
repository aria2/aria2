/* mpn/generic/hgcd_reduce.c forced to use hgcd_appr. */

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

#undef  HGCD_REDUCE_THRESHOLD
#define HGCD_REDUCE_THRESHOLD 0
#define __gmpn_hgcd_reduce mpn_hgcd_reduce_2
#define __gmpn_hgcd_reduce_itch mpn_hgcd_reduce_2_itch

#include "../mpn/generic/hgcd_reduce.c"
