dnl  ARM mpn_addmul_2.

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
C StrongARM:	 -
C XScale	 -
C Cortex-A8	 ?
C Cortex-A9	 2.38
C Cortex-A15	 2.5

C TODO
C  * Consider using more registers for the r[] loads, allowing better load-use
C    scheduling for a 6% speedup (on A9).  Free: r10, r11, r14

define(`rp',`r0')
define(`up',`r1')
define(`n', `r2')
define(`vp',`r3')

define(`v0',`r6')
define(`v1',`r7')
define(`u0',`r3')
define(`u1',`r9')

define(`cya',`r8')
define(`cyb',`r12')


ASM_START()
PROLOGUE(mpn_addmul_2)
	push	{ r4, r5, r6, r7, r8, r9 }

	ldm	vp, { v0, v1 }
	mov	cya, #0
	mov	cyb, #0

	tst	n, #1
	beq	L(evn)
L(odd):	ldr	r5, [rp, #0]
	ldr	u0, [up, #0]
	ldr	r4, [rp, #4]
	tst	n, #2
	beq	L(fi1)
L(fi3):	sub	up, up, #12
	sub	rp, rp, #16
	b	L(lo3)
L(fi1):	sub	n, n, #1
	sub	up, up, #4
	sub	rp, rp, #8
	b	L(lo1)
L(evn):	ldr	r4, [rp, #0]
	ldr	u1, [up, #0]
	ldr	r5, [rp, #4]
	tst	n, #2
	bne	L(fi2)
L(fi0):	sub	up, up, #8
	sub	rp, rp, #12
	b	L(lo0)
L(fi2):	subs	n, n, #2
	sub	rp, rp, #4
	bls	L(end)

	ALIGN(16)
L(top):	ldr	u0, [up, #4]
	umaal	r4, cya, u1, v0
	str	r4, [rp, #4]
	ldr	r4, [rp, #12]
	umaal	r5, cyb, u1, v1
L(lo1):	ldr	u1, [up, #8]
	umaal	r5, cya, u0, v0
	str	r5, [rp, #8]
	ldr	r5, [rp, #16]
	umaal	r4, cyb, u0, v1
L(lo0):	ldr	u0, [up, #12]
	umaal	r4, cya, u1, v0
	str	r4, [rp, #12]
	ldr	r4, [rp, #20]
	umaal	r5, cyb, u1, v1
L(lo3):	ldr	u1, [up, #16]!
	umaal	r5, cya, u0, v0
	str	r5, [rp, #16]!
	ldr	r5, [rp, #8]
	umaal	r4, cyb, u0, v1
	subs	n, n, #4
	bhi	L(top)

L(end):	umaal	r4, cya, u1, v0
	ldr	u0, [up, #4]
	umaal	r5, cyb, u1, v1
	str	r4, [rp, #4]
	umaal	r5, cya, u0, v0
	umaal	cya, cyb, u0, v1
	str	r5, [rp, #8]
	str	cya, [rp, #12]
	mov	r0, cyb

	pop	{ r4, r5, r6, r7, r8, r9 }
	bx	r14
EPILOGUE()
