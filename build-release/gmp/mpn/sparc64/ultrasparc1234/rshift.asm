dnl  SPARC v9 mpn_rshift

dnl  Copyright 1996, 2000, 2001, 2002, 2003, 2010 Free Software Foundation,
dnl  Inc.

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
C UltraSPARC 1&2:     2
C UltraSPARC 3:	      2.5	(for some up/rp alignments)

C INPUT PARAMETERS
define(`rp', `%i0')
define(`up', `%i1')
define(`n',  `%i2')
define(`cnt',`%i3')

define(`u0', `%l0')
define(`u1', `%l2')
define(`u2', `%l4')
define(`u3', `%l6')

define(`tnc',`%i4')

define(`fanop',`fitod %f0,%f2')		dnl  A quasi nop running in the FA pipe

ASM_START()
	REGISTER(%g2,#scratch)
	REGISTER(%g3,#scratch)
PROLOGUE(mpn_rshift)
	save	%sp,-160,%sp

	sub	%g0,cnt,tnc		C negate shift count
	ldx	[up],u3			C load first limb
	subcc	n,5,n
	sllx	u3,tnc,%i5		C compute function result
	bl,pn	%xcc,.Lend1234
	srlx	u3,cnt,%g3

	subcc	n,4,n
	ldx	[up+8],u0
	ldx	[up+16],u1
	add	up,32,up
	ldx	[up-8],u2
	ldx	[up+0],u3

	bl,pn	%xcc,.Lend5678
	sllx	u0,tnc,%g2

	b,a	.Loop
	ALIGN(16)
.Loop:
	srlx	u0,cnt,%g1
	or	%g3,%g2,%g3
	ldx	[up+8],u0
	fanop
C --
	sllx	u1,tnc,%g2
	subcc	n,4,n
	stx	%g3,[rp+0]
	fanop
C --
	srlx	u1,cnt,%g3
	or	%g1,%g2,%g1
	ldx	[up+16],u1
	fanop
C --
	sllx	u2,tnc,%g2
	stx	%g1,[rp+8]
	add	up,32,up
	fanop
C --
	srlx	u2,cnt,%g1
	or	%g3,%g2,%g3
	ldx	[up-8],u2
	fanop
C --
	sllx	u3,tnc,%g2
	stx	%g3,[rp+16]
	add	rp,32,rp
	fanop
C --
	srlx	u3,cnt,%g3
	or	%g1,%g2,%g1
	ldx	[up+0],u3
	fanop
C --
	sllx	u0,tnc,%g2
	stx	%g1,[rp-8]
	bge,pt	%xcc,.Loop
	fanop
C --
.Lend5678:
	srlx	u0,cnt,%g1
	or	%g3,%g2,%g3
	sllx	u1,tnc,%g2
	stx	%g3,[rp+0]
	srlx	u1,cnt,%g3
	or	%g1,%g2,%g1
	sllx	u2,tnc,%g2
	stx	%g1,[rp+8]
	srlx	u2,cnt,%g1
	or	%g3,%g2,%g3
	sllx	u3,tnc,%g2
	stx	%g3,[rp+16]
	add	rp,32,rp
	srlx	u3,cnt,%g3		C carry...
	or	%g1,%g2,%g1
	stx	%g1,[rp-8]

.Lend1234:
	addcc	n,4,n
	bz,pn	%xcc,.Lret
	fanop
.Loop0:
	add	rp,8,rp
	subcc	n,1,n
	ldx	[up+8],u3
	add	up,8,up
	sllx	u3,tnc,%g2
	or	%g3,%g2,%g3
	stx	%g3,[rp-8]
	srlx	u3,cnt,%g3
	bnz,pt	%xcc,.Loop0
	fanop
.Lret:
	stx	%g3,[rp+0]
	mov	%i5,%i0
	ret
	restore
EPILOGUE(mpn_rshift)
