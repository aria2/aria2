dnl PowerPC-64 umul_ppmm -- support for longlong.h

dnl Copyright 2000, 2001, 2005 Free Software Foundation, Inc.
dnl
dnl This file is part of the GNU MP Library.
dnl
dnl The GNU MP Library is free software; you can redistribute it and/or
dnl modify it under the terms of the GNU Lesser General Public License as
dnl published by the Free Software Foundation; either version 3 of the
dnl License, or (at your option) any later version.
dnl
dnl The GNU MP Library is distributed in the hope that it will be useful,
dnl but WITHOUT ANY WARRANTY; without even the implied warranty of
dnl MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Lesser
dnl General Public License for more details.
dnl
dnl You should have received a copy of the GNU Lesser General Public License
dnl along with the GNU MP Library.  If not, see http://www.gnu.org/licenses/.

include(`../config.m4')


C mp_limb_t mpn_umul_ppmm (mp_limb_t *lowptr, mp_limb_t m1, mp_limb_t m2);
C

ASM_START()
PROLOGUE(mpn_umul_ppmm)

	C r3	lowptr
	C r4	m1
	C r5	m2

	mulld	r0, r4, r5
	mulhdu	r4, r4, r5
	std	r0, 0(r3)
ifdef(`HAVE_ABI_mode32',
`	srdi	r3, r4, 32
',`	mr	r3, r4
')
	blr

EPILOGUE(mpn_umul_ppmm)
