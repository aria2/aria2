dnl  SPARC mpn_submul_1 -- Multiply a limb vector with a limb and subtract
dnl  the result from a second limb vector.

dnl  Copyright 1992, 1993, 1994, 2000 Free Software Foundation, Inc.

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

C INPUT PARAMETERS
C res_ptr	o0
C s1_ptr	o1
C size		o2
C s2_limb	o3

ASM_START()
PROLOGUE(mpn_submul_1)
	C Make S1_PTR and RES_PTR point at the end of their blocks
	C and put (- 4 x SIZE) in index/loop counter.
	sll	%o2,2,%o2
	add	%o0,%o2,%o4	C RES_PTR in o4 since o0 is retval
	add	%o1,%o2,%o1
	sub	%g0,%o2,%o2

	cmp	%o3,0xfff
	bgu	L(large)
	nop

	ld	[%o1+%o2],%o5
	mov	0,%o0
	b	L(0)
	 add	%o4,-4,%o4
L(loop0):
	subcc	%o5,%g1,%g1
	ld	[%o1+%o2],%o5
	addx	%o0,%g0,%o0
	st	%g1,[%o4+%o2]
L(0):	wr	%g0,%o3,%y
	sra	%o5,31,%g2
	and	%o3,%g2,%g2
	andcc	%g1,0,%g1
	mulscc	%g1,%o5,%g1
	mulscc	%g1,%o5,%g1
	mulscc	%g1,%o5,%g1
	mulscc	%g1,%o5,%g1
	mulscc	%g1,%o5,%g1
	mulscc	%g1,%o5,%g1
	mulscc	%g1,%o5,%g1
	mulscc	%g1,%o5,%g1
	mulscc	%g1,%o5,%g1
	mulscc	%g1,%o5,%g1
	mulscc	%g1,%o5,%g1
	mulscc	%g1,%o5,%g1
	mulscc	%g1,0,%g1
	sra	%g1,20,%g4
	sll	%g1,12,%g1
	rd	%y,%g3
	srl	%g3,20,%g3
	or	%g1,%g3,%g1

	addcc	%g1,%o0,%g1
	addx	%g2,%g4,%o0	C add sign-compensation and cy to hi limb
	addcc	%o2,4,%o2	C loop counter
	bne	L(loop0)
	 ld	[%o4+%o2],%o5

	subcc	%o5,%g1,%g1
	addx	%o0,%g0,%o0
	retl
	st	%g1,[%o4+%o2]

L(large):
	ld	[%o1+%o2],%o5
	mov	0,%o0
	sra	%o3,31,%g4	C g4 = mask of ones iff S2_LIMB < 0
	b	L(1)
	 add	%o4,-4,%o4
L(loop):
	subcc	%o5,%g3,%g3
	ld	[%o1+%o2],%o5
	addx	%o0,%g0,%o0
	st	%g3,[%o4+%o2]
L(1):	wr	%g0,%o5,%y
	and	%o5,%g4,%g2
	andcc	%g0,%g0,%g1
	mulscc	%g1,%o3,%g1
	mulscc	%g1,%o3,%g1
	mulscc	%g1,%o3,%g1
	mulscc	%g1,%o3,%g1
	mulscc	%g1,%o3,%g1
	mulscc	%g1,%o3,%g1
	mulscc	%g1,%o3,%g1
	mulscc	%g1,%o3,%g1
	mulscc	%g1,%o3,%g1
	mulscc	%g1,%o3,%g1
	mulscc	%g1,%o3,%g1
	mulscc	%g1,%o3,%g1
	mulscc	%g1,%o3,%g1
	mulscc	%g1,%o3,%g1
	mulscc	%g1,%o3,%g1
	mulscc	%g1,%o3,%g1
	mulscc	%g1,%o3,%g1
	mulscc	%g1,%o3,%g1
	mulscc	%g1,%o3,%g1
	mulscc	%g1,%o3,%g1
	mulscc	%g1,%o3,%g1
	mulscc	%g1,%o3,%g1
	mulscc	%g1,%o3,%g1
	mulscc	%g1,%o3,%g1
	mulscc	%g1,%o3,%g1
	mulscc	%g1,%o3,%g1
	mulscc	%g1,%o3,%g1
	mulscc	%g1,%o3,%g1
	mulscc	%g1,%o3,%g1
	mulscc	%g1,%o3,%g1
	mulscc	%g1,%o3,%g1
	mulscc	%g1,%o3,%g1
	mulscc	%g1,%g0,%g1
	rd	%y,%g3
	addcc	%g3,%o0,%g3
	addx	%g2,%g1,%o0
	addcc	%o2,4,%o2
	bne	L(loop)
	 ld	[%o4+%o2],%o5

	subcc	%o5,%g3,%g3
	addx	%o0,%g0,%o0
	retl
	st	%g3,[%o4+%o2]
EPILOGUE(mpn_submul_1)
