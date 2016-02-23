dnl  ARM mpn_mod_34lsub1 -- remainder modulo 2^24-1.

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
C Cortex-A9	 1.33
C Cortex-A15	 ?

define(`ap',	r0)
define(`n',	r1)

C mp_limb_t mpn_mod_34lsub1 (mp_srcptr up, mp_size_t n)

C TODO
C  * Write cleverer summation code.
C  * Consider loading 6 64-bit aligned registers at a time, to approach 1 c/l.

ASM_START()
	TEXT
	ALIGN(32)
PROLOGUE(mpn_mod_34lsub1)
	push	{ r4, r5, r6, r7 }

	subs	n, n, #3
	mov	r7, #0
	blt	L(le2)			C n <= 2

	ldmia	ap!, { r2, r3, r12 }
	subs	n, n, #3
	blt	L(sum)			C n <= 5
	adds	r0, r0, #0		C clear carry
	sub	n, n, #3
	b	L(mid)

L(top):	adcs	r2, r2, r4
	adcs	r3, r3, r5
	adcs	r12, r12, r6
L(mid):	ldmia	ap!, { r4, r5, r6 }
	tst	n, n
	sub	n, n, #3
	bpl	L(top)

	add	n, n, #3

	adcs	r2, r2, r4
	adcs	r3, r3, r5
	adcs	r12, r12, r6
	movcs	r7, #1			C r7 <= 1

L(sum):	cmn	n, #2
	movlo	r4, #0
	ldrhs	r4, [ap], #4
	movls	r5, #0
	ldrhi	r5, [ap], #4

	adds	r2, r2, r4
	adcs	r3, r3, r5
	adcs	r12, r12, #0
	adc	r7, r7, #0		C r7 <= 2

L(sum2):
	bic	r0, r2, #0xff000000
	add	r0, r0, r2, lsr #24
	add	r0, r0, r7

	lsl	r7, r3, #8
	bic	r1, r7, #0xff000000
	add	r0, r0, r1
	add	r0, r0, r3, lsr #16

	lsl	r7, r12, #16
	bic	r1, r7, #0xff000000
	add	r0, r0, r1
	add	r0, r0, r12, lsr #8

	pop	{ r4, r5, r6, r7 }
	bx	lr

L(le2):	cmn	n, #1
	bne	L(1)
	ldmia	ap!, { r2, r3 }
	mov	r12, #0
	b	L(sum2)
L(1):	ldr	r2, [ap]
	bic	r0, r2, #0xff000000
	add	r0, r0, r2, lsr #24
	pop	{ r4, r5, r6, r7 }
	bx	lr
EPILOGUE()
