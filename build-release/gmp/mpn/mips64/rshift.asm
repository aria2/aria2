dnl  MIPS64 mpn_rshift -- Right shift.

dnl  Copyright 1995, 2000, 2001, 2002 Free Software Foundation, Inc.

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
C src_ptr	$5
C size		$6
C cnt		$7

ASM_START()
PROLOGUE(mpn_rshift)
	ld	$10,0($5)	C load first limb
	dsubu	$13,$0,$7
	daddiu	$6,$6,-1
	and	$9,$6,4-1	C number of limbs in first loop
	beq	$9,$0,.L0	C if multiple of 4 limbs, skip first loop
	 dsll	$2,$10,$13	C compute function result

	dsubu	$6,$6,$9

.Loop0:	ld	$3,8($5)
	daddiu	$4,$4,8
	daddiu	$5,$5,8
	daddiu	$9,$9,-1
	dsrl	$11,$10,$7
	dsll	$12,$3,$13
	move	$10,$3
	or	$8,$11,$12
	bne	$9,$0,.Loop0
	 sd	$8,-8($4)

.L0:	beq	$6,$0,.Lend
	 nop

.Loop:	ld	$3,8($5)
	daddiu	$4,$4,32
	daddiu	$6,$6,-4
	dsrl	$11,$10,$7
	dsll	$12,$3,$13

	ld	$10,16($5)
	dsrl	$14,$3,$7
	or	$8,$11,$12
	sd	$8,-32($4)
	dsll	$9,$10,$13

	ld	$3,24($5)
	dsrl	$11,$10,$7
	or	$8,$14,$9
	sd	$8,-24($4)
	dsll	$12,$3,$13

	ld	$10,32($5)
	dsrl	$14,$3,$7
	or	$8,$11,$12
	sd	$8,-16($4)
	dsll	$9,$10,$13

	daddiu	$5,$5,32
	or	$8,$14,$9
	bgtz	$6,.Loop
	 sd	$8,-8($4)

.Lend:	dsrl	$8,$10,$7
	j	$31
	sd	$8,0($4)
EPILOGUE(mpn_rshift)
