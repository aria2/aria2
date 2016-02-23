dnl  SPARC v9 mpn_submul_1 for T1/T2.

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
C UltraSPARC T1:	74
C UltraSPARC T2:	 ?

C INPUT PARAMETERS
define(`rp', `%i0')
define(`up', `%i1')
define(`n',  `%i2')
define(`v0', `%i3')

ASM_START()
	REGISTER(%g2,#scratch)
	REGISTER(%g3,#scratch)
PROLOGUE(mpn_submul_1)
	save	%sp, -176, %sp
	mov	1, %o2
	mov	%i0, %g2
	srlx	%i3, 32, %o4
	sllx	%o2, 32, %o2
	srl	%i3, 0, %i3
	mov	0, %g3
	mov	0, %i0

L(top):	ldx	[%i1+%g3], %g1
	srl	%g1, 0, %g4
	mulx	%g4, %i3, %o5
	srlx	%g1, 32, %g1
	mulx	%g1, %i3, %g5
	mulx	%g4, %o4, %g4
	mulx	%g1, %o4, %g1
	srlx	%o5, 32, %o1
	add	%g5, %o1, %o1
	addcc	%o1, %g4, %g4
	srl	%o5, 0, %o0
	ldx	[%g2+%g3], %o5
	sllx	%g4, 32, %o1
	add	%g1, %o2, %l1
	movlu	%xcc, %l1, %g1
	add	%o1, %o0, %l0
	addcc	%l0, %i0, %g5
	srlx	%g4, 32, %i0
	add	%i0, 1, %g4
	movlu	%xcc, %g4, %i0
	subcc	%o5, %g5, %g5
	stx	%g5, [%g2+%g3]
	add	%i0, 1, %g4
	movlu	%xcc, %g4, %i0
	add	%i2, -1, %i2
	add	%i0, %g1, %i0
	brnz,pt	%i2, L(top)
	 add	%g3, 8, %g3
	return	%i7+8
	 nop
EPILOGUE()
