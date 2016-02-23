dnl  IBM POWER mpn_submul_1 -- Multiply a limb vector with a limb and subtract
dnl  the result from a second limb vector.

dnl  Copyright 1992, 1994, 1999, 2000, 2001 Free Software Foundation, Inc.

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


dnl  INPUT PARAMETERS
dnl  res_ptr	r3
dnl  s1_ptr	r4
dnl  size	r5
dnl  s2_limb	r6

dnl  The POWER architecture has no unsigned 32x32->64 bit multiplication
dnl  instruction.  To obtain that operation, we have to use the 32x32->64
dnl  signed multiplication instruction, and add the appropriate compensation to
dnl  the high limb of the result.  We add the multiplicand if the multiplier
dnl  has its most significant bit set, and we add the multiplier if the
dnl  multiplicand has its most significant bit set.  We need to preserve the
dnl  carry flag between each iteration, so we have to compute the compensation
dnl  carefully (the natural, srai+and doesn't work).  Since all POWER can
dnl  branch in zero cycles, we use conditional branches for the compensation.

include(`../config.m4')

ASM_START()
PROLOGUE(mpn_submul_1)
	cal	3,-4(3)
	l	0,0(4)
	cmpi	0,6,0
	mtctr	5
	mul	9,0,6
	srai	7,0,31
	and	7,7,6
	mfmq	11
	cax	9,9,7
	l	7,4(3)
	sf	8,11,7		C add res_limb
	a	11,8,11		C invert cy (r11 is junk)
	blt	Lneg
Lpos:	bdz	Lend

Lploop:	lu	0,4(4)
	stu	8,4(3)
	cmpi	0,0,0
	mul	10,0,6
	mfmq	0
	ae	11,0,9		C low limb + old_cy_limb + old cy
	l	7,4(3)
	aze	10,10		C propagate cy to new cy_limb
	sf	8,11,7		C add res_limb
	a	11,8,11		C invert cy (r11 is junk)
	bge	Lp0
	cax	10,10,6		C adjust high limb for negative limb from s1
Lp0:	bdz	Lend0
	lu	0,4(4)
	stu	8,4(3)
	cmpi	0,0,0
	mul	9,0,6
	mfmq	0
	ae	11,0,10
	l	7,4(3)
	aze	9,9
	sf	8,11,7
	a	11,8,11		C invert cy (r11 is junk)
	bge	Lp1
	cax	9,9,6		C adjust high limb for negative limb from s1
Lp1:	bdn	Lploop

	b	Lend

Lneg:	cax	9,9,0
	bdz	Lend
Lnloop:	lu	0,4(4)
	stu	8,4(3)
	cmpi	0,0,0
	mul	10,0,6
	mfmq	7
	ae	11,7,9
	l	7,4(3)
	ae	10,10,0		C propagate cy to new cy_limb
	sf	8,11,7		C add res_limb
	a	11,8,11		C invert cy (r11 is junk)
	bge	Ln0
	cax	10,10,6		C adjust high limb for negative limb from s1
Ln0:	bdz	Lend0
	lu	0,4(4)
	stu	8,4(3)
	cmpi	0,0,0
	mul	9,0,6
	mfmq	7
	ae	11,7,10
	l	7,4(3)
	ae	9,9,0		C propagate cy to new cy_limb
	sf	8,11,7		C add res_limb
	a	11,8,11		C invert cy (r11 is junk)
	bge	Ln1
	cax	9,9,6		C adjust high limb for negative limb from s1
Ln1:	bdn	Lnloop
	b	Lend

Lend0:	cal	9,0(10)
Lend:	st	8,4(3)
	aze	3,9
	br
EPILOGUE(mpn_submul_1)
