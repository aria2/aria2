dnl  AMD64 mpn_addlsh1_n, mpn_sublsh1_n -- rp[] = up[] +- (vp[] << 1),
dnl  optimised for Pentium 4.

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

define(LSH, 1)
define(RSH, 31)			C 31, not 63, since we use 32-bit ops

ifdef(`OPERATION_addlsh1_n', `
  define(ADDSUB,	add)
  define(func,		mpn_addlsh1_n)')
ifdef(`OPERATION_sublsh1_n', `
  define(ADDSUB,	sub)
  define(func,		mpn_sublsh1_n)')

ABI_SUPPORT(DOS64)
ABI_SUPPORT(STD64)

MULFUNC_PROLOGUE(mpn_addlsh1_n mpn_sublsh1_n)
include_mpn(`x86_64/pentium4/aorslshC_n.asm')
