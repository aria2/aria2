dnl  SPARC64 mpn_gcd_1.

dnl  Based on the K7 gcd_1.asm, by Kevin Ryde.  Rehacked for SPARC by Torbjorn
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


C		  cycles/bit (approx)
C UltraSPARC 1&2:      5.1
C UltraSPARC 3:        5.0
C UltraSPARC T1:      12.8
C Numbers measured with: speed -CD -s32-64 -t32 mpn_gcd_1

C ctz_table[n] is the number of trailing zeros on n, or MAXSHIFT if n==0.

deflit(MAXSHIFT, 7)
deflit(MASK, eval((m4_lshift(1,MAXSHIFT))-1))

	.section	".rodata"
ctz_table:
	.byte	MAXSHIFT
forloop(i,1,MASK,
`	.byte	m4_count_trailing_zeros(i)
')


C Threshold of when to call bmod when U is one limb.  Should be about
C (time_in_cycles(bmod_1,1) + call_overhead) / (cycles/bit).
define(`BMOD_THRES_LOG2', 14)

C INPUT PARAMETERS
define(`up',    `%i0')
define(`n',     `%i1')
define(`v0',    `%i2')


ASM_START()
	REGISTER(%g2,#scratch)
	REGISTER(%g3,#scratch)
PROLOGUE(mpn_gcd_1)
	save	%sp, -192, %sp
	ldx	[up+0], %g1		C U low limb
	mov	-1, %i4
	or	v0, %g1, %g2		C x | y

L(twos):
	inc	%i4
	andcc	%g2, 1, %g0
	bz,a	%xcc, L(twos)
	 srlx	%g2, 1, %g2

L(divide_strip_y):
	andcc	v0, 1, %g0
	bz,a	%xcc, L(divide_strip_y)
	 srlx	v0, 1, v0

	cmp	n, 1			C if n > 1 we need
	bnz	%xcc, L(bmod)		C to call bmod_1
	 nop

C Both U and V are single limbs, reduce with bmod if u0 >> v0.
	srlx	%g1, BMOD_THRES_LOG2, %g2
	cmp	%g2, v0
	bleu	%xcc, L(noreduce)
	 mov	%g1, %o0

L(bmod):
	mov	up, %o0
	mov	n, %o1
	mov	v0, %o2
	call	mpn_modexact_1c_odd
	 mov	0, %o3

L(noreduce):

ifdef(`PIC',`
	sethi	%hi(_GLOBAL_OFFSET_TABLE_-4), %l7
	call	L(LGETPC0)
	add	%l7, %lo(_GLOBAL_OFFSET_TABLE_+4), %l7
	sethi	%hi(ctz_table), %g1
	or	%g1, %lo(ctz_table), %g1
	ldx	[%l7+%g1], %i5
',`
	sethi	%hh(ctz_table), %l7
	or	%l7, %hm(ctz_table), %l7
	sllx	%l7, 32, %l7
	sethi	%lm(ctz_table), %g1
	add	%l7, %g1, %l7
	or	%l7, %lo(ctz_table), %i5
')

	cmp	%o0, 0
	bnz	%xcc, L(mid)
	 andcc	%o0, MASK, %g3		C

	return	%i7+8
	 sllx	%o2, %o4, %o0		C CAUTION: v0 alias for o2

	ALIGN(16)
L(top):	movcc	%xcc, %l4, v0		C v = min(u,v)
	movcc	%xcc, %l2, %o0		C u = |v - u]
	cmp	%g3, 0			C are all MAXSHIFT low bits zero?
L(mid):	ldub	[%i5+%g3], %g3		C
	bz,a	%xcc, L(shift_alot)	C
	 srlx	%o0, MAXSHIFT, %o0
	srlx	%o0, %g3, %l4		C new u, odd
	nop				C force parallel exec of sub insns
	subcc	v0, %l4, %l2		C v - u, set flags for branch and movcc
	sub	%l4, v0, %o0		C u - v
	bnz	%xcc, L(top)		C
	 and	%l2, MASK, %g3		C extract low MAXSHIFT bits from (v-u)

	return	%i7+8
	 sllx	%o2, %o4, %o0		C CAUTION: v0 alias for o2

L(shift_alot):
	b	L(mid)
	 andcc	%o0, MASK, %g3		C

ifdef(`PIC',`
L(LGETPC0):
	retl
	add	%o7, %l7, %l7
')
EPILOGUE()
