dnl  AMD64 mpn_addlsh2_n -- rp[] = up[] + (vp[] << 2)
dnl  AMD64 mpn_rsblsh2_n -- rp[] = (vp[] << 2) - up[]

dnl  Contributed to the GNU project by Torbjorn Granlund.

dnl  Copyright 2008, 2010, 2011, 2012 Free Software Foundation, Inc.

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

define(LSH, 2)
define(RSH, 62)

ifdef(`OPERATION_addlsh2_n', `
	define(ADDSUB,	add)
	define(ADCSBB,	adc)
	define(func,	mpn_addlsh2_n)')
ifdef(`OPERATION_rsblsh2_n', `
	define(ADDSUB,	sub)
	define(ADCSBB,	sbb)
	define(func,	mpn_rsblsh2_n)')

MULFUNC_PROLOGUE(mpn_addlsh2_n mpn_rsblsh2_n)

ABI_SUPPORT(DOS64)
ABI_SUPPORT(STD64)

include_mpn(`x86_64/aorrlshC_n.asm')
