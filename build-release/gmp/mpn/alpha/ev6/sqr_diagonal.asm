dnl  Alpha mpn_sqr_diagonal.

dnl  Copyright 2001, 2002, 2006 Free Software Foundation, Inc.

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

C      cycles/limb
C EV4:      ?
C EV5:      ?
C EV6:      2.3

C  INPUT PARAMETERS
C  rp	r16
C  up	r17
C  n	r18


ASM_START()
PROLOGUE(mpn_sqr_diagonal)
	lda	r18, -2(r18)	C n -= 2
	ldq	r0,   0(r17)
	mulq	r0, r0, r4
	umulh	r0, r0, r20
	blt	r18, L(ex1)
	ldq	r1,   8(r17)
	mulq	r1, r1, r5
	umulh	r1, r1, r21
	beq	r18, L(ex2)
	lda	r18, -2(r18)	C n -= 2
	ldq	r0,  16(r17)
	blt	r18, L(ex3)
	ldq	r1,  24(r17)
	beq	r18, L(ex4)

	ALIGN(16)
L(top):	lda	r18, -2(r18)	C n -= 2
	stq	r4,   0(r16)
	mulq	r0, r0, r4
	stq	r20,  8(r16)
	umulh	r0, r0, r20
	ldq	r0,  32(r17)
	blt	r18, L(x)
	stq	r5,  16(r16)
	mulq	r1, r1, r5
	stq	r21, 24(r16)
	umulh	r1, r1, r21
	ldq	r1,  40(r17)
	lda	r16, 32(r16)	C rp += 4
	lda	r17, 16(r17)	C up += 2
	bne	r18, L(top)

	ALIGN(16)
L(ex4):	stq	r4,   0(r16)
	mulq	r0, r0, r4
	stq	r20,  8(r16)
	umulh	r0, r0, r20
	stq	r5,  16(r16)
	mulq	r1, r1, r5
	stq	r21, 24(r16)
	umulh	r1, r1, r21
	stq	r4,  32(r16)
	stq	r20, 40(r16)
	stq	r5,  48(r16)
	stq	r21, 56(r16)
	ret	r31, (r26), 1
	ALIGN(16)
L(x):	stq	r5,  16(r16)
	mulq	r1, r1, r5
	stq	r21, 24(r16)
	umulh	r1, r1, r21
	stq	r4,  32(r16)
	mulq	r0, r0, r4
	stq	r20, 40(r16)
	umulh	r0, r0, r20
	stq	r5,  48(r16)
	stq	r21, 56(r16)
	stq	r4,  64(r16)
	stq	r20, 72(r16)
	ret	r31, (r26), 1
L(ex1):	stq	r4,   0(r16)
	stq	r20,  8(r16)
	ret	r31, (r26), 1
	ALIGN(16)
L(ex2):	stq	r4,   0(r16)
	stq	r20,  8(r16)
	stq	r5,  16(r16)
	stq	r21, 24(r16)
	ret	r31, (r26), 1
	ALIGN(16)
L(ex3):	stq	r4,   0(r16)
	mulq	r0, r0, r4
	stq	r20,  8(r16)
	umulh	r0, r0, r20
	stq	r5,  16(r16)
	stq	r21, 24(r16)
	stq	r4,  32(r16)
	stq	r20, 40(r16)
	ret	r31, (r26), 1
EPILOGUE()
ASM_END()
