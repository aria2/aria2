dnl  SPARC v9 32-bit mpn_submul_1 -- Multiply a limb vector with a limb and
dnl  subtract the result from a second limb vector.

dnl  Copyright 1998, 2000, 2001, 2003 Free Software Foundation, Inc.

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

C Algorithm: We use two floating-point multiplies per limb product, with the
C invariant v operand split into two 16-bit pieces, and the u operand split
C into 32-bit pieces.  We convert the two 48-bit products and transfer them to
C the integer unit.

C		   cycles/limb
C UltraSPARC 1&2:     6.5
C UltraSPARC 3:	      ?

C Possible optimizations:
C   1. Combine 32-bit memory operations into 64-bit operations.  Since we're
C      memory bandwidth limited, this could save 1.5 cycles/limb.
C   2. Unroll the inner loop.  Since we already use alternate temporary areas,
C      it is very straightforward to unroll, using an exit branch midways.
C      Unrolling would allow deeper scheduling which could improve speed for L2
C      cache case.
C   3. For mpn_mul_1: Use more alternating temp areas.  The std'es and ldx'es
C      aren't sufficiently apart-scheduled with just two temp areas.
C   4. Specialize for particular v values.  If its upper 16 bits are zero, we
C      could save many operations.

C INPUT PARAMETERS
C rp	i0
C up	i1
C n	i2
C v	i3

define(`FSIZE',224)

ASM_START()
PROLOGUE(mpn_submul_1)
	add	%sp, -FSIZE, %sp
	sethi	%hi(0xffff), %g1
	srl	%o3, 16, %g2
	or	%g1, %lo(0xffff), %g1
	and	%o3, %g1, %g1
	stx	%g1, [%sp+104]
	stx	%g2, [%sp+112]
	ldd	[%sp+104], %f6
	ldd	[%sp+112], %f8
	fxtod	%f6, %f6
	fxtod	%f8, %f8
	ld	[%sp+104], %f10		C zero f10

	mov	0, %g3			C cy = 0

define(`fanop', `fitod %f18, %f0')	C  A quasi nop running in the FA pipe

	add	%sp, 160, %o5		C point in scratch area
	and	%o5, -32, %o5		C align at 0 (mod 32) in scratch area

	subcc	%o2, 1, %o2
	ld	[%o1], %f11		C read up[i]
	add	%o1, 4, %o1		C up++
	bne,pt	%icc, .L_two_or_more
	fxtod	%f10, %f2

	fmuld	%f2, %f8, %f16
	fmuld	%f2, %f6, %f4
	fdtox	%f16, %f14
	fdtox	%f4, %f12
	std	%f14, [%o5+16]
	std	%f12, [%o5+24]
	ldx	[%o5+16], %g2		C p16
	ldx	[%o5+24], %g1		C p0
	lduw	[%o0], %g5		C read rp[i]
	b	.L1
	add	%o0, -16, %o0

	.align	16
.L_two_or_more:
	subcc	%o2, 1, %o2
	ld	[%o1], %f11		C read up[i]
	fmuld	%f2, %f8, %f16
	fmuld	%f2, %f6, %f4
	add	%o1, 4, %o1		C up++
	bne,pt	%icc, .L_three_or_more
	fxtod	%f10, %f2

	fdtox	%f16, %f14
	fdtox	%f4, %f12
	std	%f14, [%o5+16]
	fmuld	%f2, %f8, %f16
	std	%f12, [%o5+24]
	fmuld	%f2, %f6, %f4
	fdtox	%f16, %f14
	fdtox	%f4, %f12
	std	%f14, [%o5+0]
	std	%f12, [%o5+8]
	lduw	[%o0], %g5		C read rp[i]
	ldx	[%o5+16], %g2		C p16
	ldx	[%o5+24], %g1		C p0
	b	.L2
	add	%o0, -12, %o0

	.align	16
