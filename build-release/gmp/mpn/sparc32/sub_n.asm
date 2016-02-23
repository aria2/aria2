dnl  SPARC mpn_sub_n -- Subtract two limb vectors of the same length > 0 and
dnl  store difference in a third limb vector.

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
define(res_ptr,%o0)
define(s1_ptr,%o1)
define(s2_ptr,%o2)
define(n,%o3)

ASM_START()
PROLOGUE(mpn_sub_n)
	xor	s2_ptr,res_ptr,%g1
	andcc	%g1,4,%g0
	bne	L(1)			C branch if alignment differs
	nop
C **  V1a  **
	andcc	res_ptr,4,%g0		C res_ptr unaligned? Side effect: cy=0
	be	L(v1)			C if no, branch
	nop
C Add least significant limb separately to align res_ptr and s2_ptr
	ld	[s1_ptr],%g4
	add	s1_ptr,4,s1_ptr
	ld	[s2_ptr],%g2
	add	s2_ptr,4,s2_ptr
	add	n,-1,n
	subcc	%g4,%g2,%o4
	st	%o4,[res_ptr]
	add	res_ptr,4,res_ptr
L(v1):	addx	%g0,%g0,%o4		C save cy in register
	cmp	n,2			C if n < 2 ...
	bl	L(end2)			C ... branch to tail code
	subcc	%g0,%o4,%g0		C restore cy

	ld	[s1_ptr+0],%g4
	addcc	n,-10,n
	ld	[s1_ptr+4],%g1
	ldd	[s2_ptr+0],%g2
	blt	L(fin1)
	subcc	%g0,%o4,%g0		C restore cy
C Add blocks of 8 limbs until less than 8 limbs remain
L(loop1):
	subxcc	%g4,%g2,%o4
	ld	[s1_ptr+8],%g4
	subxcc	%g1,%g3,%o5
	ld	[s1_ptr+12],%g1
	ldd	[s2_ptr+8],%g2
	std	%o4,[res_ptr+0]
	subxcc	%g4,%g2,%o4
	ld	[s1_ptr+16],%g4
	subxcc	%g1,%g3,%o5
	ld	[s1_ptr+20],%g1
	ldd	[s2_ptr+16],%g2
	std	%o4,[res_ptr+8]
	subxcc	%g4,%g2,%o4
	ld	[s1_ptr+24],%g4
	subxcc	%g1,%g3,%o5
	ld	[s1_ptr+28],%g1
	ldd	[s2_ptr+24],%g2
	std	%o4,[res_ptr+16]
	subxcc	%g4,%g2,%o4
	ld	[s1_ptr+32],%g4
	subxcc	%g1,%g3,%o5
	ld	[s1_ptr+36],%g1
	ldd	[s2_ptr+32],%g2
	std	%o4,[res_ptr+24]
	addx	%g0,%g0,%o4		C save cy in register
	addcc	n,-8,n
	add	s1_ptr,32,s1_ptr
	add	s2_ptr,32,s2_ptr
	add	res_ptr,32,res_ptr
	bge	L(loop1)
	subcc	%g0,%o4,%g0		C restore cy

L(fin1):
	addcc	n,8-2,n
	blt	L(end1)
	subcc	%g0,%o4,%g0		C restore cy
C Add blocks of 2 limbs until less than 2 limbs remain
L(loope1):
	subxcc	%g4,%g2,%o4
	ld	[s1_ptr+8],%g4
	subxcc	%g1,%g3,%o5
	ld	[s1_ptr+12],%g1
	ldd	[s2_ptr+8],%g2
	std	%o4,[res_ptr+0]
	addx	%g0,%g0,%o4		C save cy in register
	addcc	n,-2,n
	add	s1_ptr,8,s1_ptr
	add	s2_ptr,8,s2_ptr
	add	res_ptr,8,res_ptr
	bge	L(loope1)
	subcc	%g0,%o4,%g0		C restore cy
L(end1):
	subxcc	%g4,%g2,%o4
	subxcc	%g1,%g3,%o5
	std	%o4,[res_ptr+0]
	addx	%g0,%g0,%o4		C save cy in register

	andcc	n,1,%g0
	be	L(ret1)
	subcc	%g0,%o4,%g0		C restore cy
