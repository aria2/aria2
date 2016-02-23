dnl  mc68020 mpn_udiv_qrnnd -- 2x1 limb division

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


C mp_limb_t mpn_udiv_qrnnd (mp_limb_t *rp,
C                           mp_limb_t nh, mp_limb_t nl, mp_limb_t d);
C

PROLOGUE(mpn_udiv_qrnnd)
	movel	M(sp,4), a0	C rp
	movel	M(sp,8), d1	C nh
	movel	M(sp,12), d0	C nl
	divul	M(sp,16), d1:d0
	movel	d1, M(a0)	C r
	rts
EPILOGUE(mpn_udiv_qrnnd)
