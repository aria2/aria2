dnl  HP-PA  mpn_rshift -- Shift a number right.

dnl  Copyright 1992, 1994, 2000, 2001, 2002 Free Software Foundation, Inc.

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
C res_ptr	gr26
C s_ptr		gr25
C size		gr24
C cnt		gr23

ASM_START()
PROLOGUE(mpn_rshift)
	ldws,ma		4(0,%r25),%r22
	mtsar		%r23
	addib,=		-1,%r24,L(0004)
	vshd		%r22,%r0,%r28		C compute carry out limb
	ldws,ma		4(0,%r25),%r29
	addib,=		-1,%r24,L(0002)
	vshd		%r29,%r22,%r20

LDEF(loop)
	ldws,ma		4(0,%r25),%r22
	stws,ma		%r20,4(0,%r26)
	addib,=		-1,%r24,L(0003)
	vshd		%r22,%r29,%r20
	ldws,ma		4(0,%r25),%r29
	stws,ma		%r20,4(0,%r26)
	addib,<>	-1,%r24,L(loop)
	vshd		%r29,%r22,%r20

LDEF(0002)
	stws,ma		%r20,4(0,%r26)
	vshd		%r0,%r29,%r20
	bv		0(%r2)
	stw		%r20,0(0,%r26)

LDEF(0003)
	stws,ma		%r20,4(0,%r26)

LDEF(0004)
	vshd		%r0,%r22,%r20
	bv		0(%r2)
	stw		%r20,0(0,%r26)
EPILOGUE()
