dnl  mpn_umul_ppmm -- 1x1->2 limb multiplication

dnl  Copyright 1999, 2000, 2002 Free Software Foundation, Inc.

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


C mp_limb_t mpn_umul_ppmm (mp_limb_t *lowptr, mp_limb_t m1, mp_limb_t m2);
C

ASM_START()
PROLOGUE(mpn_umul_ppmm)
	mulq	r17, r18, r1
	umulh	r17, r18, r0
	stq	r1, 0(r16)
	ret	r31, (r26), 1
EPILOGUE()
ASM_END()
