dnl  X86-64 mpn_addmul_1 and mpn_submul_1 optimised for Intel Sandy Bridge.

dnl  Copyright 2003, 2004, 2005, 2007, 2008, 2011, 2012 Free Software
dnl  Foundation, Inc.

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

C	     cycles/limb
C AMD K8,K9	 4.77
C AMD K10	 4.77
C AMD bd1	 ?
C AMD bobcat	 5.78
C Intel P4	15-17
C Intel core2	 5.4
C Intel NHM	 5.23
C Intel SBR	 3.25
C Intel atom	 ?
C VIA nano	 5.5

C The loop of this code is the result of running a code generation and
C optimisation tool suite written by David Harvey and Torbjorn Granlund.

C TODO
C  * The loop is great, but the prologue code was quickly written.  Tune it!

define(`rp',      `%rdi')   C rcx
define(`up',      `%rsi')   C rdx
define(`n_param', `%rdx')   C r8
define(`v0',      `%rcx')   C r9

define(`n',	  `%rbx')

ifdef(`OPERATION_addmul_1',`
      define(`ADDSUB',        `add')
      define(`func',  `mpn_addmul_1')
')
ifdef(`OPERATION_submul_1',`
      define(`ADDSUB',        `sub')
      define(`func',  `mpn_submul_1')
')

dnl Disable until tested ABI_SUPPORT(DOS64)
ABI_SUPPORT(STD64)

MULFUNC_PROLOGUE(mpn_addmul_1 mpn_submul_1)

IFDOS(`	define(`up', ``%rsi'')	') dnl
IFDOS(`	define(`rp', ``%rcx'')	') dnl
IFDOS(`	define(`v0', ``%r9'')	') dnl
IFDOS(`	define(`r9', ``rdi'')	') dnl
IFDOS(`	define(`n',  ``%r8'')	') dnl
IFDOS(`	define(`r8', ``r11'')	') dnl

ASM_START()
	TEXT
	ALIGN(16)
PROLOGUE(func)

IFDOS(``push	%rsi		'')
IFDOS(``push	%rdi		'')
IFDOS(``mov	%rdx, %rsi	'')

	mov	(up), %rax
	push	%rbx
IFSTD(`	mov	R32(n_param), R32(%rdx)	')
IFDOS(`	mov	n, %rdx			')
IFSTD(`	mov	R32(n_param), R32(n)	')

	lea	-8(up,n,8), up
	and	$3, R32(%rdx)
	jz	L(b0)
	cmp	$2, R32(%rdx)
	jz	L(b2)
	jnc	L(b3)

L(b1):	mov	(rp), %r8
	lea	-8(rp,n,8), rp
	neg	n
	mov	$0, R32(%r11)
	add	$4, n
	jc	L(end)
	jmp	L(top)

L(b2):	mov	(rp), %r10
	lea	-8(rp,n,8), rp
	neg	n
	add	$1, n
	mul	v0
	ADDSUB	%rax, %r10
	mov	8(up,n,8), %rax
	mov	%rdx, %r11
	mov	$0, R32(%r9)
	jmp	L(L2)

L(b3):	mov	(rp), %r8
	lea	-8(rp,n,8), rp
	neg	n
	add	$2, n
	mul	v0
	mov	%rdx, %r9
	mov	$0, R32(%r11)
	jmp	L(L3)

L(b0):	mov	(rp), %r10
	lea	-8(rp,n,8), rp
	neg	n
	add	$3, n
	mul	v0
	ADDSUB	%rax, %r10
	mov	%rdx, %r11
	mov	-8(up,n,8), %rax
	adc	$0, %r11
	mov	$0, R32(%r9)
	jmp	L(L0)

	ALIGN(16)
L(top):	mul	v0
	ADDSUB	%rax, %r8
	mov	%rdx, %r9
	adc	$0, %r9
	mov	-16(up,n,8), %rax
	ADDSUB	%r11, %r8
	mov	-16(rp,n,8), %r10
	adc	$0, %r9
	mul	v0
	ADDSUB	%rax, %r10
	mov	%rdx, %r11
	mov	-8(up,n,8), %rax
	adc	$0, %r11
	mov	%r8, -24(rp,n,8)
L(L0):	mul	v0
	ADDSUB	%r9, %r10
	mov	-8(rp,n,8), %r8
	adc	$0, %r11
	mov	%rdx, %r9
	mov	%r10, -16(rp,n,8)
L(L3):	ADDSUB	%rax, %r8
	adc	$0, %r9
	mov	(up,n,8), %rax
	ADDSUB	%r11, %r8
	adc	$0, %r9
	mov	(rp,n,8), %r10
	mul	v0
	ADDSUB	%rax, %r10
	mov	8(up,n,8), %rax
	mov	%rdx, %r11
	mov	%r8, -8(rp,n,8)
L(L2):	adc	$0, %r11
	mov	8(rp,n,8), %r8
	ADDSUB	%r9, %r10
	adc	$0, %r11
	mov	%r10, (rp,n,8)
	add	$4, n
	jnc	L(top)

L(end):	mul	v0
	ADDSUB	%rax, %r8
	mov	%rdx, %rax
	adc	$0, %rax
	ADDSUB	%r11, %r8
	adc	$0, %rax
	mov	%r8, (rp)

	pop	%rbx
IFDOS(``pop	%rdi		'')
IFDOS(``pop	%rsi		'')
	ret
EPILOGUE()
