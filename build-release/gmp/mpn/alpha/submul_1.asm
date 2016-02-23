dnl  Alpha mpn_submul_1 -- Multiply a limb vector with a limb and subtract
dnl  the result from a second limb vector.

dnl  Copyright 1992, 1994, 1995, 2000, 2002 Free Software Foundation, Inc.

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
C EV4:     42
C EV5:     18
C EV6:      7

C  INPUT PARAMETERS
C  rp	r16
C  up	r17
C  n	r18
C  limb	r19


ASM_START()
PROLOGUE(mpn_submul_1)
	ldq	r2,0(r17)	C r2 = s1_limb
	addq	r17,8,r17	C s1_ptr++
	subq	r18,1,r18	C size--
	mulq	r2,r19,r3	C r3 = prod_low
	ldq	r5,0(r16)	C r5 = *res_ptr
	umulh	r2,r19,r0	C r0 = prod_high
	beq	r18,$Lend1	C jump if size was == 1
	ldq	r2,0(r17)	C r2 = s1_limb
	addq	r17,8,r17	C s1_ptr++
	subq	r18,1,r18	C size--
	subq	r5,r3,r3
	cmpult	r5,r3,r4
	stq	r3,0(r16)
	addq	r16,8,r16	C res_ptr++
	beq	r18,$Lend2	C jump if size was == 2

	ALIGN(8)
$Loop:	mulq	r2,r19,r3	C r3 = prod_low
	ldq	r5,0(r16)	C r5 = *res_ptr
	addq	r4,r0,r0	C cy_limb = cy_limb + 'cy'
	subq	r18,1,r18	C size--
	umulh	r2,r19,r4	C r4 = cy_limb
	ldq	r2,0(r17)	C r2 = s1_limb
	addq	r17,8,r17	C s1_ptr++
	addq	r3,r0,r3	C r3 = cy_limb + prod_low
	cmpult	r3,r0,r0	C r0 = carry from (cy_limb + prod_low)
	subq	r5,r3,r3
	cmpult	r5,r3,r5
	stq	r3,0(r16)
	addq	r16,8,r16	C res_ptr++
	addq	r5,r0,r0	C combine carries
	bne	r18,$Loop

$Lend2:	mulq	r2,r19,r3	C r3 = prod_low
	ldq	r5,0(r16)	C r5 = *res_ptr
	addq	r4,r0,r0	C cy_limb = cy_limb + 'cy'
	umulh	r2,r19,r4	C r4 = cy_limb
	addq	r3,r0,r3	C r3 = cy_limb + prod_low
	cmpult	r3,r0,r0	C r0 = carry from (cy_limb + prod_low)
	subq	r5,r3,r3
	cmpult	r5,r3,r5
	stq	r3,0(r16)
	addq	r5,r0,r0	C combine carries
	addq	r4,r0,r0	C cy_limb = prod_high + cy
	ret	r31,(r26),1
$Lend1:	subq	r5,r3,r3
	cmpult	r5,r3,r5
	stq	r3,0(r16)
	addq	r0,r5,r0
	ret	r31,(r26),1
EPILOGUE(mpn_submul_1)
ASM_END()
