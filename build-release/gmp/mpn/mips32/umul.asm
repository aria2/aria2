dnl  MIPS32 umul_ppmm -- longlong.h support.

dnl  Copyright 1999, 2002 Free Software Foundation, Inc.

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
C plp   $4
C u     $5
C v     $6

ASM_START()
PROLOGUE(mpn_umul_ppmm)
	multu	$5,$6
	mflo	$3
	mfhi	$2
	j	$31
	sw	$3,0($4)
EPILOGUE(mpn_umul_ppmm)
