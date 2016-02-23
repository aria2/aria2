dnl  SPARC mpn_umul_ppmm -- support for longlong.h for non-gcc.

dnl  Copyright 1995, 1996, 2000 Free Software Foundation, Inc.

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
PROLOGUE(mpn_umul_ppmm)
	wr	%g0,%o1,%y
	sra	%o2,31,%g2	C Don't move this insn
	and	%o1,%g2,%g2	C Don't move this insn
	andcc	%g0,0,%g1	C Don't move this insn
	mulscc	%g1,%o2,%g1
	mulscc	%g1,%o2,%g1
	mulscc	%g1,%o2,%g1
	mulscc	%g1,%o2,%g1
	mulscc	%g1,%o2,%g1
	mulscc	%g1,%o2,%g1
	mulscc	%g1,%o2,%g1
	mulscc	%g1,%o2,%g1
	mulscc	%g1,%o2,%g1
	mulscc	%g1,%o2,%g1
	mulscc	%g1,%o2,%g1
	mulscc	%g1,%o2,%g1
	mulscc	%g1,%o2,%g1
	mulscc	%g1,%o2,%g1
	mulscc	%g1,%o2,%g1
	mulscc	%g1,%o2,%g1
	mulscc	%g1,%o2,%g1
	mulscc	%g1,%o2,%g1
	mulscc	%g1,%o2,%g1
	mulscc	%g1,%o2,%g1
	mulscc	%g1,%o2,%g1
	mulscc	%g1,%o2,%g1
	mulscc	%g1,%o2,%g1
	mulscc	%g1,%o2,%g1
	mulscc	%g1,%o2,%g1
	mulscc	%g1,%o2,%g1
	mulscc	%g1,%o2,%g1
	mulscc	%g1,%o2,%g1
	mulscc	%g1,%o2,%g1
	mulscc	%g1,%o2,%g1
	mulscc	%g1,%o2,%g1
	mulscc	%g1,%o2,%g1
	mulscc	%g1,0,%g1
	rd	%y,%g3
	st	%g3,[%o0]
	retl
	add	%g1,%g2,%o0
EPILOGUE(mpn_umul_ppmm)
