dnl  PowerPC-64 mpn_mul_1 -- Multiply a limb vector with a limb and add
dnl  the result to a second limb vector.

dnl  Copyright 1999, 2000, 2001, 2003, 2005 Free Software Foundation, Inc.

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

C		cycles/limb
C POWER3/PPC630:     ?
C POWER4/PPC970:     10

C INPUT PARAMETERS
C rp	r3
C up	r4
C n	r5
C v	r6,r7  or  r7,r8

ASM_START()
PROLOGUE(mpn_mul_1)

ifdef(`BROKEN_LONGLONG_PARAM',
`	rldimi	r8, r7, 32,0	C assemble vlimb from separate 32-bit arguments
	mr	r6, r8
',`
	rldimi	r7, r6, 32,0	C assemble vlimb from separate 32-bit arguments
	mr	r6, r7
')
	li	r7, 0		C cy_limb = 0
	mtctr	r5
	addic	r0, r0, 0
	addi	r3, r3, -8
	addi	r4, r4, -8

L(oop):	ldu	r0, 8(r4)
	mulld	r9, r0, r6
	adde	r12, r9, r7	C add old high limb and new low limb
	srdi	r5, r9, 32
	srdi	r11, r7, 32
	adde	r5, r5, r11	C add high limb parts, set cy
	mulhdu	r7, r0, r6
	stdu	r12, 8(r3)
	bdnz	L(oop)

	addze	r4, r7
	srdi	r3, r4, 32
	blr
EPILOGUE()
