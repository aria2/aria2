dnl  Copyright 1999, 2001 Free Software Foundation, Inc.

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

ASM_START()
PROLOGUE(mpn_umul_ppmm)
C	.callinfo frame=64,no_calls

	ldo	64(%r30),%r30
	stw	%r25,-16(0,%r30)
	fldws	-16(0,%r30),%fr22R
	stw	%r24,-16(0,%r30)
	fldws	-16(0,%r30),%fr22L
	xmpyu	%fr22R,%fr22L,%fr22
	fstds	%fr22,-16(0,%r30)
	ldw	-16(0,%r30),%r28
	ldw	-12(0,%r30),%r29
	stw	%r29,0(0,%r26)
	bv	0(%r2)
	ldo	-64(%r30),%r30
EPILOGUE()
