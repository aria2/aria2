dnl  Alpha mpn_sqr_diagonal.

dnl  Copyright 2001, 2002 Free Software Foundation, Inc.

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
C EV6:      3.45

C  INPUT PARAMETERS
C  rp	r16
C  up	r17
C  n	r18


ASM_START()
PROLOGUE(mpn_sqr_diagonal)
	ldq	r2,0(r17)	C r2 = s1_limb
	lda	r18,-2(r18)	C size -= 2
	mulq	r2,r2,r3	C r3 = prod_low
	umulh	r2,r2,r4	C r4 = prod_high
	blt	r18,$Lend1	C jump if size was == 1
	ldq	r2,8(r17)	C r2 = s1_limb
	beq	r18,$Lend2	C jump if size was == 2

	ALIGN(8)
$Loop:	stq	r3,0(r16)
	mulq	r2,r2,r3	C r3 = prod_low
	lda	r18,-1(r18)	C size--
	stq	r4,8(r16)
	umulh	r2,r2,r4	C r4 = cy_limb
	ldq	r2,16(r17)	C r2 = s1_limb
	lda	r17,8(r17)	C s1_ptr++
	lda	r16,16(r16)	C res_ptr++
	bne	r18,$Loop

$Lend2:	stq	r3,0(r16)
	mulq	r2,r2,r3	C r3 = prod_low
	stq	r4,8(r16)
	umulh	r2,r2,r4	C r4 = cy_limb
	stq	r3,16(r16)
	stq	r4,24(r16)
	ret	r31,(r26),1
$Lend1:	stq	r3,0(r16)
	stq	r4,8(r16)
	ret	r31,(r26),1
EPILOGUE(mpn_sqr_diagonal)
ASM_END()
