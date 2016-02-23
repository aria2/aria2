dnl  mc68020 mpn_add_n, mpn_sub_n -- add or subtract limb vectors

dnl  Copyright 1992, 1994, 1996, 1999, 2000, 2001, 2002, 2003, 2005 Free
dnl  Software Foundation, Inc.
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

C         cycles/limb
C 68040:      6

ifdef(`OPERATION_add_n',`
  define(M4_inst,       addxl)
  define(M4_function_n, mpn_add_n)
',`ifdef(`OPERATION_sub_n',`
  define(M4_inst,       subxl)
  define(M4_function_n, mpn_sub_n)
',
`m4_error(`Need OPERATION_add_n or OPERATION_sub_n
')')')

MULFUNC_PROLOGUE(mpn_add_n mpn_sub_n)


C INPUT PARAMETERS
C res_ptr	(sp + 4)
C s1_ptr	(sp + 8)
C s2_ptr	(sp + 12)
C size		(sp + 16)


PROLOGUE(M4_function_n)

C Save used registers on the stack.
	movel	d2, M(-,sp)
	movel	a2, M(-,sp)

C Copy the arguments to registers.  Better use movem?
	movel	M(sp,12), a2
	movel	M(sp,16), a0
	movel	M(sp,20), a1
	movel	M(sp,24), d2

	eorw	#1, d2
	lsrl	#1, d2
	bcc	L(L1)
	subql	#1, d2	C clears cy as side effect

L(Loop):
	movel	M(a0,+), d0
	movel	M(a1,+), d1
	M4_inst	d1, d0
	movel	d0, M(a2,+)
L(L1):	movel	M(a0,+), d0
	movel	M(a1,+), d1
	M4_inst	d1, d0
	movel	d0, M(a2,+)

	dbf	d2, L(Loop)		C loop until 16 lsb of %4 == -1
	subxl	d0, d0			C d0 <= -cy; save cy as 0 or -1 in d0
	subl	#0x10000, d2
	bcs	L(L2)
	addl	d0, d0			C restore cy
	bra	L(Loop)

L(L2):
	negl	d0

C Restore used registers from stack frame.
	movel	M(sp,+), a2
	movel	M(sp,+), d2

	rts

EPILOGUE(M4_function_n)
