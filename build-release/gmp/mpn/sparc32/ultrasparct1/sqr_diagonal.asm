dnl  SPARC T1 32-bit mpn_sqr_diagonal.

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

C INPUT PARAMETERS
define(`rp',	`%o0')
define(`up',	`%o1')
define(`n',	`%o2')

ASM_START()
PROLOGUE(mpn_sqr_diagonal)
	deccc	n			C n--
	nop

L(top):	lduw	[up+0], %g1
	add	up, 4, up		C up++
	mulx	%g1, %g1, %g3
	stw	%g3, [rp+0]
	srlx	%g3, 32, %g4
	stw	%g4, [rp+4]
	add	rp, 8, rp		C rp += 2
	bnz	%icc, L(top)
	deccc	n			C n--

	retl
	nop
EPILOGUE()
