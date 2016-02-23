dnl  Intel P5 mpn_popcount -- mpn bit population count.

dnl  Copyright 2001, 2002 Free Software Foundation, Inc.
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


C P5: 8.0 cycles/limb


C unsigned long mpn_popcount (mp_srcptr src, mp_size_t size);
C
C An arithmetic approach has been found to be slower than the table lookup,
C due to needing too many instructions.

C The slightly strange quoting here helps the renaming done by tune/many.pl.
deflit(TABLE_NAME,
m4_assert_defined(`GSYM_PREFIX')
GSYM_PREFIX`'mpn_popcount``'_table')

	RODATA
	ALIGN(8)
	GLOBL	TABLE_NAME
TABLE_NAME:
forloop(i,0,255,
`	.byte	m4_popcount(i)
')

defframe(PARAM_SIZE,8)
defframe(PARAM_SRC, 4)

	TEXT
	ALIGN(8)

PROLOGUE(mpn_popcount)
deflit(`FRAME',0)

	movl	PARAM_SIZE, %ecx
	pushl	%esi	FRAME_pushl()

ifdef(`PIC',`
	pushl	%ebx	FRAME_pushl()
	pushl	%ebp	FRAME_pushl()

	call	L(here)
L(here):
	popl	%ebp
	shll	%ecx		C size in byte pairs

	addl	$_GLOBAL_OFFSET_TABLE_+[.-L(here)], %ebp
	movl	PARAM_SRC, %esi

	xorl	%eax, %eax	C total
	xorl	%ebx, %ebx	C byte

	movl	TABLE_NAME@GOT(%ebp), %ebp
	xorl	%edx, %edx	C byte
define(TABLE,`(%ebp,$1)')
',`
dnl non-PIC
	shll	%ecx		C size in byte pairs
	movl	PARAM_SRC, %esi

	pushl	%ebx	FRAME_pushl()
	xorl	%eax, %eax	C total

	xorl	%ebx, %ebx	C byte
	xorl	%edx, %edx	C byte

define(TABLE,`TABLE_NAME`'($1)')
')


	ALIGN(8)	C necessary on P55 for claimed speed
L(top):
	C eax	total
	C ebx	byte
	C ecx	counter, 2*size to 2
	C edx	byte
	C esi	src
	C edi
	C ebp	[PIC] table

	addl	%ebx, %eax
	movb	-1(%esi,%ecx,2), %bl

	addl	%edx, %eax
	movb	-2(%esi,%ecx,2), %dl

	movb	TABLE(%ebx), %bl
	decl	%ecx

	movb	TABLE(%edx), %dl
	jnz	L(top)


ifdef(`PIC',`
	popl	%ebp
')
	addl	%ebx, %eax
	popl	%ebx

	addl	%edx, %eax
	popl	%esi

	ret

EPILOGUE()
