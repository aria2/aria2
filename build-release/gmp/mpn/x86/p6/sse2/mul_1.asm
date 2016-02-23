dnl  Intel P6/SSE2 mpn_mul_1.

dnl  Copyright 2008 Free Software Foundation, Inc.
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

C TODO
C  * Write P6 specific SSE2 code.  It should reach 3 c/l.
C    The Pentium4 code runs at 4.2 c/l.

MULFUNC_PROLOGUE(mpn_mul_1)
include_mpn(`x86/pentium4/sse2/mul_1.asm')