C Add last limb
	ld	[s1_ptr+8],%g4
	ld	[s2_ptr+8],%g2
	subxcc	%g4,%g2,%o4
	st	%o4,[res_ptr+8]

L(ret1):
	retl
	addx	%g0,%g0,%o0	C return carry-out from most sign. limb

L(1):	xor	s1_ptr,res_ptr,%g1
	andcc	%g1,4,%g0
	bne	L(2)
	nop
C **  V1b  **
	andcc	res_ptr,4,%g0		C res_ptr unaligned? Side effect: cy=0
	be	L(v1b)			C if no, branch
	nop
C Add least significant limb separately to align res_ptr and s1_ptr
	ld	[s2_ptr],%g4
	add	s2_ptr,4,s2_ptr
	ld	[s1_ptr],%g2
	add	s1_ptr,4,s1_ptr
	add	n,-1,n
	subcc	%g2,%g4,%o4
	st	%o4,[res_ptr]
	add	res_ptr,4,res_ptr
L(v1b):	addx	%g0,%g0,%o4		C save cy in register
	cmp	n,2			C if n < 2 ...
	bl	L(end2)			C ... branch to tail code
	subcc	%g0,%o4,%g0		C restore cy

	ld	[s2_ptr+0],%g4
	addcc	n,-10,n
	ld	[s2_ptr+4],%g1
	ldd	[s1_ptr+0],%g2
	blt	L(fin1b)
	subcc	%g0,%o4,%g0		C restore cy
C Add blocks of 8 limbs until less than 8 limbs remain
L(loop1b):
	subxcc	%g2,%g4,%o4
	ld	[s2_ptr+8],%g4
	subxcc	%g3,%g1,%o5
	ld	[s2_ptr+12],%g1
	ldd	[s1_ptr+8],%g2
	std	%o4,[res_ptr+0]
	subxcc	%g2,%g4,%o4
	ld	[s2_ptr+16],%g4
	subxcc	%g3,%g1,%o5
	ld	[s2_ptr+20],%g1
	ldd	[s1_ptr+16],%g2
	std	%o4,[res_ptr+8]
	subxcc	%g2,%g4,%o4
	ld	[s2_ptr+24],%g4
	subxcc	%g3,%g1,%o5
	ld	[s2_ptr+28],%g1
	ldd	[s1_ptr+24],%g2
	std	%o4,[res_ptr+16]
	subxcc	%g2,%g4,%o4
	ld	[s2_ptr+32],%g4
	subxcc	%g3,%g1,%o5
	ld	[s2_ptr+36],%g1
	ldd	[s1_ptr+32],%g2
	std	%o4,[res_ptr+24]
	addx	%g0,%g0,%o4		C save cy in register
	addcc	n,-8,n
	add	s1_ptr,32,s1_ptr
	add	s2_ptr,32,s2_ptr
	add	res_ptr,32,res_ptr
	bge	L(loop1b)
	subcc	%g0,%o4,%g0		C restore cy

L(fin1b):
	addcc	n,8-2,n
	blt	L(end1b)
	subcc	%g0,%o4,%g0		C restore cy
C Add blocks of 2 limbs until less than 2 limbs remain
L(loope1b):
	subxcc	%g2,%g4,%o4
	ld	[s2_ptr+8],%g4
	subxcc	%g3,%g1,%o5
	ld	[s2_ptr+12],%g1
	ldd	[s1_ptr+8],%g2
	std	%o4,[res_ptr+0]
	addx	%g0,%g0,%o4		C save cy in register
	addcc	n,-2,n
	add	s1_ptr,8,s1_ptr
	add	s2_ptr,8,s2_ptr
	add	res_ptr,8,res_ptr
	bge	L(loope1b)
	subcc	%g0,%o4,%g0		C restore cy
L(end1b):
	subxcc	%g2,%g4,%o4
	subxcc	%g3,%g1,%o5
	std	%o4,[res_ptr+0]
	addx	%g0,%g0,%o4		C save cy in register

	andcc	n,1,%g0
	be	L(ret1b)
	subcc	%g0,%o4,%g0		C restore cy
