dnl  SPARC T1 32-bit mpn_addmul_1.

dnl  Copyright 2010 Free Software Foundation, Inc.

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

C		   cycles/limb
C UltraSPARC T1:       27

C INPUT PARAMETERS
define(`rp',	`%o0')
define(`up',	`%o1')
define(`n',	`%o2')
define(`v0',	`%o3')

ASM_START()
PROLOGUE(mpn_addmul_1)
	mov	0, %g4
	srl	v0, 0, v0
	srl	n, 0, n
	dec	n			C n--

L(top):	lduw	[up+0], %g1
	add	up, 4, up		C up++
	mulx	%g1, v0, %g3
	lduw	[rp+0], %g2
	add	%g2, %g3, %g3
	add	%g4, %g3, %g3
	stw	%g3, [rp+0]
	add	rp, 4, rp		C rp++
	srlx	%g3, 32, %g4
	brnz	n, L(top)
	dec	n			C n--

	retl
	mov	%g4, %o0		C return value
EPILOGUE()