.L_three_or_more:
	subcc	%o2, 1, %o2
	ld	[%o1], %f11		C read up[i]
	fdtox	%f16, %f14
	fdtox	%f4, %f12
	std	%f14, [%o5+16]
	fmuld	%f2, %f8, %f16
	std	%f12, [%o5+24]
	fmuld	%f2, %f6, %f4
	add	%o1, 4, %o1		C up++
	bne,pt	%icc, .L_four_or_more
	fxtod	%f10, %f2

	fdtox	%f16, %f14
	fdtox	%f4, %f12
	std	%f14, [%o5+0]
	fmuld	%f2, %f8, %f16
	std	%f12, [%o5+8]
	fmuld	%f2, %f6, %f4
	fdtox	%f16, %f14
	ldx	[%o5+16], %g2		C p16
	fdtox	%f4, %f12
	ldx	[%o5+24], %g1		C p0
	std	%f14, [%o5+16]
	std	%f12, [%o5+24]
	lduw	[%o0], %g5		C read rp[i]
	b	.L3
	add	%o0, -8, %o0

	.align	16
.L_four_or_more:
	subcc	%o2, 1, %o2
	ld	[%o1], %f11		C read up[i]
	fdtox	%f16, %f14
	fdtox	%f4, %f12
	std	%f14, [%o5+0]
	fmuld	%f2, %f8, %f16
	std	%f12, [%o5+8]
	fmuld	%f2, %f6, %f4
	add	%o1, 4, %o1		C up++
	bne,pt	%icc, .L_five_or_more
	fxtod	%f10, %f2

	fdtox	%f16, %f14
	ldx	[%o5+16], %g2		C p16
	fdtox	%f4, %f12
	ldx	[%o5+24], %g1		C p0
	std	%f14, [%o5+16]
	fmuld	%f2, %f8, %f16
	std	%f12, [%o5+24]
	fmuld	%f2, %f6, %f4
	add	%o1, 4, %o1		C up++
	lduw	[%o0], %g5		C read rp[i]
	b	.L4
	add	%o0, -4, %o0

	.align	16
.L_five_or_more:
	subcc	%o2, 1, %o2
	ld	[%o1], %f11		C read up[i]
	fdtox	%f16, %f14
	ldx	[%o5+16], %g2		C p16
	fdtox	%f4, %f12
	ldx	[%o5+24], %g1		C p0
	std	%f14, [%o5+16]
	fmuld	%f2, %f8, %f16
	std	%f12, [%o5+24]
	fmuld	%f2, %f6, %f4
	add	%o1, 4, %o1		C up++
	lduw	[%o0], %g5		C read rp[i]
	bne,pt	%icc, .Loop
	fxtod	%f10, %f2
	b,a	.L5

C BEGIN MAIN LOOP
	.align 16
C -- 0
.Loop:	sub	%g0, %g3, %g3
	subcc	%o2, 1, %o2
	ld	[%o1], %f11		C read up[i]
	fdtox	%f16, %f14
C -- 1
	sllx	%g2, 16, %g4		C (p16 << 16)
	add	%o0, 4, %o0		C rp++
	ldx	[%o5+0], %g2		C p16
	fdtox	%f4, %f12
C -- 2
	srl	%g3, 0, %g3		C zero most significant 32 bits
	add	%g1, %g4, %g4		C p = p0 + (p16 << 16)
	ldx	[%o5+8], %g1		C p0
	fanop
C -- 3
	nop
	add	%g3, %g4, %g4		C p += cy
	std	%f14, [%o5+0]
	fmuld	%f2, %f8, %f16
C -- 4
	nop
	sub	%g5, %g4, %g4		C p += rp[i]
	std	%f12, [%o5+8]
	fmuld	%f2, %f6, %f4
C -- 5
	xor	%o5, 16, %o5		C alternate scratch variables
	add	%o1, 4, %o1		C up++
	stw	%g4, [%o0-4]
	fanop
