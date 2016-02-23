dnl  S/390-32 mpn_mul_1 for systems with MLR instruction

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
C z900		14
C z990		 9
C z9		 ?
C z10		 ?
C z196		 ?

C INPUT PARAMETERS
define(`rp',	`%r2')
define(`up',	`%r3')
define(`n',	`%r4')
define(`v0',	`%r5')

ASM_START()
PROLOGUE(mpn_mul_1)
	stm	%r11, %r12, 44(%r15)
	lhi	%r12, 0			C zero index register
	ahi	%r12, 0			C clear carry flag
	lhi	%r11, 0			C clear carry limb

L(top):	l	%r1, 0(%r12,up)
	mlr	%r0, v0
	alcr	%r1, %r11
	lr	%r11, %r0		C copy high part to carry limb
	st	%r1, 0(%r12,rp)
	la	%r12, 4(%r12)
	brct	n, L(top)

	lhi	%r2, 0
	alcr	%r2, %r11

	lm	%r11, %r12, 44(%r15)
	br	%r14
EPILOGUE()
