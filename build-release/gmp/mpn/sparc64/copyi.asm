dnl  SPARC v9 mpn_copyi -- Copy a limb vector, incrementing.

dnl  Copyright 1999, 2000, 2001, 2002, 2003 Free Software Foundation, Inc.

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
C UltraSPARC 3:	      2.5

C INPUT PARAMETERS
C rptr	%o0
C sptr	%o1
C n	%o2

ASM_START()
	REGISTER(%g2,#scratch)
	REGISTER(%g3,#scratch)
PROLOGUE(mpn_copyi)
	addcc	%o2,-8,%o2
	bl,pt	%xcc,L(end01234567)
	nop
L(loop1):
	ldx	[%o1+0],%g1
	ldx	[%o1+8],%g2
	ldx	[%o1+16],%g3
	ldx	[%o1+24],%g4
	ldx	[%o1+32],%g5
	ldx	[%o1+40],%o3
	ldx	[%o1+48],%o4
	ldx	[%o1+56],%o5
	add	%o1,64,%o1
	stx	%g1,[%o0+0]
	stx	%g2,[%o0+8]
	stx	%g3,[%o0+16]
	stx	%g4,[%o0+24]
	stx	%g5,[%o0+32]
	stx	%o3,[%o0+40]
	stx	%o4,[%o0+48]
	stx	%o5,[%o0+56]
	addcc	%o2,-8,%o2
	bge,pt	%xcc,L(loop1)
	add	%o0,64,%o0
L(end01234567):
	addcc	%o2,8,%o2
	bz,pn	%xcc,L(end)
	nop
L(loop2):
	ldx	[%o1+0],%g1
	add	%o1,8,%o1
	addcc	%o2,-1,%o2
	stx	%g1,[%o0+0]
	bg,pt	%xcc,L(loop2)
	add	%o0,8,%o0
L(end):	retl
	nop
EPILOGUE(mpn_copyi)
