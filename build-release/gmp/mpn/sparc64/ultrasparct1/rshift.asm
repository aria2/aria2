dnl  SPARC v9 mpn_rshift for T1/T2.

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
C UltraSPARC T1:	17
C UltraSPARC T2:	 ?

C INPUT PARAMETERS
define(`rp', `%o0')
define(`up', `%o1')
define(`vp', `%o2')
define(`n',  `%o3')
define(`cy', `%o4')

ASM_START()
	REGISTER(%g2,#scratch)
	REGISTER(%g3,#scratch)
PROLOGUE(mpn_rshift)
	add	%o1, 0, %o1
	add	%o0, -16, %g1
	sllx	%o2, 3, %g5
	add	%o1, %g5, %o1
	add	%g1, %g5, %g1
	neg	%g5
	sub	%g0, %o3, %o4
	ldx	[%o1+%g5], %g2
	add	%g5, 8, %g5
	brz,pn	%g5, L(end)
	srlx	%g2, %o3, %g4

L(top):	ldx	[%o1+%g5], %o5
	add	%g5, 8, %g5
	sllx	%o5, %o4, %g3
	or	%g4, %g3, %g3
	srlx	%o5, %o3, %g4
	stx	%g3, [%g1+%g5]
	brnz	%g5, L(top)
	nop

L(end):	stx	%g4, [%g1+8]
	retl
	 sllx	%g2, %o4, %o0
EPILOGUE()
