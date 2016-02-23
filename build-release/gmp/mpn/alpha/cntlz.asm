dnl  Alpha auxiliary for longlong.h's count_leading_zeros

dnl  Copyright 1997, 2000, 2002 Free Software Foundation, Inc.

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


ASM_START()
EXTERN(__clz_tab)
PROLOGUE(mpn_count_leading_zeros,gp)
	cmpbge	r31,  r16, r1
	LEA(r3,__clz_tab)
	sra	r1,   1,   r1
	xor	r1,   127, r1
	srl	r16,  1,   r16
	addq	r1,   r3,  r1
	ldq_u	r0,   0(r1)
	lda	r2,   64
	extbl	r0,   r1,   r0
	s8subl	r0,   8,    r0
	srl	r16,  r0,   r16
	addq	r16,  r3,   r16
	ldq_u	r1,   0(r16)
	extbl	r1,   r16,  r1
	subq	r2,   r1,   r2
	subq	r2,   r0,   r0
	ret	r31,  (r26),1
EPILOGUE(mpn_count_leading_zeros)
ASM_END()
