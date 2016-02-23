dnl  Intel Pentium 4 mpn_popcount, mpn_hamdist -- population count and
dnl  hamming distance.

dnl  Copyright 2000, 2001, 2002, 2007 Free Software Foundation, Inc.
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


C			     popcount	     hamdist
C P3 model 9  (Banias)		?		?
C P3 model 13 (Dothan)		6		6
C P4 model 0  (Willamette)
C P4 model 1  (?)
C P4 model 2  (Northwood)	8		9
C P4 model 3  (Prescott)	8		9
C P4 model 4  (Nocona)

C unsigned long mpn_popcount (mp_srcptr src, mp_size_t size);
C unsigned long mpn_hamdist (mp_srcptr src, mp_srcptr src2, mp_size_t size);
C
C Loading with unaligned movq's costs an extra 1 c/l and hence is avoided.
C Two movd's and a punpckldq seems to be the same speed as an aligned movq,
C and using them saves fiddling about with alignment testing on entry.
C
C For popcount there's 13 mmx instructions in the loop, so perhaps 6.5 c/l
C might be possible, but 8 c/l relying on out-of-order execution is already
C quite reasonable.

ifdef(`OPERATION_popcount',,
`ifdef(`OPERATION_hamdist',,
`m4_error(`Need OPERATION_popcount or OPERATION_hamdist defined
')')')

define(HAM,
m4_assert_numargs(1)
`ifdef(`OPERATION_hamdist',`$1')')

define(POP,
m4_assert_numargs(1)
`ifdef(`OPERATION_popcount',`$1')')

HAM(`
defframe(PARAM_SIZE, 12)
defframe(PARAM_SRC2,  8)
defframe(PARAM_SRC,   4)
define(M4_function,mpn_hamdist)
')
POP(`
defframe(PARAM_SIZE,  8)
defframe(PARAM_SRC,   4)
define(M4_function,mpn_popcount)
')

MULFUNC_PROLOGUE(mpn_popcount mpn_hamdist)


ifdef(`PIC',,`
	dnl  non-PIC
	RODATA
	ALIGN(8)
L(rodata_AAAAAAAAAAAAAAAA):
	.long	0xAAAAAAAA
	.long	0xAAAAAAAA
L(rodata_3333333333333333):
	.long	0x33333333
	.long	0x33333333
L(rodata_0F0F0F0F0F0F0F0F):
	.long	0x0F0F0F0F
	.long	0x0F0F0F0F
')

	TEXT
	ALIGN(16)

PROLOGUE(M4_function)
deflit(`FRAME',0)

	movl	PARAM_SIZE, %ecx
	movl	PARAM_SRC, %eax

ifdef(`PIC',`
	movl	$0xAAAAAAAA, %edx
	movd	%edx, %mm7
	punpckldq %mm7, %mm7

	movl	$0x33333333, %edx
	movd	%edx, %mm6
	punpckldq %mm6, %mm6

	movl	$0x0F0F0F0F, %edx
	movd	%edx, %mm5
	punpckldq %mm5, %mm5

HAM(`	movl	PARAM_SRC2, %edx')

',`
	dnl non-PIC
HAM(`	movl	PARAM_SRC2, %edx')
	movq	L(rodata_AAAAAAAAAAAAAAAA), %mm7
	movq	L(rodata_3333333333333333), %mm6
	movq	L(rodata_0F0F0F0F0F0F0F0F), %mm5
')

	pxor	%mm4, %mm4		C zero
	pxor	%mm0, %mm0		C total

	subl	$1, %ecx
	ja	L(top)

L(last):
	movd	(%eax,%ecx,4), %mm1		C src high limb
HAM(`	movd	(%edx,%ecx,4), %mm2
	pxor	%mm2, %mm1
')
	jmp	L(loaded)


L(top):
	C eax	src
	C ebx
	C ecx	counter, size-1 to 2 or 1, inclusive
	C edx	[hamdist] src2
	C
	C mm0	total (low dword)
	C mm1	(scratch)
	C mm2	(scratch)
	C mm3
	C mm4	0x0000000000000000
	C mm5	0x0F0F0F0F0F0F0F0F
	C mm6	0x3333333333333333
	C mm7	0xAAAAAAAAAAAAAAAA

	movd	(%eax), %mm1
	movd	4(%eax), %mm2
	punpckldq %mm2, %mm1
	addl	$8, %eax

HAM(`	movd	(%edx), %mm2
	movd	4(%edx), %mm3
	punpckldq %mm3, %mm2
	pxor	%mm2, %mm1
	addl	$8, %edx
')

L(loaded):
	movq	%mm7, %mm2
	pand	%mm1, %mm2
	psrlq	$1, %mm2
	psubd	%mm2, %mm1	C bit pairs

	movq	%mm6, %mm2
	pand	%mm1, %mm2
	psrlq	$2, %mm1
	pand	%mm6, %mm1
	paddd	%mm2, %mm1	C nibbles

	movq	%mm5, %mm2
	pand	%mm1, %mm2
	psrlq	$4, %mm1
	pand	%mm5, %mm1
	paddd	%mm2, %mm1	C bytes

	psadbw(	%mm4, %mm1)
	paddd	%mm1, %mm0	C to total

	subl	$2, %ecx
	jg	L(top)

	C ecx is 0 or -1 representing respectively 1 or 0 further limbs
	jz	L(last)


	movd	%mm0, %eax
	emms
	ret

EPILOGUE()
