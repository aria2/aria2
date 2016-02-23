dnl  ARM mpn_modexact_1c_odd

dnl  Contributed to the GNU project by Torbjorn Granlund.

dnl  Copyright 2012 Free Software Foundation, Inc.

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

C	     cycles/limb
C StrongARM	 ?
C XScale	 ?
C Cortex-A8	 ?
C Cortex-A9	10
C Cortex-A15	 ?

define(`up', `r0')
define(`n',  `r1')
define(`d',  `r2')
define(`cy', `r3')

	.protected	binvert_limb_table
ASM_START()
PROLOGUE(mpn_modexact_1c_odd)
	stmfd	sp!, {r4, r5}

	LEA(	r4, binvert_limb_table)

	ldr	r5, [up], #4		C up[0]

	and	r12, d, #254
	ldrb	r4, [r4, r12, lsr #1]
	mul	r12, r4, r4
	mul	r12, d, r12
	rsb	r12, r12, r4, asl #1
	mul	r4, r12, r12
	mul	r4, d, r4
	rsb	r4, r4, r12, asl #1	C r4 = inverse

	subs	n, n, #1		C set carry as side-effect
	beq	L(end)

L(top):	sbcs	cy, r5, cy
	ldr	r5, [up], #4
	sub	n, n, #1
	mul	r12, r4, cy
	tst	n, n
	umull	r12, cy, d, r12
	bne	L(top)

L(end):	sbcs	cy, r5, cy
	mul	r12, r4, cy
	umull	r12, r0, d, r12
	addcc	r0, r0, #1

	ldmfd	sp!, {r4, r5}
	bx	r14
EPILOGUE()
