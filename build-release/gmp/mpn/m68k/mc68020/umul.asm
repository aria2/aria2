dnl  mc68020 mpn_umul_ppmm -- limb by limb multiplication

dnl  Copyright 1999, 2000, 2001 Free Software Foundation, Inc.
dnl
dnl  This file is part of the GNU MP Library.
dnl
dnl  The GNU MP Library is free software; you can redistribute it and/or
dnl  modify it under the terms of the GNU Lesser General Public License as
dnl  published by the Free Software Foundation; either version 3 of the
dnl  License, or (at your option) any later version.
dnl
dnl  The GNU MP Library is distributed in the hope that it will be useful,
dnl  but WITHOUT ANY WARRANTY; without even the implied warranty of
dnl  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
dnl  Lesser General Public License for more details.
dnl
dnl  You should have received a copy of the GNU Lesser General Public License
dnl  along with the GNU MP Library.  If not, see http://www.gnu.org/licenses/.

include(`../config.m4')


C mp_limb_t mpn_umul_ppmm (mp_limb_t *lp, mp_limb_t x, mp_limb_t y);
C

PROLOGUE(mpn_umul_ppmm)
	movel	M(sp,4), a0	C lp
	movel	M(sp,8), d1	C x
	movel	M(sp,12), d0	C y
	mulul	d0, d0:d1
	movel	d1, M(a0)	C low
	rts
EPILOGUE(mpn_umul_ppmm)
