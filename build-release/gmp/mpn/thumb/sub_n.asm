dnl  ARM/Thumb mpn_sub_n.

dnl  Copyright 1997, 2000, 2012 Free Software Foundation, Inc.

dnl  This file is part of the GNU MP Library.

dnl  The GNU MP Library is free software; you can redistribute it and/or modify
dnl  it under the terms of the GNU Lesser General Public License as published by
dnl  the Free Software Foundation; either version 3 of the License, or (at your
dnl  option) any later version.

dnl  The GNU MP Library is distributed in the hope that it will be useful, but
dnl  WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
dnl  or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public
dnl  License for more details.

dnl  You should have received a copy of the GNU Lesser General Public License
dnl  along with the GNU MP Library.  If not, see http://www.gnu.org/licenses/.

include(`../config.m4')

C INPUT PARAMETERS
define(`rp',	r0)
define(`up',	r1)
define(`vp',	r2)
define(`n',	r3)

ASM_START()
	.thumb
PROLOGUE(mpn_sub_nc)
	push	{r4, r5, r6}
	ldr	r6, [sp, #12]		C init carry save register
	neg	r6, r6
	b	L(top)
EPILOGUE()
PROLOGUE(mpn_sub_n)
	push	{r4, r5, r6}
	mov	r6, n			C init carry save register

L(top):	ldmia	up!, {r4}		C load next limb from S1
	cmp	n, r6			C tricky carry restore
	ldmia	vp!, {r5}		C load next limb from S2
	sbc	r4, r5
	stmia	rp!, {r4}		C store result limb to RES
	sbc	r6, r6			C save negated carry
	sub	n, #1
	bne	L(top)

	neg	r0, r6
	pop	{r4, r5, r6}
	bx	lr
EPILOGUE()
