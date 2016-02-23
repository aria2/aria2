dnl  AMD64 mpn_addcnd_n, mpn_subcnd_n

dnl  Copyright 2011, 2012 Free Software Foundation, Inc.

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
C AMD K8,K9	 2.25
C AMD K10	 2
C AMD bd1	 3.55
C AMD bobcat	 2.5
C Intel P4	13
C Intel core2	 2.9
C Intel NHM	 2.9
C Intel SBR	 2.4
C Intel atom	 6.5
C VIA nano	 3

C NOTES
C  * It might seem natural to use the cmov insn here, but since this function
C    is supposed to have the exact same execution pattern for cnd true and
C    false, and since cmov's documentation is not clear about wheather it
C    actually reads both source operands and writes the register for a false
C    condition, we cannot use it.
C  * Two cases could be optimised: (1) addcnd_n could use ADCSBB-from-memory
C    to save one insn/limb, and (2) when up=rp addcnd_n and subcnd_n could use
C    ADCSBB-to-memory, again saving 1 insn/limb.
C  * This runs optimally at decoder bandwidth on K10.  It has not been tuned
C    for any other processor.

C INPUT PARAMETERS
define(`rp',	`%rdi')
define(`up',	`%rsi')
define(`vp',	`%rdx')
define(`n',	`%rcx')
define(`cnd',	`%r8')

ifdef(`OPERATION_addcnd_n', `
	define(ADDSUB,	      add)
	define(ADCSBB,	      adc)
	define(func,	      mpn_addcnd_n)')
ifdef(`OPERATION_subcnd_n', `
	define(ADDSUB,	      sub)
	define(ADCSBB,	      sbb)
	define(func,	      mpn_subcnd_n)')

MULFUNC_PROLOGUE(mpn_addcnd_n mpn_subcnd_n)

ABI_SUPPORT(DOS64)
ABI_SUPPORT(STD64)

ASM_START()
	TEXT
	ALIGN(16)
PROLOGUE(func)
	FUNC_ENTRY(4)
IFDOS(`	mov	56(%rsp), %r8	')
	push	%rbx
	push	%rbp
	push	%r12
	push	%r13
	push	%r14

	neg	cnd
	sbb	cnd, cnd		C make cnd mask

	lea	(vp,n,8), vp
	lea	(up,n,8), up
	lea	(rp,n,8), rp

	mov	R32(n), R32(%rax)
	neg	n
	and	$3, R32(%rax)
	jz	L(top)			C carry-save reg rax = 0 in this arc
	cmp	$2, R32(%rax)
	jc	L(b1)
	jz	L(b2)

L(b3):	mov	(vp,n,8), %r12
	mov	8(vp,n,8), %r13
	mov	16(vp,n,8), %r14
	mov	(up,n,8), %r10
	mov	8(up,n,8), %rbx
	mov	16(up,n,8), %rbp
	and	cnd, %r12
	and	cnd, %r13
	and	cnd, %r14
	ADDSUB	%r12, %r10
	ADCSBB	%r13, %rbx
	ADCSBB	%r14, %rbp
	sbb	R32(%rax), R32(%rax)	C save carry
	mov	%r10, (rp,n,8)
	mov	%rbx, 8(rp,n,8)
	mov	%rbp, 16(rp,n,8)
	add	$3, n
	js	L(top)
	jmp	L(end)

L(b2):	mov	(vp,n,8), %r12
	mov	8(vp,n,8), %r13
	mov	(up,n,8), %r10
	mov	8(up,n,8), %rbx
	and	cnd, %r12
	and	cnd, %r13
	ADDSUB	%r12, %r10
	ADCSBB	%r13, %rbx
	sbb	R32(%rax), R32(%rax)	C save carry
	mov	%r10, (rp,n,8)
	mov	%rbx, 8(rp,n,8)
	add	$2, n
	js	L(top)
	jmp	L(end)

L(b1):	mov	(vp,n,8), %r12
	mov	(up,n,8), %r10
	and	cnd, %r12
	ADDSUB	%r12, %r10
	sbb	R32(%rax), R32(%rax)	C save carry
	mov	%r10, (rp,n,8)
	add	$1, n
	jns	L(end)

	ALIGN(16)
L(top):	mov	(vp,n,8), %r12
	mov	8(vp,n,8), %r13
	mov	16(vp,n,8), %r14
	mov	24(vp,n,8), %r11
	mov	(up,n,8), %r10
	mov	8(up,n,8), %rbx
	mov	16(up,n,8), %rbp
	mov	24(up,n,8), %r9
	and	cnd, %r12
	and	cnd, %r13
	and	cnd, %r14
	and	cnd, %r11
	add	R32(%rax), R32(%rax)	C restore carry
	ADCSBB	%r12, %r10
	ADCSBB	%r13, %rbx
	ADCSBB	%r14, %rbp
	ADCSBB	%r11, %r9
	sbb	R32(%rax), R32(%rax)	C save carry
	mov	%r10, (rp,n,8)
	mov	%rbx, 8(rp,n,8)
	mov	%rbp, 16(rp,n,8)
	mov	%r9, 24(rp,n,8)
	add	$4, n
	js	L(top)

L(end):	neg	R32(%rax)
	pop	%r14
	pop	%r13
	pop	%r12
	pop	%rbp
	pop	%rbx
	FUNC_EXIT()
	ret
EPILOGUE()
