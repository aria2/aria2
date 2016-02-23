dnl  ARM mpn_addmul_1.

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
C StrongARM:	 -
C XScale	 -
C Cortex-A8	 ?
C Cortex-A9	 3.25
C Cortex-A15	 4

C TODO
C  * Micro-optimise feed-in code.
C  * Optimise for n=1,2 by delaying register saving.
C  * Try using ldm/stm.

define(`rp',`r0')
define(`up',`r1')
define(`n', `r2')
define(`v0',`r3')

ASM_START()
PROLOGUE(mpn_addmul_1)
	stmfd	sp!, { r4, r5, r6, r7 }

	ands	r6, n, #3
	mov	r12, #0
	beq	L(fi0)
	cmp	r6, #2
	bcc	L(fi1)
	beq	L(fi2)

L(fi3):	ldr	r4, [up], #4
	ldr	r6, [rp, #0]
	ldr	r5, [up], #4
	b	L(lo3)

L(fi0):	ldr	r5, [up], #4
	ldr	r7, [rp], #4
	ldr	r4, [up], #4
	b	L(lo0)

L(fi1):	ldr	r4, [up], #4
	ldr	r6, [rp], #8
	subs	n, n, #1
	beq	L(1)
	ldr	r5, [up], #4
	b	L(lo1)

L(fi2):	ldr	r5, [up], #4
	ldr	r7, [rp], #12
	ldr	r4, [up], #4
	b	L(lo2)

	ALIGN(16)
L(top):	ldr	r6, [rp, #-8]
	ldr	r5, [up], #4
	str	r7, [rp, #-12]
L(lo1):	umaal	r6, r12, r4, v0
	ldr	r7, [rp, #-4]
	ldr	r4, [up], #4
	str	r6, [rp, #-8]
L(lo0):	umaal	r7, r12, r5, v0
	ldr	r6, [rp, #0]
	ldr	r5, [up], #4
	str	r7, [rp, #-4]
L(lo3):	umaal	r6, r12, r4, v0
	ldr	r7, [rp, #4]
	ldr	r4, [up], #4
	str	r6, [rp], #16
L(lo2):	umaal	r7, r12, r5, v0
	subs	n, n, #4
	bhi	L(top)

	ldr	r6, [rp, #-8]
	str	r7, [rp, #-12]
L(1):	umaal	r6, r12, r4, v0
	str	r6, [rp, #-8]
	mov	r0, r12
	ldmfd	sp!, { r4, r5, r6, r7 }
	bx	lr
EPILOGUE()
