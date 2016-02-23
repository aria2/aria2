dnl  SPARC v9 mpn_sub_n -- Subtract two limb vectors of the same length > 0 and
dnl  store difference in a third limb vector.

dnl  Copyright 2001, 2002, 2003, 2011 Free Software Foundation, Inc.

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
C UltraSPARC 1&2:     4
C UltraSPARC 3:	      4.5

C Compute carry-out from the most significant bits of u,v, and r, where
C r=u-v-carry_in, using logic operations.

C This code runs at 4 cycles/limb on UltraSPARC 1 and 2.  It has a 4 insn
C recurrency, and the UltraSPARC 1 and 2 the IE units are 100% saturated.
C Therefore, it seems futile to try to optimize this any further...

C INPUT PARAMETERS
define(`rp',`%i0')
define(`up',`%i1')
define(`vp',`%i2')
define(`n',`%i3')

define(`u0',`%l0')
define(`u1',`%l2')
define(`u2',`%l4')
define(`u3',`%l6')
define(`v0',`%l1')
define(`v1',`%l3')
define(`v2',`%l5')
define(`v3',`%l7')

define(`cy',`%i4')

define(`fanop',`fitod %f0,%f2')		dnl  A quasi nop running in the FA pipe
define(`fmnop',`fmuld %f0,%f0,%f4')	dnl  A quasi nop running in the FM pipe

ASM_START()
	REGISTER(%g2,#scratch)
	REGISTER(%g3,#scratch)
PROLOGUE(mpn_sub_nc)
	save	%sp,-160,%sp

	fitod	%f0,%f0		C make sure f0 contains small, quiet number
	subcc	n,4,%g0
	bl,pn	%xcc,.Loop0
	nop
	b,a	L(com)
EPILOGUE()

PROLOGUE(mpn_sub_n)
	save	%sp,-160,%sp

	fitod	%f0,%f0		C make sure f0 contains small, quiet number
	subcc	n,4,%g0
	bl,pn	%xcc,.Loop0
	mov	0,cy
L(com):
	ldx	[up+0],u0
	ldx	[vp+0],v0
	add	up,32,up
	ldx	[up-24],u1
	ldx	[vp+8],v1
	add	vp,32,vp
	ldx	[up-16],u2
	ldx	[vp-16],v2
	ldx	[up-8],u3
	ldx	[vp-8],v3
	subcc	n,8,n
	sub	u0,v0,%g1	C main sub
	sub	%g1,cy,%g4	C carry sub
	orn	u0,v0,%g2
	bl,pn	%xcc,.Lend4567
	fanop
	b,a	.Loop

	.align	16
C START MAIN LOOP
.Loop:	orn	%g4,%g2,%g2
	andn	u0,v0,%g3
	ldx	[up+0],u0
	fanop
C --
	andn	%g2,%g3,%g2
	ldx	[vp+0],v0
	add	up,32,up
	fanop
C --
	srlx	%g2,63,cy
	sub	u1,v1,%g1
	stx	%g4,[rp+0]
	fanop
C --
	sub	%g1,cy,%g4
	orn	u1,v1,%g2
	fmnop
	fanop
C --
	orn	%g4,%g2,%g2
	andn	u1,v1,%g3
	ldx	[up-24],u1
	fanop
C --
	andn	%g2,%g3,%g2
	ldx	[vp+8],v1
	add	vp,32,vp
	fanop
C --
	srlx	%g2,63,cy
	sub	u2,v2,%g1
	stx	%g4,[rp+8]
	fanop
C --
	sub	%g1,cy,%g4
	orn	u2,v2,%g2
	fmnop
	fanop
C --
	orn	%g4,%g2,%g2
	andn	u2,v2,%g3
	ldx	[up-16],u2
	fanop
C --
	andn	%g2,%g3,%g2
	ldx	[vp-16],v2
	add	rp,32,rp
	fanop
C --
	srlx	%g2,63,cy
	sub	u3,v3,%g1
	stx	%g4,[rp-16]
	fanop
C --
	sub	%g1,cy,%g4
	orn	u3,v3,%g2
	fmnop
	fanop
C --
	orn	%g4,%g2,%g2
	andn	u3,v3,%g3
	ldx	[up-8],u3
	fanop
C --
	andn	%g2,%g3,%g2
	subcc	n,4,n
	ldx	[vp-8],v3
	fanop
C --
	srlx	%g2,63,cy
	sub	u0,v0,%g1
	stx	%g4,[rp-8]
	fanop
C --
	sub	%g1,cy,%g4
	orn	u0,v0,%g2
	bge,pt	%xcc,.Loop
	fanop
C END MAIN LOOP
.Lend4567:
	orn	%g4,%g2,%g2
	andn	u0,v0,%g3
	andn	%g2,%g3,%g2
	srlx	%g2,63,cy
	sub	u1,v1,%g1
	stx	%g4,[rp+0]
	sub	%g1,cy,%g4
	orn	u1,v1,%g2
	orn	%g4,%g2,%g2
	andn	u1,v1,%g3
	andn	%g2,%g3,%g2
	srlx	%g2,63,cy
	sub	u2,v2,%g1
	stx	%g4,[rp+8]
	sub	%g1,cy,%g4
	orn	u2,v2,%g2
	orn	%g4,%g2,%g2
	andn	u2,v2,%g3
	andn	%g2,%g3,%g2
	add	rp,32,rp
	srlx	%g2,63,cy
	sub	u3,v3,%g1
	stx	%g4,[rp-16]
	sub	%g1,cy,%g4
	orn	u3,v3,%g2
	orn	%g4,%g2,%g2
	andn	u3,v3,%g3
	andn	%g2,%g3,%g2
	srlx	%g2,63,cy
	stx	%g4,[rp-8]

	addcc	n,4,n
	bz,pn	%xcc,.Lret
	fanop

.Loop0:	ldx	[up],u0
	add	up,8,up
	ldx	[vp],v0
	add	vp,8,vp
	add	rp,8,rp
	subcc	n,1,n
	sub	u0,v0,%g1
	orn	u0,v0,%g2
	sub	%g1,cy,%g4
	andn	u0,v0,%g3
	orn	%g4,%g2,%g2
	stx	%g4,[rp-8]
	andn	%g2,%g3,%g2
	bnz,pt	%xcc,.Loop0
	srlx	%g2,63,cy

.Lret:	mov	cy,%i0
	ret
	restore
EPILOGUE(mpn_sub_n)
