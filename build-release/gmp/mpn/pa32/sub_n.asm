dnl  HP-PA mpn_sub_n -- Subtract two limb vectors of the same length > 0 and
dnl  store difference in a third limb vector.

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
C s1_ptr	gr25
C s2_ptr	gr24
C size		gr23

C One might want to unroll this as for other processors, but it turns out that
C the data cache contention after a store makes such unrolling useless.  We
C can't come under 5 cycles/limb anyway.

ASM_START()
PROLOGUE(mpn_sub_n)
	ldws,ma		4(0,%r25),%r20
	ldws,ma		4(0,%r24),%r19

	addib,=		-1,%r23,L(end)	C check for (SIZE == 1)
	 sub		%r20,%r19,%r28	C subtract first limbs ignoring cy

LDEF(loop)
	ldws,ma		4(0,%r25),%r20
	ldws,ma		4(0,%r24),%r19
	stws,ma		%r28,4(0,%r26)
	addib,<>	-1,%r23,L(loop)
	 subb		%r20,%r19,%r28

LDEF(end)
	stws		%r28,0(0,%r26)
	addc		%r0,%r0,%r28
	bv		0(%r2)
	 subi		1,%r28,%r28
EPILOGUE()
