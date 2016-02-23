dnl  SPARC mpn_rshift -- Shift a number right.

dnl  Copyright 1995, 1996, 2000 Free Software Foundation, Inc.

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
C res_ptr	%o0
C src_ptr	%o1
C size		%o2
C cnt		%o3

ASM_START()
PROLOGUE(mpn_rshift)
	ld	[%o1],%g2	C load first limb
	sub	%g0,%o3,%o5	C negate shift count
	add	%o2,-1,%o2
	andcc	%o2,4-1,%g4	C number of limbs in first loop
	sll	%g2,%o5,%g1	C compute function result
	be	L(0)		C if multiple of 4 limbs, skip first loop
	st	%g1,[%sp+80]

	sub	%o2,%g4,%o2	C adjust count for main loop

L(loop0):
	ld	[%o1+4],%g3
	add	%o0,4,%o0
	add	%o1,4,%o1
	addcc	%g4,-1,%g4
	srl	%g2,%o3,%o4
	sll	%g3,%o5,%g1
	mov	%g3,%g2
	or	%o4,%g1,%o4
	bne	L(loop0)
	 st	%o4,[%o0-4]

L(0):	tst	%o2
	be	L(end)
	 nop

L(loop):
	ld	[%o1+4],%g3
	add	%o0,16,%o0
	addcc	%o2,-4,%o2
	srl	%g2,%o3,%o4
	sll	%g3,%o5,%g1

	ld	[%o1+8],%g2
	srl	%g3,%o3,%g4
	or	%o4,%g1,%o4
	st	%o4,[%o0-16]
	sll	%g2,%o5,%g1

	ld	[%o1+12],%g3
	srl	%g2,%o3,%o4
	or	%g4,%g1,%g4
	st	%g4,[%o0-12]
	sll	%g3,%o5,%g1

	ld	[%o1+16],%g2
	srl	%g3,%o3,%g4
	or	%o4,%g1,%o4
	st	%o4,[%o0-8]
	sll	%g2,%o5,%g1

	add	%o1,16,%o1
	or	%g4,%g1,%g4
	bne	L(loop)
	 st	%g4,[%o0-4]

L(end):	srl	%g2,%o3,%g2
	st	%g2,[%o0-0]
	retl
	ld	[%sp+80],%o0
EPILOGUE(mpn_rshift)
