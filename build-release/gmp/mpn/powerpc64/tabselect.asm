dnl  PowerPC-64 mpn_tabselect.

dnl  Copyright 2011 Free Software Foundation, Inc.

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

C                  cycles/limb
C POWER3/PPC630          ?
C POWER4/PPC970          3.3
C POWER5                 ?
C POWER6                 ?
C POWER7                 2.5

C NOTES
C  * This has not been tuned for any specific processor.  Its speed should not
C    be too bad, though.
C  * Using VMX could result in significant speedup for certain CPUs.

C mpn_tabselect (mp_limb_t *rp, mp_limb_t *tp, mp_size_t n, mp_size_t nents, mp_size_t which)
define(`rp',     `r3')
define(`tp',     `r4')
define(`n',      `r5')
define(`nents',  `r6')
define(`which',  `r7')

define(`mask',   `r8')

ASM_START()
	TEXT
	ALIGN(16)
PROLOGUE(mpn_tabselect)
	addi	r0, n, 1
	srdi	r0, r0, 1		C inner loop count
	andi.	r9, n, 1		C set cr0 for use in inner loop
	subf	which, nents, which
	sldi	n, n, 3

L(outer):
	mtctr	r0			C put inner loop count in ctr

	add	r9, which, nents	C are we at the selected table entry?
	addic	r9, r9, -1		C set CF iff not selected entry
	subfe	mask, r0, r0

	beq	cr0, L(top)		C branch to loop entry if n even

	ld	r9, 0(tp)
	addi	tp, tp, 8
	and	r9, r9, mask
	ld	r11, 0(rp)
	andc	r11, r11, mask
	or	r9, r9, r11
	std	r9, 0(rp)
	addi	rp, rp, 8
	bdz	L(end)

	ALIGN(16)
L(top):	ld	r9, 0(tp)
	ld	r10, 8(tp)
	addi	tp, tp, 16
	nop
	and	r9, r9, mask
	and	r10, r10, mask
	ld	r11, 0(rp)
	ld	r12, 8(rp)
	andc	r11, r11, mask
	andc	r12, r12, mask
	or	r9, r9, r11
	or	r10, r10, r12
	std	r9, 0(rp)
	std	r10, 8(rp)
	addi	rp, rp, 16
	bdnz	L(top)

L(end):	subf	rp, n, rp		C move rp back to beginning
	cmpdi	cr6, nents, 1
	addi	nents, nents, -1
	bne	cr6, L(outer)

	blr
EPILOGUE()
