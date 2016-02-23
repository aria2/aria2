dnl  PowerPC mftb_function -- read time base registers.

dnl  Copyright 2002 Free Software Foundation, Inc.
dnl
dnl  This file is part of the GNU MP Library.
dnl
dnl  The GNU MP Library is free software; you can redistribute it and/or
dnl  modify it under the terms of the GNU Lesser General Public License as
dnl  published by the Free Software Foundationn; either version 3 of the
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


C void mftb_function (unsigned a[2]);
C

ASM_START()
PROLOGUE(mftb_function)

	C r3	a

L(again):
	mftbu	r4
	mftb	r5
	mftbu	r6
	cmpw	cr0, r4, r6
	bne	L(again)

	stw	r5, 0(r3)
	stw	r4, 4(r3)
	blr

EPILOGUE()
