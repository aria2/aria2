dnl  X86-64 mpn_addmul_2 optimised for Intel Sandy Bridge.

dnl  Copyright 2008, 2011, 2012 Free Software Foundation, Inc.

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
C AMD K8,K9
C AMD K10	 4.07
C AMD bd1
C AMD bobcat	 5.25
C Intel P4	16.1
C Intel core2
C Intel NHM
C Intel SBR	 3.2
C Intel atom
C VIA nano	 5.23

C This code is the result of running a code generation and optimisation tool
C suite written by David Harvey and Torbjorn Granlund.

C TODO
C  * Tune feed-in and wind-down code.

C INPUT PARAMETERS
define(`rp',     `%rdi')
define(`up',     `%rsi')
define(`n_param',`%rdx')
define(`vp',     `%rcx')

define(`v0', `%r12')
define(`v1', `%r13')
define(`n',  `%r11')

ABI_SUPPORT(DOS64)
ABI_SUPPORT(STD64)

ASM_START()
	TEXT
	ALIGN(16)
PROLOGUE(mpn_addmul_2)
	FUNC_ENTRY(4)
	push	%rbx
	push	%r12
	push	%r13
	push	%r14

	mov	(up), %rax

	mov	n_param, n
	mov	0(vp), v0
	mov	8(vp), v1
	shr	$2, n
	and	$3, R32(n_param)
	jz	L(b0)
	cmp	$2, R32(n_param)
	jb	L(b1)
	jz	L(b2)

L(b3):	mov	(rp), %r10
	mov	$0, R32(%rcx)
	mul	v0
	add	%rax, %r10
	mov	%rdx, %r14
	adc	$0, %r14
	lea	-16(rp), rp
	lea	-16(up), up
	mov	$0, R32(%r9)
	mov	$0, R32(%rbx)
	inc	n
	jmp	L(L3)

L(b0):	mov	(rp), %r8
	mul	v0
	add	%rax, %r8
	mov	%rdx, %r9
	adc	$0, %r9
	mov	$0, R32(%rbx)
	lea	-8(rp), rp
	lea	-8(up), up
	jmp	L(L0)

L(b1):	mov	(rp), %r10
	mov	$0, R32(%rcx)
	mul	v0
	add	%rax, %r10
	mov	%rdx, %r14
	adc	$0, %r14
	mov	%r10, 0(rp)
	jmp	L(L1)

L(b2):	mov	(rp), %r8
	mul	v0
	add	%rax, %r8
	mov	$0, R32(%rbx)
	mov	%rdx, %r9
	adc	$0, %r9
	lea	-24(rp), rp
	lea	-24(up), up
	inc	n
	jmp	L(L2)

	ALIGN(32)
L(top):	mov	%r10, 32(rp)
	adc	%rbx, %r14		C 10
	lea	32(rp), rp
L(L1):	mov	0(up), %rax
	adc	$0, R32(%rcx)
	mul	v1
	mov	$0, R32(%rbx)
	mov	8(rp), %r8
	add	%rax, %r8
	mov	%rdx, %r9
	mov	8(up), %rax
	adc	$0, %r9
	mul	v0
	add	%rax, %r8
	adc	%rdx, %r9
	adc	$0, R32(%rbx)
	add	%r14, %r8		C 0 12
	adc	%rcx, %r9		C 1
L(L0):	mov	8(up), %rax
	adc	$0, R32(%rbx)
	mov	16(rp), %r10
	mul	v1
	add	%rax, %r10
	mov	%rdx, %r14
	mov	16(up), %rax
	mov	$0, R32(%rcx)
	adc	$0, %r14
	mul	v0
	add	%rax, %r10
	adc	%rdx, %r14
	adc	$0, R32(%rcx)
	mov	%r8, 8(rp)
L(L3):	mov	24(rp), %r8
	mov	16(up), %rax
	mul	v1
	add	%r9, %r10		C 3
	adc	%rbx, %r14		C 4
	adc	$0, R32(%rcx)
	add	%rax, %r8
	mov	%rdx, %r9
	adc	$0, %r9
	mov	24(up), %rax
	mul	v0
	add	%rax, %r8
	mov	$0, R32(%rbx)
	adc	%rdx, %r9
	adc	$0, R32(%rbx)
	add	%r14, %r8		C 6
	adc	%rcx, %r9		C 7
	mov	%r10, 16(rp)
L(L2):	mov	24(up), %rax
	adc	$0, R32(%rbx)
	mov	32(rp), %r10
	mul	v1
	add	%rax, %r10
	mov	32(up), %rax
	lea	32(up), up
	mov	%rdx, %r14
	adc	$0, %r14
	mov	%r8, 24(rp)
	mov	$0, R32(%rcx)
	mul	v0
	add	%rax, %r10
	adc	%rdx, %r14
	adc	$0, R32(%rcx)
	add	%r9, %r10		C 9
	dec	n
	jnz	L(top)

	mov	%r10, 32(rp)
	adc	%rbx, %r14
	mov	0(up), %rax
	adc	$0, R32(%rcx)
	mul	v1
	mov	%rax, %r8
	mov	%rdx, %rax
	add	%r14, %r8
	adc	%rcx, %rax
	mov	%r8, 40(rp)

	pop	%r14
	pop	%r13
	pop	%r12
	pop	%rbx
	FUNC_EXIT()
	ret
EPILOGUE()
ASM_END()
