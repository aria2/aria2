dnl  ARM v6 mpn_sqr_basecase.

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

C Code structure:
C
C
C        m_2(0m4)        m_2(2m4)        m_2(1m4)        m_2(3m4)
C           |               |               |               |
C           |               |               |               |
C           |               |               |               |
C          \|/             \|/             \|/             \|/
C              ____________                   ____________
C             /            \                 /            \
C            \|/            \               \|/            \
C         am_2(3m4)       am_2(1m4)       am_2(0m4)       am_2(2m4)
C            \            /|\                \            /|\
C             \____________/                  \____________/
C                       \                        /
C                        \                      /
C                         \                    /
C                       tail(0m2)          tail(1m2)
C                            \              /
C                             \            /
C                            sqr_diag_addlsh1

C TODO
C  * Further tweak counter and updates in outer loops.  (This could save
C    perhaps 5n cycles).
C  * Try to use fewer register.  Perhaps coalesce r9 branch target and n_saved.
C    (This could save 2-3 cycles for n > 4.)
C  * Optimise sqr_diag_addlsh1 loop.  (This could save O(n) cycles.)
C  * Implement larger final corners (xit/tix).  Also stop loops earlier
C    suppressing writes of upper-most rp[] values.  (This could save 10-20
C    cycles for n > 4.)
C  * Is the branch really faster than discrete branches?

