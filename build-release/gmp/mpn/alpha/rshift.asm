dnl  Alpha mpn_rshift -- Shift a number right.

dnl  Copyright 1994, 1995, 2000, 2009 Free Software Foundation, Inc.

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
C EV4:     ?
C EV5:     3.25
C EV6:     1.75

C  INPUT PARAMETERS
C  rp	r16
C  up	r17
C  n	r18
C  cnt	r19


ASM_START()
PROLOGUE(mpn_rshift)
	ldq	r4,0(r17)	C load first limb
	subq	r31,r19,r20
	subq	r18,1,r18
	and	r18,4-1,r28	C number of limbs in first loop
	sll	r4,r20,r0	C compute function result

	beq	r28,L(L0)
	subq	r18,r28,r18

	ALIGN(8)
L(top0):
	ldq	r3,8(r17)
	addq	r16,8,r16
	srl	r4,r19,r5
	addq	r17,8,r17
	subq	r28,1,r28
	sll	r3,r20,r6
	bis	r3,r3,r4
	bis	r5,r6,r8
	stq	r8,-8(r16)
	bne	r28,L(top0)

L(L0):	srl	r4,r19,r24
	beq	r18,L(end)
C warm up phase 1
	ldq	r1,8(r17)
	subq	r18,4,r18
	ldq	r2,16(r17)
	ldq	r3,24(r17)
	ldq	r4,32(r17)
C warm up phase 2
	sll	r1,r20,r7
	srl	r1,r19,r21
	sll	r2,r20,r8
	beq	r18,L(end1)
	ldq	r1,40(r17)
	srl	r2,r19,r22
	ldq	r2,48(r17)
	sll	r3,r20,r5
	bis	r7,r24,r7
	srl	r3,r19,r23
	bis	r8,r21,r8
	sll	r4,r20,r6
	ldq	r3,56(r17)
	srl	r4,r19,r24
	ldq	r4,64(r17)
	subq	r18,4,r18
	beq	r18,L(end2)
	ALIGN(16)
C main loop
L(top):	stq	r7,0(r16)
	bis	r5,r22,r5
	stq	r8,8(r16)
	bis	r6,r23,r6

	sll	r1,r20,r7
	subq	r18,4,r18
	srl	r1,r19,r21
	unop	C ldq	r31,-96(r17)

	sll	r2,r20,r8
	ldq	r1,72(r17)
	srl	r2,r19,r22
	ldq	r2,80(r17)

	stq	r5,16(r16)
	bis	r7,r24,r7
	stq	r6,24(r16)
	bis	r8,r21,r8

	sll	r3,r20,r5
	unop	C ldq	r31,-96(r17)
	srl	r3,r19,r23
	addq	r16,32,r16

	sll	r4,r20,r6
	ldq	r3,88(r17)
	srl	r4,r19,r24
	ldq	r4,96(r17)

	addq	r17,32,r17
	bne	r18,L(top)
C cool down phase 2/1
L(end2):
	stq	r7,0(r16)
	bis	r5,r22,r5
	stq	r8,8(r16)
	bis	r6,r23,r6
	sll	r1,r20,r7
	srl	r1,r19,r21
	sll	r2,r20,r8
	srl	r2,r19,r22
	stq	r5,16(r16)
	bis	r7,r24,r7
	stq	r6,24(r16)
	bis	r8,r21,r8
	sll	r3,r20,r5
	srl	r3,r19,r23
	sll	r4,r20,r6
	srl	r4,r19,r24
C cool down phase 2/2
	stq	r7,32(r16)
	bis	r5,r22,r5
	stq	r8,40(r16)
	bis	r6,r23,r6
	stq	r5,48(r16)
	stq	r6,56(r16)
C cool down phase 2/3
	stq	r24,64(r16)
	ret	r31,(r26),1

C cool down phase 1/1
L(end1):
	srl	r2,r19,r22
	sll	r3,r20,r5
	bis	r7,r24,r7
	srl	r3,r19,r23
	bis	r8,r21,r8
	sll	r4,r20,r6
	srl	r4,r19,r24
C cool down phase 1/2
	stq	r7,0(r16)
	bis	r5,r22,r5
	stq	r8,8(r16)
	bis	r6,r23,r6
	stq	r5,16(r16)
	stq	r6,24(r16)
	stq	r24,32(r16)
	ret	r31,(r26),1

L(end):	stq	r24,0(r16)
	ret	r31,(r26),1
EPILOGUE(mpn_rshift)
ASM_END()
