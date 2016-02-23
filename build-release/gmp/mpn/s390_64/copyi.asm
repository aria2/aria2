dnl  S/390-64 mpn_copyi

dnl  Copyright 2011 Free Software Foundation, Inc.

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

C            cycles/limb
C z900		 1.25
C z990           0.75
C z9		 ?
C z10		 ?
C z196		 ?

C NOTE
C  * This is based on GNU libc memcpy which was written by Martin Schwidefsky.

C INPUT PARAMETERS
define(`rp',	`%r2')
define(`up',	`%r3')
define(`n',	`%r4')

ASM_START()
PROLOGUE(mpn_copyi)
	ltgr	%r4, %r4
	sllg	%r4, %r4, 3
	je	L(rtn)
	aghi	%r4, -1
	srlg	%r5, %r4, 8
	ltgr	%r5, %r5		C < 256 bytes to copy?
	je	L(1)

L(top):	mvc	0(256, rp), 0(up)
	la	rp, 256(rp)
	la	up, 256(up)
	brctg	%r5, L(top)

L(1):	bras	%r5, L(2)		C make r5 point to mvc insn
	mvc	0(1, rp), 0(up)
L(2):	ex	%r4, 0(%r5)		C execute mvc with length ((n-1) mod 256)+1
L(rtn):	br	%r14
EPILOGUE()
