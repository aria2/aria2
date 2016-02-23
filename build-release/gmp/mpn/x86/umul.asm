dnl  mpn_umul_ppmm -- 1x1->2 limb multiplication

dnl  Copyright 1999, 2000, 2002 Free Software Foundation, Inc.
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


C mp_limb_t mpn_umul_ppmm (mp_limb_t *lowptr, mp_limb_t m1, mp_limb_t m2);
C

defframe(PARAM_M2,    12)
defframe(PARAM_M1,     8)
defframe(PARAM_LOWPTR, 4)

	TEXT
	ALIGN(8)
PROLOGUE(mpn_umul_ppmm)
deflit(`FRAME',0)
	movl	PARAM_LOWPTR, %ecx
	movl	PARAM_M1, %eax
	mull	PARAM_M2
	movl	%eax, (%ecx)
	movl	%edx, %eax
	ret
EPILOGUE()
