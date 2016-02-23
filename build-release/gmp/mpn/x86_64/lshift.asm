dnl  AMD64 mpn_lshift -- mpn left shift.

dnl  Copyright 2003, 2005, 2007, 2009, 2011, 2012 Free Software Foundation,
dnl  Inc.
dnl
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


C	     cycles/limb   cycles/limb cnt=1
C AMD K8,K9	 2.375		 1.375
C AMD K10	 2.375		 1.375
C Intel P4	 8		10.5
C Intel core2	 2.11		 4.28
C Intel corei	 ?		 ?
C Intel atom	 5.75		 3.5
C VIA nano	 3.5		 2.25


C INPUT PARAMETERS
define(`rp',	`%rdi')
define(`up',	`%rsi')
define(`n',	`%rdx')
define(`cnt',	`%rcx')

ABI_SUPPORT(DOS64)
ABI_SUPPORT(STD64)

ASM_START()
	TEXT
	ALIGN(32)
PROLOGUE(mpn_lshift)
	FUNC_ENTRY(4)
	cmp	$1, R8(%rcx)
	jne	L(gen)

C For cnt=1 we want to work from lowest limb towards higher limbs.
C Check for bad overlap (up=rp is OK!) up=rp+1..rp+n-1 is bad.
C FIXME: this could surely be done more cleverly.

	mov    rp, %rax
	sub    up, %rax
	je     L(fwd)			C rp = up
	shr    $3, %rax
	cmp    n, %rax
	jb     L(gen)

L(fwd):	mov	R32(n), R32(%rax)
	shr	$2, n
	je	L(e1)
	and	$3, R32(%rax)

	ALIGN(8)
	nop
	nop
L(t1):	mov	(up), %r8
	mov	8(up), %r9
	mov	16(up), %r10
	mov	24(up), %r11
	lea	32(up), up
	adc	%r8, %r8
	mov	%r8, (rp)
	adc	%r9, %r9
	mov	%r9, 8(rp)
	adc	%r10, %r10
	mov	%r10, 16(rp)
	adc	%r11, %r11
	mov	%r11, 24(rp)
	lea	32(rp), rp
	dec	n
	jne	L(t1)

	inc	R32(%rax)
	dec	R32(%rax)
	jne	L(n00)
	adc	R32(%rax), R32(%rax)
	FUNC_EXIT()
	ret
L(e1):	test	R32(%rax), R32(%rax)	C clear cy
L(n00):	mov	(up), %r8
	dec	R32(%rax)
	jne	L(n01)
	adc	%r8, %r8
	mov	%r8, (rp)
L(ret):	adc	R32(%rax), R32(%rax)
	FUNC_EXIT()
	ret
L(n01):	dec	R32(%rax)
	mov	8(up), %r9
	jne	L(n10)
	adc	%r8, %r8
	adc	%r9, %r9
	mov	%r8, (rp)
	mov	%r9, 8(rp)
	adc	R32(%rax), R32(%rax)
	FUNC_EXIT()
	ret
L(n10):	mov	16(up), %r10
	adc	%r8, %r8
	adc	%r9, %r9
	adc	%r10, %r10
	mov	%r8, (rp)
	mov	%r9, 8(rp)
	mov	%r10, 16(rp)
	adc	$-1, R32(%rax)
	FUNC_EXIT()
	ret

L(gen):	neg	R32(%rcx)		C put rsh count in cl
	mov	-8(up,n,8), %rax
	shr	R8(%rcx), %rax		C function return value

	neg	R32(%rcx)		C put lsh count in cl
	lea	1(n), R32(%r8)
	and	$3, R32(%r8)
	je	L(rlx)			C jump for n = 3, 7, 11, ...

	dec	R32(%r8)
	jne	L(1)
C	n = 4, 8, 12, ...
	mov	-8(up,n,8), %r10
	shl	R8(%rcx), %r10
	neg	R32(%rcx)		C put rsh count in cl
	mov	-16(up,n,8), %r8
	shr	R8(%rcx), %r8
	or	%r8, %r10
	mov	%r10, -8(rp,n,8)
	dec	n
	jmp	L(rll)

L(1):	dec	R32(%r8)
	je	L(1x)			C jump for n = 1, 5, 9, 13, ...
C	n = 2, 6, 10, 16, ...
	mov	-8(up,n,8), %r10
	shl	R8(%rcx), %r10
	neg	R32(%rcx)		C put rsh count in cl
	mov	-16(up,n,8), %r8
	shr	R8(%rcx), %r8
	or	%r8, %r10
	mov	%r10, -8(rp,n,8)
	dec	n
	neg	R32(%rcx)		C put lsh count in cl
L(1x):
	cmp	$1, n
	je	L(ast)
	mov	-8(up,n,8), %r10
	shl	R8(%rcx), %r10
	mov	-16(up,n,8), %r11
	shl	R8(%rcx), %r11
	neg	R32(%rcx)		C put rsh count in cl
	mov	-16(up,n,8), %r8
	mov	-24(up,n,8), %r9
	shr	R8(%rcx), %r8
	or	%r8, %r10
	shr	R8(%rcx), %r9
	or	%r9, %r11
	mov	%r10, -8(rp,n,8)
	mov	%r11, -16(rp,n,8)
	sub	$2, n

L(rll):	neg	R32(%rcx)		C put lsh count in cl
L(rlx):	mov	-8(up,n,8), %r10
	shl	R8(%rcx), %r10
	mov	-16(up,n,8), %r11
	shl	R8(%rcx), %r11

	sub	$4, n			C				      4
	jb	L(end)			C				      2
	ALIGN(16)
L(top):
	C finish stuff from lsh block
	neg	R32(%rcx)		C put rsh count in cl
	mov	16(up,n,8), %r8
	mov	8(up,n,8), %r9
	shr	R8(%rcx), %r8
	or	%r8, %r10
	shr	R8(%rcx), %r9
	or	%r9, %r11
	mov	%r10, 24(rp,n,8)
	mov	%r11, 16(rp,n,8)
	C start two new rsh
	mov	0(up,n,8), %r8
	mov	-8(up,n,8), %r9
	shr	R8(%rcx), %r8
	shr	R8(%rcx), %r9

	C finish stuff from rsh block
	neg	R32(%rcx)		C put lsh count in cl
	mov	8(up,n,8), %r10
	mov	0(up,n,8), %r11
	shl	R8(%rcx), %r10
	or	%r10, %r8
	shl	R8(%rcx), %r11
	or	%r11, %r9
	mov	%r8, 8(rp,n,8)
	mov	%r9, 0(rp,n,8)
	C start two new lsh
	mov	-8(up,n,8), %r10
	mov	-16(up,n,8), %r11
	shl	R8(%rcx), %r10
	shl	R8(%rcx), %r11

	sub	$4, n
	jae	L(top)			C				      2
L(end):
	neg	R32(%rcx)		C put rsh count in cl
	mov	8(up), %r8
	shr	R8(%rcx), %r8
	or	%r8, %r10
	mov	(up), %r9
	shr	R8(%rcx), %r9
	or	%r9, %r11
	mov	%r10, 16(rp)
	mov	%r11, 8(rp)

	neg	R32(%rcx)		C put lsh count in cl
L(ast):	mov	(up), %r10
	shl	R8(%rcx), %r10
	mov	%r10, (rp)
	FUNC_EXIT()
	ret
EPILOGUE()
