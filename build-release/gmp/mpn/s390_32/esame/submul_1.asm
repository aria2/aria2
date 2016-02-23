dnl  S/390-32 mpn_submul_1 for systems with MLR instruction.

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
C z900		20
C z990		11
C z9		 ?
C z10		 ?
C z196		 ?

C INPUT PARAMETERS
define(`rp',	`%r2')
define(`up',	`%r3')
define(`n',	`%r4')
define(`v0',	`%r5')

ASM_START()
PROLOGUE(mpn_submul_1)
	stm	%r9, %r12, 36(%r15)
	lhi	%r12, 0
	slr	%r11, %r11

L(top):	l	%r1, 0(%r12, up)
	l	%r10, 0(%r12, rp)
	mlr	%r0, v0
	slbr	%r10, %r1
	slbr	%r9, %r9
	slr	%r0, %r9		C conditional incr
	slr	%r10, %r11
	lr	%r11, %r0
	st	%r10, 0(%r12, rp)
	la	%r12, 4(%r12)
	brct	%r4,  L(top)

	lr	%r2, %r11
	slbr	%r9, %r9
	slr	%r2, %r9

	lm	%r9, %r12, 36(%r15)
	br	%r14
EPILOGUE()
