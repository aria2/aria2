dnl  Alpha ev67 mpn_popcount -- mpn bit population count.

dnl  Copyright 2003, 2005 Free Software Foundation, Inc.

dnl  This file is part of the GNU MP Library.
dnl
dnl  The GNU MP Library is free software; you can redistribute it and/or
dnl  modify it under the terms of the GNU Lesser General Public License as
dnl  published by the Free Software Foundation; either version 3 of the
dnl  License, or (at your option) any later version.
dnl
dnl  The GNU MP Library is distributed in the hope that it will be useful,
dnl  but WITHOUT ANY WARRANTY; without even the implied warranty of
dnl  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
dnl  Lesser General Public License for more details.
dnl
dnl  You should have received a copy of the GNU Lesser General Public License
dnl  along with the GNU MP Library.  If not, see http://www.gnu.org/licenses/.

include(`../config.m4')


C ev67: 1.5 cycles/limb


C unsigned long mpn_popcount (mp_srcptr src, mp_size_t size);
C
C This schedule seems necessary for the full 1.5 c/l, the IQ can't quite hide
C all latencies, the addq's must be deferred to the next iteration.
C
C Since we need just 3 instructions per limb, further unrolling could approach
C 1.0 c/l.
C
C The main loop processes two limbs at a time.  An odd size is handled by
C processing src[0] at the start.  If the size is even that result is
C discarded, and src[0] is repeated by the main loop.
C

ASM_START()
PROLOGUE(mpn_popcount)

	C r16	src
	C r17	size

	ldq	r0, 0(r16)		C L0  src[0]
	and	r17, 1, r8		C U1  1 if size odd
	srl	r17, 1, r17		C U0  size, limb pairs

	s8addq	r8, r16, r16		C L1  src++ if size odd
	ctpop	r0, r0			C U0
	beq	r17, L(one)		C U1  if size==1

	cmoveq	r8, r31, r0		C L   discard first limb if size even
	clr	r3			C L

	clr	r4			C L
	unop				C U
	unop				C L
	unop				C U


	ALIGN(16)
L(top):
	C r0	total accumulating
	C r3	pop 0
	C r4	pop 1
	C r16	src, incrementing
	C r17	size, decrementing

	ldq	r1, 0(r16)		C L
	ldq	r2, 8(r16)		C L
	lda	r16, 16(r16)		C U
	lda	r17, -1(r17)		C U

	addq	r0, r3, r0		C L
	addq	r0, r4, r0		C L
	ctpop	r1, r3			C U0
	ctpop	r2, r4			C U0

	ldl	r31, 512(r16)		C L	prefetch
	bne	r17, L(top)		C U


	addq	r0, r3, r0		C L
	addq	r0, r4, r0		C U
L(one):
	ret	r31, (r26), 1		C L0

EPILOGUE()
ASM_END()
