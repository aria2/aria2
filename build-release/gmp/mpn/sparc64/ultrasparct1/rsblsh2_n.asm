dnl  SPARC v9 mpn_rsblsh2_n for T1/T2.

dnl  Copyright 2010 Free Software Foundation, Inc.

dnl  This file is part of the GNU MP Library.

dnl  The GNU MP Library is free software; you can redistribute it and/or modify
dnl  it under the terms of the GNU Lesser General Public License as published
dnl  by the Free Software Foundation; either version 3 of the License, or (at
dnl  your option) any later version.

dnl  The GNU MP Library is distributed in the hope that it will be useful, but
dnl  WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
dnl  or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public
dnl  License for more details.

dnl  You should have received a copy of the GNU Lesser General Public License
dnl  along with the GNU MP Library.  If not, see http://www.gnu.org/licenses/.

include(`../config.m4')


define(LSH,             2)
define(RSH,             62)

define(func, mpn_rsblsh2_n)

MULFUNC_PROLOGUE(mpn_rsblsh2_n)

include_mpn(`sparc64/ultrasparct1/rsblshC_n.asm')
