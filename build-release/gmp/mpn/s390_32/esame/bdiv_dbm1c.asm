dnl  S/390-32 mpn_bdiv_dbm1c for systems with MLR instruction.

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
C z990		10
C z9		 ?
C z10		 ?
C z196		 ?

C INPUT PARAMETERS
define(`qp',	  `%r2')
define(`up',	  `%r3')
define(`n',	  `%r4')
define(`bd',	  `%r5')
define(`cy',	  `%r6')

ASM_START()
	TEXT
	ALIGN(16)
PROLOGUE(mpn_bdiv_dbm1c)
	stm	%r6, %r7, 24(%r15)
	lhi	%r7, 0			C zero index register

L(top):	l	%r1, 0(%r7,up)
	mlr	%r0, bd
	slr	%r6, %r1
	st	%r6, 0(%r7,qp)
	slbr	%r6, %r0
	la	%r7, 4(%r7)
	brct	n, L(top)

	lr	%r2, %r6
	lm	%r6, %r7, 24(%r15)
	br	%r14
EPILOGUE()
