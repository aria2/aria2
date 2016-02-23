dnl  SPARC v9 mpn_sub_n for T1/T2.

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
C UltraSPARC T1:	 ?
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
PROLOGUE(mpn_sub_nc)
	b,a	L(ent)
EPILOGUE()
PROLOGUE(mpn_sub_n)
	mov	0, cy
L(ent):	cmp	%g0, cy
L(top):	ldx	[up+0], %o4
	add	up, 8, up
	ldx	[vp+0], %o5
	add	vp, 8, vp
	add	rp, 8, rp
	add	n, -1, n
	srlx	%o4, 32, %g1
	srlx	%o5, 32, %g2
	subccc	%o4, %o5, %g3
	subccc	%g1, %g2, %g0
	brgz	n, L(top)
	 stx	%g3, [rp-8]

	retl
	addc	%g0, %g0, %o0
EPILOGUE()