C -- 6
	srlx	%g4, 32, %g3		C new cy
	lduw	[%o0], %g5		C read rp[i]
	bne,pt	%icc, .Loop
	fxtod	%f10, %f2
C END MAIN LOOP

.L5:	sub	%g0, %g3, %g3
	fdtox	%f16, %f14
	sllx	%g2, 16, %g4		C (p16 << 16)
	ldx	[%o5+0], %g2		C p16
	fdtox	%f4, %f12
	srl	%g3, 0, %g3		C zero most significant 32 bits
	add	%g1, %g4, %g4		C p = p0 + (p16 << 16)
	ldx	[%o5+8], %g1		C p0
	add	%g4, %g3, %g4		C p += cy
	std	%f14, [%o5+0]
	fmuld	%f2, %f8, %f16
	sub	%g5, %g4, %g4		C p += rp[i]
	std	%f12, [%o5+8]
	fmuld	%f2, %f6, %f4
	xor	%o5, 16, %o5
	stw	%g4, [%o0+0]
	srlx	%g4, 32, %g3		C new cy
	lduw	[%o0+4], %g5		C read rp[i]

	sub	%g0, %g3, %g3
.L4:	fdtox	%f16, %f14
	sllx	%g2, 16, %g4		C (p16 << 16)
	ldx	[%o5+0], %g2		C p16
	fdtox	%f4, %f12
	srl	%g3, 0, %g3		C zero most significant 32 bits
	add	%g1, %g4, %g4		C p = p0 + (p16 << 16)
	ldx	[%o5+8], %g1		C p0
	add	%g3, %g4, %g4		C p += cy
	std	%f14, [%o5+0]
	sub	%g5, %g4, %g4		C p += rp[i]
	std	%f12, [%o5+8]
	xor	%o5, 16, %o5
	stw	%g4, [%o0+4]
	srlx	%g4, 32, %g3		C new cy
	lduw	[%o0+8], %g5		C read rp[i]

	sub	%g0, %g3, %g3
.L3:	sllx	%g2, 16, %g4		C (p16 << 16)
	ldx	[%o5+0], %g2		C p16
	srl	%g3, 0, %g3		C zero most significant 32 bits
	add	%g1, %g4, %g4		C p = p0 + (p16 << 16)
	ldx	[%o5+8], %g1		C p0
	add	%g3, %g4, %g4		C p += cy
	sub	%g5, %g4, %g4		C p += rp[i]
	xor	%o5, 16, %o5
	stw	%g4, [%o0+8]
	srlx	%g4, 32, %g3		C new cy
	lduw	[%o0+12], %g5		C read rp[i]

	sub	%g0, %g3, %g3
.L2:	sllx	%g2, 16, %g4		C (p16 << 16)
	ldx	[%o5+0], %g2		C p16
	srl	%g3, 0, %g3		C zero most significant 32 bits
	add	%g1, %g4, %g4		C p = p0 + (p16 << 16)
	ldx	[%o5+8], %g1		C p0
	add	%g3, %g4, %g4		C p += cy
	sub	%g5, %g4, %g4		C p += rp[i]
	stw	%g4, [%o0+12]
	srlx	%g4, 32, %g3		C new cy
	lduw	[%o0+16], %g5		C read rp[i]

	sub	%g0, %g3, %g3
.L1:	sllx	%g2, 16, %g4		C (p16 << 16)
	srl	%g3, 0, %g3		C zero most significant 32 bits
	add	%g1, %g4, %g4		C p = p0 + (p16 << 16)
	add	%g3, %g4, %g4		C p += cy
	sub	%g5, %g4, %g4		C p += rp[i]
	stw	%g4, [%o0+16]
	srlx	%g4, 32, %g3		C new cy

	sub	%g0, %g3, %o0
	retl
	sub	%sp, -FSIZE, %sp
EPILOGUE(mpn_submul_1)
