dnl  VAX mpn_addmul_1 -- Multiply a limb vector with a limb and add the result
dnl  to a second limb vector.

dnl  Copyright 1992, 1994, 1996, 2000, 2012 Free Software Foundation, Inc.

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
PROLOGUE(mpn_addmul_1)
	.word	0xfc0
	movl	12(ap), r4
	movl	8(ap), r8
	movl	4(ap), r9
	clrl	r3
	incl	r4
	ashl	$-1, r4, r7
	clrl	r11
	movl	16(ap), r6
	jlss	L(v0_big)
	jlbc	r4, L(1)

C Loop for v0 < 0x80000000
L(tp1):	movl	(r8)+, r1
	jlss	L(1n0)
	emul	r1, r6, $0, r2
	addl2	r11, r2
	adwc	$0, r3
	addl2	r2, (r9)+
	adwc	$0, r3
L(1):	movl	(r8)+, r1
	jlss	L(1n1)
L(1p1):	emul	r1, r6, $0, r10
	addl2	r3, r10
	adwc	$0, r11
	addl2	r10, (r9)+
	adwc	$0, r11

	sobgtr	r7, L(tp1)
	movl	r11, r0
	ret

L(1n0):	emul	r1, r6, $0, r2
	addl2	r11, r2
	adwc	r6, r3
	addl2	r2, (r9)+
	adwc	$0, r3
	movl	(r8)+, r1
	jgeq	L(1p1)
L(1n1):	emul	r1, r6, $0, r10
	addl2	r3, r10
	adwc	r6, r11
	addl2	r10, (r9)+
	adwc	$0, r11

	sobgtr	r7, L(tp1)
	movl	r11, r0
	ret

L(v0_big):
	jlbc	r4, L(2)

C Loop for v0 >= 0x80000000
L(tp2):	movl	(r8)+, r1
	jlss	L(2n0)
	emul	r1, r6, $0, r2
	addl2	r11, r2
	adwc	r1, r3
	addl2	r2, (r9)+
	adwc	$0, r3
L(2):	movl	(r8)+, r1
	jlss	L(2n1)
L(2p1):	emul	r1, r6, $0, r10
	addl2	r3, r10
	adwc	r1, r11
	addl2	r10, (r9)+
	adwc	$0, r11

	sobgtr	r7, L(tp2)
	movl	r11, r0
	ret

L(2n0):	emul	r1, r6, $0, r2
	addl2	r11, r2
	adwc	r6, r3
	addl2	r2, (r9)+
	adwc	r1, r3
	movl	(r8)+, r1
	jgeq	L(2p1)
L(2n1):	emul	r1, r6, $0, r10
	addl2	r3, r10
	adwc	r6, r11
	addl2	r10, (r9)+
	adwc	r1, r11

	sobgtr	r7, L(tp2)
	movl	r11, r0
	ret
EPILOGUE()
