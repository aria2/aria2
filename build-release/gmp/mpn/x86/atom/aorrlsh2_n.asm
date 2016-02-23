dnl  Intel Atom mpn_addlsh2_n/mpn_rsblsh2_n -- rp[] = (vp[] << 2) +- up[]

dnl  Contributed to the GNU project by Marco Bodrato.

dnl  Copyright 2011 Free Software Foundation, Inc.
dnl
dnl  This file is part of the GNU MP Library.
dnl
dnl  The GNU MP Library is free software; you can redistribute it and/or
dnl  modify it under the terms of the GNU Lesser General Public License as
dnl  published by the Free Software Foundation; either version 3 of the
dnl  License, or (at your option) any later version.
dnl
dnl  The GNU MP Library is distributed in the hope that it will be useful,
dnl  but WITHOUT ANY WARRANTY; without even the implied warranty of
dnl  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
dnl  Lesser General Public License for more details.
dnl
dnl  You should have received a copy of the GNU Lesser General Public License
dnl  along with the GNU MP Library.  If not, see http://www.gnu.org/licenses/.

include(`../config.m4')

define(LSH, 2)
define(RSH, 30)

ifdef(`OPERATION_addlsh2_n', `
	define(M4_inst,        adcl)
	define(M4_opp,         subl)
	define(M4_function,    mpn_addlsh2_n)
	define(M4_function_c,  mpn_addlsh2_nc)
',`ifdef(`OPERATION_rsblsh2_n', `
	define(M4_inst,        sbbl)
	define(M4_opp,         addl)
	define(M4_function,    mpn_rsblsh2_n)
	define(M4_function_c,  mpn_rsblsh2_nc)
',`m4_error(`Need OPERATION_addlsh2_n or OPERATION_rsblsh2_n
')')')

MULFUNC_PROLOGUE(mpn_addlsh2_n mpn_addlsh2_nc mpn_rsblsh2_n mpn_rsblsh2_nc)

include_mpn(`x86/atom/aorrlshC_n.asm')
