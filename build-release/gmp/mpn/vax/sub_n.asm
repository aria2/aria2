dnl  VAX mpn_sub_n -- Subtract two limb vectors of the same length > 0 and
dnl  store difference in a third limb vector.

dnl  Copyright 1999, 2000, 2012 Free Software Foundation, Inc.

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

ASM_START()
PROLOGUE(mpn_sub_n)
	.word	0x0
	movl	16(ap), r0
	movl	12(ap), r1
	movl	8(ap), r2
	movl	4(ap), r3
	mnegl	r0, r5
	addl2	$3, r0
	ashl	$-2, r0, r0	C unroll loop count
	bicl2	$-4, r5		C mask out low 2 bits
	movaq	(r5)[r5], r5	C 9x
	jmp	L(top)[r5]

L(top):	movl	(r2)+, r4
	sbwc	(r1)+, r4
	movl	r4, (r3)+
	movl	(r2)+, r4
	sbwc	(r1)+, r4
	movl	r4, (r3)+
	movl	(r2)+, r4
	sbwc	(r1)+, r4
	movl	r4, (r3)+
	movl	(r2)+, r4
	sbwc	(r1)+, r4
	movl	r4, (r3)+
	sobgtr	r0, L(top)

	adwc	r0, r0
	ret
EPILOGUE()
