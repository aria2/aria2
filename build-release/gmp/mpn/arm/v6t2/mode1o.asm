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
C Cortex-A9	 9
C Cortex-A15	 ?

define(`up', `r0')
define(`n',  `r1')
define(`d',  `r2')
define(`cy', `r3')

	.protected	binvert_limb_table
ASM_START()
PROLOGUE(mpn_modexact_1c_odd)
	stmfd	sp!, {r4, r5, r6, r7}

	LEA(	r4, binvert_limb_table)

	ldr	r6, [up], #4		C up[0]

	ubfx	r12, d, #1, #7
	ldrb	r4, [r4, r12]
	smulbb	r12, r4, r4
	mul	r12, d, r12
	rsb	r12, r12, r4, asl #1
	mul	r4, r12, r12
	mul	r4, d, r4
	rsb	r4, r4, r12, asl #1	C r4 = inverse

	subs	n, n, #1
	sub	r6, r6, cy
	mul	r6, r6, r4
	beq	L(end)

	rsb	r5, r4, #0		C r5 = -inverse

L(top):	ldr	r7, [up], #4
	mov	r12, #0
	umaal	r12, cy, r6, d
	mul	r6, r7, r4
	mla	r6, cy, r5, r6
	subs	n, n, #1
	bne	L(top)

L(end):	mov	r12, #0
	umaal	r12, cy, r6, d
	mov	r0, cy

	ldmfd	sp!, {r4, r5, r6, r7}
	bx	r14
EPILOGUE()
