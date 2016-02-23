dnl  ARM v5 mpn_gcd_1.

dnl  Based on the K7 gcd_1.asm, by Kevin Ryde.  Rehacked for ARM by Torbjorn
dnl  Granlund.

dnl  Copyright 2000, 2001, 2002, 2005, 2009, 2011, 2012 Free Software
dnl  Foundation, Inc.

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

C	     cycles/bit (approx)
C StrongARM	 ?
C XScale	 ?
C Cortex-A8	 ?
C Cortex-A9	 5.9
C Cortex-A15	 ?
C Numbers measured with: speed -CD -s8-32 -t24 mpn_gcd_1

C TODO
C  * Optimise inner-loop better.

C Threshold of when to call bmod when U is one limb.  Should be about
C (time_in_cycles(bmod_1,1) + call_overhead) / (cycles/bit).
define(`BMOD_THRES_LOG2', 6)

C INPUT PARAMETERS
define(`up',    `r0')
define(`n',     `r1')
define(`v0',    `r2')

ifdef(`BMOD_1_TO_MOD_1_THRESHOLD',,
  `define(`BMOD_1_TO_MOD_1_THRESHOLD',0xffffffff)')

ASM_START()
	TEXT
	ALIGN(16)
PROLOGUE(mpn_gcd_1)
	push	{r4, r7, lr}
	ldr	r3, [up]	C U low limb

	orr	r3, r3, v0
	rsb	r4, r3, #0
	and	r4, r4, r3
	clz	r4, r4		C min(ctz(u0),ctz(v0))
	rsb	r4, r4, #31

	rsb	r12, v0, #0
	and	r12, r12, v0
	clz	r12, r12
	rsb	r12, r12, #31
	lsr	v0, v0, r12

	mov	r7, v0

	cmp	n, #1
	bne	L(nby1)

C Both U and V are single limbs, reduce with bmod if u0 >> v0.
	ldr	r3, [up]
	cmp	v0, r3, lsr #BMOD_THRES_LOG2
	bhi	L(red1)

L(bmod):mov	r3, #0		C carry argument
	bl	mpn_modexact_1c_odd
	b	L(red0)

L(nby1):cmp	n, #BMOD_1_TO_MOD_1_THRESHOLD
	blo	L(bmod)

	bl	mpn_mod_1

L(red0):mov	r3, r0
L(red1):rsbs	r12, r3, #0
	and	r12, r12, r3
	clz	r12, r12
	rsb	r12, r12, #31
	bne	L(mid)
	b	L(end)

	ALIGN(8)
L(top):	rsb	r12, r12, #31
	movcc	r3, r1		C if x-y < 0
	movcc	r7, r0		C use x,y-x
L(mid):	lsr	r3, r3, r12	C
	mov	r0, r3		C
	sub	r1, r7, r3	C
	rsbs	r3, r7, r3	C
	and	r12, r1, r3	C
	clz	r12, r12	C
	bne	L(top)		C

L(end):	lsl	r0, r7, r4
	pop	{r4, r7, pc}
EPILOGUE()
