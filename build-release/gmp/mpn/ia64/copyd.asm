dnl  IA-64 mpn_copyd -- copy limb vector, decrementing.

dnl  Contributed to the GNU project by Torbjorn Granlund.

dnl  Copyright 2001, 2002, 2004 Free Software Foundation, Inc.

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

C         cycles/limb
C Itanium:    1
C Itanium 2:  0.5

C INPUT PARAMETERS
C rp = r32
C sp = r33
C n = r34

ASM_START()
PROLOGUE(mpn_copyd)
	.prologue
	.save ar.lc, r2
	.body
ifdef(`HAVE_ABI_32',
`	addp4		r32 = 0, r32
	addp4		r33 = 0, r33
	sxt4		r34 = r34
	;;
')
{.mmi
	shladd		r32 = r34, 3, r32
	shladd		r33 = r34, 3, r33
	mov.i		r2 = ar.lc
}
{.mmi
	and		r14 = 3, r34
	cmp.ge		p14, p15 = 3, r34
	add		r34 = -4, r34
	;;
}
{.mmi
	cmp.eq		p8, p0 = 1, r14
	cmp.eq		p10, p0 = 2, r14
	cmp.eq		p12, p0 = 3, r14
}
{.bbb
  (p8)	br.dptk		.Lb01
  (p10)	br.dptk		.Lb10
  (p12)	br.dptk		.Lb11
}

.Lb00:	C  n = 0, 4, 8, 12, ...
	add		r32 = -8, r32
	add		r33 = -8, r33
  (p14)	br.dptk		.Ls00
	;;
	add		r21 = -8, r33
	ld8		r16 = [r33], -16
	shr		r15 = r34, 2
	;;
	ld8		r17 = [r21], -16
	mov.i		ar.lc = r15
	ld8		r18 = [r33], -16
	add		r20 = -8, r32
	;;
	ld8		r19 = [r21], -16
	br.cloop.dptk	.Loop
	;;
	br.sptk		.Lend
	;;

.Lb01:	C  n = 1, 5, 9, 13, ...
	add		r21 = -8, r33
	add		r20 = -8, r32
	add		r33 = -16, r33
	add		r32 = -16, r32
	;;
	ld8		r19 = [r21], -16
	shr		r15 = r34, 2
  (p14)	br.dptk		.Ls01
	;;
	ld8		r16 = [r33], -16
	mov.i		ar.lc = r15
	;;
	ld8		r17 = [r21], -16
	ld8		r18 = [r33], -16
	br.sptk		.Li01
	;;

.Lb10:	C  n = 2,6, 10, 14, ...
	add		r21 = -16, r33
	shr		r15 = r34, 2
	add		r20 = -16, r32
	add		r32 = -8, r32
	add		r33 = -8, r33
	;;
	ld8		r18 = [r33], -16
	ld8		r19 = [r21], -16
	mov.i		ar.lc = r15
  (p14)	br.dptk		.Ls10
	;;
	ld8		r16 = [r33], -16
	ld8		r17 = [r21], -16
	br.sptk		.Li10
	;;

.Lb11:	C  n = 3, 7, 11, 15, ...
	add		r21 = -8, r33
	add		r20 = -8, r32
	add		r33 = -16, r33
	add		r32 = -16, r32
	;;
	ld8		r17 = [r21], -16
	shr		r15 = r34, 2
	;;
	ld8		r18 = [r33], -16
	mov.i		ar.lc = r15
	ld8		r19 = [r21], -16
  (p14)	br.dptk		.Ls11
	;;
	ld8		r16 = [r33], -16
	br.sptk		.Li11
	;;

	ALIGN(32)
.Loop:
.Li00:
{.mmb
	st8		[r32] = r16, -16
	ld8		r16 = [r33], -16
	nop.b		0
}
.Li11:
{.mmb
	st8		[r20] = r17, -16
	ld8		r17 = [r21], -16
	nop.b		0
	;;
}
.Li10:
{.mmb
	st8		[r32] = r18, -16
	ld8		r18 = [r33], -16
	nop.b		0
}
.Li01:
{.mmb
	st8		[r20] = r19, -16
	ld8		r19 = [r21], -16
	br.cloop.dptk	.Loop
	;;
}
.Lend:	st8		[r32] = r16, -16
.Ls11:	st8		[r20] = r17, -16
	;;
.Ls10:	st8		[r32] = r18, -16
.Ls01:	st8		[r20] = r19, -16
.Ls00:	mov.i		ar.lc = r2
	br.ret.sptk.many b0
EPILOGUE()
ASM_END()