define(`rp',      r0)
define(`up',      r1)
define(`n',       r2)

define(`v0',      r3)
define(`v1',      r6)
define(`i',       r8)
define(`n_saved', r14)
define(`cya',     r11)
define(`cyb',     r12)
define(`u0',      r7)
define(`u1',      r9)

ASM_START()
PROLOGUE(mpn_sqr_basecase)
	and	r12, n, #3
	cmp	n, #4
	addgt	r12, r12, #4
	add	pc, pc, r12, lsl #2
	nop
	b	L(4)
	b	L(1)
	b	L(2)
	b	L(3)
	b	L(0m4)
	b	L(1m4)
	b	L(2m4)
	b	L(3m4)


L(1m4):	push	{r4-r10,r11,r14}
	mov	n_saved, n
	sub	i, n, #4
	sub	n, n, #2
	add	r10, pc, #L(am2_2m4)-.-8
	ldm	up, {v0,v1,u0}
	sub	up, up, #4
	mov	cyb, #0
	mov	r5, #0
	umull	r4, cya, v1, v0
	str	r4, [rp], #-12
	mov	r4, #0
	b	L(ko0)

L(3m4):	push	{r4-r10,r11,r14}
	mov	n_saved, n
	sub	i, n, #4
	sub	n, n, #2
	add	r10, pc, #L(am2_0m4)-.-8
	ldm	up, {v0,v1,u0}
	add	up, up, #4
	mov	cyb, #0
	mov	r5, #0
	umull	r4, cya, v1, v0
	str	r4, [rp], #-4
	mov	r4, #0
	b	L(ko2)

L(2m4):	push	{r4-r10,r11,r14}
	mov	n_saved, n
	sub	i, n, #4
	sub	n, n, #2
	add	r10, pc, #L(am2_3m4)-.-8
	ldm	up, {v0,v1,u1}
	mov	cyb, #0
	mov	r4, #0
	umull	r5, cya, v1, v0
	str	r5, [rp], #-8
	mov	r5, #0
	b	L(ko1)

L(0m4):	push	{r4-r10,r11,r14}
	mov	n_saved, n
	sub	i, n, #4
	sub	n, n, #2
	add	r10, pc, #L(am2_1m4)-.-8
	ldm	up, {v0,v1,u1}
	mov	cyb, #0
	mov	r4, #0
	add	up, up, #8
	umull	r5, cya, v1, v0
	str	r5, [rp, #0]
	mov	r5, #0

L(top):	ldr	u0, [up, #4]
	umaal	r4, cya, u1, v0
	str	r4, [rp, #4]
	mov	r4, #0
	umaal	r5, cyb, u1, v1
L(ko2):	ldr	u1, [up, #8]
	umaal	r5, cya, u0, v0
	str	r5, [rp, #8]
	mov	r5, #0
	umaal	r4, cyb, u0, v1
L(ko1):	ldr	u0, [up, #12]
	umaal	r4, cya, u1, v0
	str	r4, [rp, #12]
	mov	r4, #0
	umaal	r5, cyb, u1, v1
L(ko0):	ldr	u1, [up, #16]!
	umaal	r5, cya, u0, v0
	str	r5, [rp, #16]!
	mov	r5, #0
	umaal	r4, cyb, u0, v1
	subs	i, i, #4
	bhi	L(top)
	bx	r10

L(evnloop):
	subs	i, n, #4
	sub	n, n, #2
	blt	L(tix)
	ldm	up, {v0,v1,u0}
	add	up, up, #4
	mov	cya, #0
	mov	cyb, #0
	ldm	rp, {r4,r5}
	sub	rp, rp, #4
	umaal	r4, cya, v1, v0
	str	r4, [rp, #4]
	ldr	r4, [rp, #12]
	b	L(lo2)
L(ua2):	ldr	u0, [up, #4]
	umaal	r4, cya, u1, v0
	str	r4, [rp, #4]
	ldr	r4, [rp, #12]
	umaal	r5, cyb, u1, v1
L(lo2):	ldr	u1, [up, #8]
	umaal	r5, cya, u0, v0
	str	r5, [rp, #8]
	ldr	r5, [rp, #16]
	umaal	r4, cyb, u0, v1
	ldr	u0, [up, #12]
	umaal	r4, cya, u1, v0
	str	r4, [rp, #12]
	ldr	r4, [rp, #20]
	umaal	r5, cyb, u1, v1
	ldr	u1, [up, #16]!
	umaal	r5, cya, u0, v0
	str	r5, [rp, #16]!
	ldr	r5, [rp, #8]
	umaal	r4, cyb, u0, v1
	subs	i, i, #4
	bhi	L(ua2)
L(am2_0m4):
	umaal	r4, cya, u1, v0
	ldr	u0, [up, #4]
	umaal	r5, cyb, u1, v1
	str	r4, [rp, #4]
	umaal	r5, cya, u0, v0
	umaal	cya, cyb, u0, v1
	str	r5, [rp, #8]
	str	cya, [rp, #12]
	str	cyb, [rp, #16]
	sub	up, up, n, lsl #2
	sub	rp, rp, n, lsl #2
	add	up, up, #8
	sub	i, n, #4
	sub	n, n, #2
	ldm	up, {v0,v1,u0}
	sub	up, up, #4
	mov	cya, #0
	mov	cyb, #0
	ldr	r4, [rp, #24]
	ldr	r5, [rp, #28]
	add	rp, rp, #12
	umaal	r4, cya, v1, v0
	str	r4, [rp, #12]
	ldr	r4, [rp, #20]
	b	L(lo0)
L(ua0):	ldr	u0, [up, #4]
	umaal	r4, cya, u1, v0
	str	r4, [rp, #4]
	ldr	r4, [rp, #12]
	umaal	r5, cyb, u1, v1
	ldr	u1, [up, #8]
	umaal	r5, cya, u0, v0
	str	r5, [rp, #8]
	ldr	r5, [rp, #16]
	umaal	r4, cyb, u0, v1
	ldr	u0, [up, #12]
	umaal	r4, cya, u1, v0
	str	r4, [rp, #12]
	ldr	r4, [rp, #20]
	umaal	r5, cyb, u1, v1
L(lo0):	ldr	u1, [up, #16]!
	umaal	r5, cya, u0, v0
	str	r5, [rp, #16]!
	ldr	r5, [rp, #8]
	umaal	r4, cyb, u0, v1
	subs	i, i, #4
	bhi	L(ua0)
L(am2_2m4):
	umaal	r4, cya, u1, v0
	ldr	u0, [up, #4]
	umaal	r5, cyb, u1, v1
	str	r4, [rp, #4]
	umaal	r5, cya, u0, v0
	umaal	cya, cyb, u0, v1
	str	r5, [rp, #8]
	str	cya, [rp, #12]
	str	cyb, [rp, #16]
	sub	up, up, n, lsl #2
	sub	rp, rp, n, lsl #2
	add	up, up, #8
	add	rp, rp, #24
	b	L(evnloop)


L(oddloop):
	subs	i, n, #4
	sub	n, n, #2
	blt	L(xit)
	ldm	up, {v0,v1,u1}
	mov	cya, #0
	mov	cyb, #0
	sub	rp, rp, #8
	ldr	r5, [rp, #8]
	ldr	r4, [rp, #12]
	umaal	r5, cya, v1, v0
	str	r5, [rp, #8]
	ldr	r5, [rp, #16]
	b	L(lo1)
L(ua1):	ldr	u0, [up, #4]
	umaal	r4, cya, u1, v0
	str	r4, [rp, #4]
	ldr	r4, [rp, #12]
	umaal	r5, cyb, u1, v1
	ldr	u1, [up, #8]
	umaal	r5, cya, u0, v0
	str	r5, [rp, #8]
	ldr	r5, [rp, #16]
	umaal	r4, cyb, u0, v1
L(lo1):	ldr	u0, [up, #12]
	umaal	r4, cya, u1, v0
	str	r4, [rp, #12]
	ldr	r4, [rp, #20]
	umaal	r5, cyb, u1, v1
	ldr	u1, [up, #16]!
	umaal	r5, cya, u0, v0
	str	r5, [rp, #16]!
	ldr	r5, [rp, #8]
	umaal	r4, cyb, u0, v1
	subs	i, i, #4
	bhi	L(ua1)
L(am2_3m4):
	umaal	r4, cya, u1, v0
	ldr	u0, [up, #4]
	umaal	r5, cyb, u1, v1
	str	r4, [rp, #4]
	umaal	r5, cya, u0, v0
	umaal	cya, cyb, u0, v1
	str	r5, [rp, #8]
	str	cya, [rp, #12]
	str	cyb, [rp, #16]
	sub	up, up, n, lsl #2
	sub	rp, rp, n, lsl #2
	add	up, up, #8
	add	rp, rp, #24
	subs	i, n, #4
	sub	n, n, #2
	ldm	up, {v0,v1,u1}
	mov	cya, #0
	mov	cyb, #0
	ldr	r5, [rp, #0]
	ldr	r4, [rp, #4]
	add	up, up, #8
	umaal	r5, cya, v1, v0
	str	r5, [rp, #0]
	ldr	r5, [rp, #8]
	bls	L(e3)
L(ua3):	ldr	u0, [up, #4]
	umaal	r4, cya, u1, v0
	str	r4, [rp, #4]
	ldr	r4, [rp, #12]
	umaal	r5, cyb, u1, v1
	ldr	u1, [up, #8]
	umaal	r5, cya, u0, v0
	str	r5, [rp, #8]
	ldr	r5, [rp, #16]
	umaal	r4, cyb, u0, v1
	ldr	u0, [up, #12]
	umaal	r4, cya, u1, v0
	str	r4, [rp, #12]
	ldr	r4, [rp, #20]
	umaal	r5, cyb, u1, v1
	ldr	u1, [up, #16]!
	umaal	r5, cya, u0, v0
	str	r5, [rp, #16]!
	ldr	r5, [rp, #8]
	umaal	r4, cyb, u0, v1
	subs	i, i, #4
	bhi	L(ua3)
L(e3):
L(am2_1m4):
	umaal	r4, cya, u1, v0
	ldr	u0, [up, #4]
	umaal	r5, cyb, u1, v1
	str	r4, [rp, #4]
	umaal	r5, cya, u0, v0
	umaal	cya, cyb, u0, v1
	str	r5, [rp, #8]
	str	cya, [rp, #12]
	str	cyb, [rp, #16]
	sub	up, up, n, lsl #2
	sub	rp, rp, n, lsl #2
	add	up, up, #8
	add	rp, rp, #24
	b	L(oddloop)

L(xit):	ldm	up!, {v0,u0}
	ldr	cya, [rp], #12
	mov	cyb, #0
	umaal	cya, cyb, u0, v0
	b	L(sqr_diag_addlsh1)

L(tix):	ldm	up!, {v0,v1,u0}
	ldm	rp, {r4,r5}
	mov	cya, #0
	mov	cyb, #0
	umaal	r4, cya, v1, v0
	umaal	r5, cya, u0, v0
	stm	rp, {r4,r5}
	umaal	cya, cyb, u0, v1
	add	rp, rp, #20
C	b	L(sqr_diag_addlsh1)


define(`w0',  r6)
define(`w1',  r7)
define(`w2',  r8)
define(`rbx', r9)

L(sqr_diag_addlsh1):
	str	cya, [rp, #-12]
	str	cyb, [rp, #-8]
	sub	n, n_saved, #1
	sub	up, up, n_saved, lsl #2
	sub	rp, rp, n_saved, lsl #3
	ldr	r3, [up], #4
	umull	w1, r5, r3, r3
	mov	w2, #0
C	cmn	r0, #0			C clear cy (already clear by luck)
	b	L(lm)

L(tsd):	adds	w0, w0, rbx
	adcs	w1, w1, r4
	str	w0, [rp, #0]
L(lm):	ldr	w0, [rp, #4]
	str	w1, [rp, #4]
	ldr	w1, [rp, #8]!
	add	rbx, r5, w2
	adcs	w0, w0, w0
	ldr	r3, [up], #4
	adcs	w1, w1, w1
	mov	w2, #0
	adc	w2, w2, w2
	umull	r4, r5, r3, r3
	subs	n, n, #1
	bne	L(tsd)

	adds	w0, w0, rbx
	adcs	w1, w1, r4
	adc	w2, r5, w2
	stm	rp, {w0,w1,w2}

	pop	{r4-r10,r11,pc}


C Straight line code for n <= 4

L(1):	ldr	r3, [up, #0]
	umull	r1, r2, r3, r3
	stm	rp, {r1,r2}
	bx	r14

L(2):	push	{r4-r5}
	ldm	up, {r5,r12}
	umull	r1, r2, r5, r5
	umull	r3, r4, r12, r12
	umull	r5, r12, r5, r12
	adds	r5, r5, r5
	adcs	r12, r12, r12
	adc	r4, r4, #0
	adds	r2, r2, r5
	adcs	r3, r3, r12
	adc	r4, r4, #0
	stm	rp, {r1,r2,r3,r4}
	pop	{r4-r5}
	bx	r14

L(3):	push	{r4-r11}
	ldm	up, {r7,r8,r9}
	umull	r1, r2, r7, r7
	umull	r3, r4, r8, r8
	umull	r5, r6, r9, r9
	umull	r10, r11, r7, r8
	mov	r12, #0
	umlal	r11, r12, r7, r9
	mov	r7, #0
	umlal	r12, r7, r8, r9
	adds	r10, r10, r10
	adcs	r11, r11, r11
	adcs	r12, r12, r12
	adcs	r7, r7, r7
	adc	r6, r6, #0
	adds	r2, r2, r10
	adcs	r3, r3, r11
	adcs	r4, r4, r12
	adcs	r5, r5, r7
	adc	r6, r6, #0
	stm	rp, {r1,r2,r3,r4,r5,r6}
	pop	{r4-r11}
	bx	r14

L(4):	push	{r4-r11, r14}
	ldm	up, {r9,r10,r11,r12}
	umull	r1, r2, r9, r9
	umull	r3, r4, r10, r10
	umull	r5, r6, r11, r11
	umull	r7, r8, r12, r12
	stm	rp, {r1,r2,r3,r4,r5,r6,r7}
	umull	r1, r2, r9, r10
	mov	r3, #0
	umlal	r2, r3, r9, r11
	mov	r4, #0
	umlal	r3, r4, r9, r12
	mov	r5, #0
	umlal	r3, r5, r10, r11
	umaal	r4, r5, r10, r12
	mov	r6, #0
	umlal	r5, r6, r11, r12
	adds	r1, r1, r1
	adcs	r2, r2, r2
	adcs	r3, r3, r3
	adcs	r4, r4, r4
	adcs	r5, r5, r5
	adcs	r6, r6, r6
	adc	r7, r8, #0
	add	rp, rp, #4
	ldm	rp, {r8,r9,r10,r11,r12,r14}
	adds	r1, r1, r8
	adcs	r2, r2, r9
	adcs	r3, r3, r10
	adcs	r4, r4, r11
	adcs	r5, r5, r12
	adcs	r6, r6, r14
	adc	r7, r7, #0
	stm	rp, {r1,r2,r3,r4,r5,r6,r7}
	pop	{r4-r11, pc}
EPILOGUE()