C Add last limb
	ld	[s2_ptr+8],%g4
	ld	[s1_ptr+8],%g2
	subxcc	%g2,%g4,%o4
	st	%o4,[res_ptr+8]

L(ret1b):
	retl
	addx	%g0,%g0,%o0		C return carry-out from most sign. limb

C **  V2  **
C If we come here, the alignment of s1_ptr and res_ptr as well as the
C alignment of s2_ptr and res_ptr differ.  Since there are only two ways
C things can be aligned (that we care about) we now know that the alignment
C of s1_ptr and s2_ptr are the same.

L(2):	cmp	n,1
	be	L(jone)
	nop
	andcc	s1_ptr,4,%g0		C s1_ptr unaligned? Side effect: cy=0
	be	L(v2)			C if no, branch
	nop
C Add least significant limb separately to align s1_ptr and s2_ptr
	ld	[s1_ptr],%g4
	add	s1_ptr,4,s1_ptr
	ld	[s2_ptr],%g2
	add	s2_ptr,4,s2_ptr
	add	n,-1,n
	subcc	%g4,%g2,%o4
	st	%o4,[res_ptr]
	add	res_ptr,4,res_ptr

L(v2):	addx	%g0,%g0,%o4		C save cy in register
	addcc	n,-8,n
	blt	L(fin2)
	subcc	%g0,%o4,%g0		C restore cy
C Add blocks of 8 limbs until less than 8 limbs remain
L(loop2):
	ldd	[s1_ptr+0],%g2
	ldd	[s2_ptr+0],%o4
	subxcc	%g2,%o4,%g2
	st	%g2,[res_ptr+0]
	subxcc	%g3,%o5,%g3
	st	%g3,[res_ptr+4]
	ldd	[s1_ptr+8],%g2
	ldd	[s2_ptr+8],%o4
	subxcc	%g2,%o4,%g2
	st	%g2,[res_ptr+8]
	subxcc	%g3,%o5,%g3
	st	%g3,[res_ptr+12]
	ldd	[s1_ptr+16],%g2
	ldd	[s2_ptr+16],%o4
	subxcc	%g2,%o4,%g2
	st	%g2,[res_ptr+16]
	subxcc	%g3,%o5,%g3
	st	%g3,[res_ptr+20]
	ldd	[s1_ptr+24],%g2
	ldd	[s2_ptr+24],%o4
	subxcc	%g2,%o4,%g2
	st	%g2,[res_ptr+24]
	subxcc	%g3,%o5,%g3
	st	%g3,[res_ptr+28]
	addx	%g0,%g0,%o4		C save cy in register
	addcc	n,-8,n
	add	s1_ptr,32,s1_ptr
	add	s2_ptr,32,s2_ptr
	add	res_ptr,32,res_ptr
	bge	L(loop2)
	subcc	%g0,%o4,%g0		C restore cy

L(fin2):
	addcc	n,8-2,n
	blt	L(end2)
	subcc	%g0,%o4,%g0		C restore cy
L(loope2):
	ldd	[s1_ptr+0],%g2
	ldd	[s2_ptr+0],%o4
	subxcc	%g2,%o4,%g2
	st	%g2,[res_ptr+0]
	subxcc	%g3,%o5,%g3
	st	%g3,[res_ptr+4]
	addx	%g0,%g0,%o4		C save cy in register
	addcc	n,-2,n
	add	s1_ptr,8,s1_ptr
	add	s2_ptr,8,s2_ptr
	add	res_ptr,8,res_ptr
	bge	L(loope2)
	subcc	%g0,%o4,%g0		C restore cy
L(end2):
	andcc	n,1,%g0
	be	L(ret2)
	subcc	%g0,%o4,%g0		C restore cy
C Add last limb
L(jone):
	ld	[s1_ptr],%g4
	ld	[s2_ptr],%g2
	subxcc	%g4,%g2,%o4
	st	%o4,[res_ptr]

L(ret2):
	retl
	addx	%g0,%g0,%o0		C return carry-out from most sign. limb
EPILOGUE(mpn_sub_n)
