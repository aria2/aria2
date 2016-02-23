dnl  IBM POWER mpn_lshift -- Shift a number left.

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
dnl  s_ptr	r4
dnl  size	r5
dnl  cnt	r6

include(`../config.m4')

ASM_START()
PROLOGUE(mpn_lshift)
	sli	0,5,2
	cax	9,3,0
	cax	4,4,0
	sfi	8,6,32
	mtctr	5		C put limb count in CTR loop register
	lu	0,-4(4)		C read most significant limb
	sre	3,0,8		C compute carry out limb, and init MQ register
	bdz	Lend2		C if just one limb, skip loop
	lu	0,-4(4)		C read 2:nd most significant limb
	sreq	7,0,8		C compute most significant limb of result
	bdz	Lend		C if just two limb, skip loop
Loop:	lu	0,-4(4)		C load next lower limb
	stu	7,-4(9)		C store previous result during read latency
	sreq	7,0,8		C compute result limb
	bdn	Loop		C loop back until CTR is zero
Lend:	stu	7,-4(9)		C store 2:nd least significant limb
Lend2:	sle	7,0,6		C compute least significant limb
	st	7,-4(9)		C store it
	br
EPILOGUE(mpn_lshift)
