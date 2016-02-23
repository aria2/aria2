dnl  PowerPC-64 mpn_sqr_diagonal.

dnl  Copyright 2001, 2002, 2003, 2005, 2006, 20010 Free Software Foundation,
dnl  Inc.

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

C		    cycles/limb
C POWER3/PPC630		18
C POWER4/PPC970		 ?
C POWER5		 7.25
C POWER6		 9.5

C INPUT PARAMETERS
define(`rp',  r3)
define(`up',  r4)
define(`n',   r5)

ASM_START()
PROLOGUE(mpn_sqr_diagonal)
ifdef(`HAVE_ABI_mode32',
`	rldicl	n, n, 0, 32')		C zero extend n

	rldicl.	r0, n, 0,62		C r0 = n & 3, set cr0
	addi	n, n, 3			C compute count...
	cmpdi	cr6, r0, 2
	srdi	n, n, 2			C ...for ctr
	mtctr	n			C copy count into ctr
	beq	cr0, L(b00)
	blt	cr6, L(b01)
	beq	cr6, L(b10)

L(b11):	ld	r0, 0(up)
	ld	r10, 8(up)
	ld	r12, 16(up)
	addi	rp, rp, -16
	mulld	r7, r0, r0
	mulhdu	r8, r0, r0
	mulld	r9, r10, r10
	mulhdu	r10, r10, r10
	mulld	r11, r12, r12
	mulhdu	r12, r12, r12
	addi	up, up, 24
	b	L(11)

	ALIGN(16)
L(b01):	ld	r0, 0(up)
	addi	rp, rp, -48
	addi	up, up, 8
	mulld	r11, r0, r0
	mulhdu	r12, r0, r0
	b	L(01)

	ALIGN(16)
L(b10):	ld	r0, 0(up)
	ld	r12, 8(up)
	addi	rp, rp, -32
	addi	up, up, 16
	mulld	r9, r0, r0
	mulhdu	r10, r0, r0
	mulld	r11, r12, r12
	mulhdu	r12, r12, r12
	b	L(10)

	ALIGN(32)
L(b00):
L(top):	ld	r0, 0(up)
	ld	r8, 8(up)
	ld	r10, 16(up)
	ld	r12, 24(up)
	mulld	r5, r0, r0
	mulhdu	r6, r0, r0
	mulld	r7, r8, r8
	mulhdu	r8, r8, r8
	mulld	r9, r10, r10
	mulhdu	r10, r10, r10
	mulld	r11, r12, r12
	mulhdu	r12, r12, r12
	addi	up, up, 32
	std	r5, 0(rp)
	std	r6, 8(rp)
L(11):	std	r7, 16(rp)
	std	r8, 24(rp)
L(10):	std	r9, 32(rp)
	std	r10, 40(rp)
L(01):	std	r11, 48(rp)
	std	r12, 56(rp)
	addi	rp, rp, 64
	bdnz	L(top)

	blr
EPILOGUE()
