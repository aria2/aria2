dnl  MIPS32 mpn_mul_1 -- Multiply a limb vector with a single limb and store
dnl  the product in a second limb vector.

dnl  Copyright 1992, 1994, 1996, 2000, 2002 Free Software Foundation, Inc.

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

C INPUT PARAMETERS
C res_ptr	$4
C s1_ptr	$5
C size		$6
C s2_limb	$7

ASM_START()
PROLOGUE(mpn_mul_1)

C feed-in phase 0
	lw	$8,0($5)

C feed-in phase 1
	addiu	$5,$5,4
	multu	$8,$7

	addiu	$6,$6,-1
	beq	$6,$0,$LC0
	 move	$2,$0		C zero cy2

	addiu	$6,$6,-1
	beq	$6,$0,$LC1
	lw	$8,0($5)	C load new s1 limb as early as possible

Loop:	mflo	$10
	mfhi	$9
	addiu	$5,$5,4
	addu	$10,$10,$2	C add old carry limb to low product limb
	multu	$8,$7
	lw	$8,0($5)	C load new s1 limb as early as possible
	addiu	$6,$6,-1	C decrement loop counter
	sltu	$2,$10,$2	C carry from previous addition -> $2
	sw	$10,0($4)
	addiu	$4,$4,4
	bne	$6,$0,Loop
	 addu	$2,$9,$2	C add high product limb and carry from addition

C wind-down phase 1
$LC1:	mflo	$10
	mfhi	$9
	addu	$10,$10,$2
	sltu	$2,$10,$2
	multu	$8,$7
	sw	$10,0($4)
	addiu	$4,$4,4
	addu	$2,$9,$2	C add high product limb and carry from addition

C wind-down phase 0
$LC0:	mflo	$10
	mfhi	$9
	addu	$10,$10,$2
	sltu	$2,$10,$2
	sw	$10,0($4)
	j	$31
	addu	$2,$9,$2	C add high product limb and carry from addition
EPILOGUE(mpn_mul_1)
