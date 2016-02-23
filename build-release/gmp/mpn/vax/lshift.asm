dnl  VAX mpn_lshift -- left shift.

dnl  Copyright 1999, 2000, 2001, 2012 Free Software Foundation, Inc.

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
PROLOGUE(mpn_lshift)
	.word	0x1c0
	movl	4(ap), r7
	movl	8(ap), r6
	movl	12(ap), r1
	movl	16(ap), r8

	moval	(r6)[r1], r6
	moval	(r7)[r1], r7
	clrl	r3
	movl	-(r6), r2
	ashq	r8, r2, r4
	movl	r5, r0
	movl	r2, r3
	decl	r1
	jeql	L(end)

L(top):	movl	-(r6), r2
	ashq	r8, r2, r4
	movl	r5, -(r7)
	movl	r2, r3
	sobgtr	r1, L(top)

L(end):	movl	r4, -4(r7)
	ret
EPILOGUE()
