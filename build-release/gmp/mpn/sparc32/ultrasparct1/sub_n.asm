dnl  SPARC T1 32-bit mpn_sub_n.

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
define(`rp',  %o0)
define(`ap',  %o1)
define(`bp',  %o2)
define(`n',   %o3)
define(`cy',  %o4)

define(`i',   %o3)

MULFUNC_PROLOGUE(mpn_sub_n mpn_sub_nc)

ASM_START()
PROLOGUE(mpn_sub_nc)
	b	L(ent)
	srl	cy, 0, cy	C strip any bogus high bits
EPILOGUE()

PROLOGUE(mpn_sub_n)
	mov	0, cy
L(ent):	srl	n, 0, n		C strip any bogus high bits
	sll	n, 2, n
	add	ap, n, ap
	add	bp, n, bp
	add	rp, n, rp
	neg	n, i

L(top):	lduw	[ap+i], %g1
	lduw	[bp+i], %g2
	sub	%g1, %g2, %g3
	sub	%g3, cy, %g3
	stw	%g3, [rp+i]
	add	i, 4, i
	brnz	i, L(top)
	srlx	%g3, 63, cy

	retl
	mov	cy, %o0		C return value
EPILOGUE()
