dnl  X86-64 mpn_add_n, mpn_sub_n, optimized for Intel Sandy Bridge.

dnl  Copyright 2003, 2004, 2005, 2007, 2008, 2010, 2011, 2012 Free Software
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
C AMD K8,K9	 1.85
C AMD K10	 ?
C Intel P4	 ?
C Intel core2	 5
C Intel NHM	 5.5
C Intel SBR	 1.61
C Intel atom	 3
C VIA nano	 3

C INPUT PARAMETERS
define(`rp',	`%rdi')
define(`up',	`%rsi')
define(`vp',	`%rdx')
define(`n',	`%rcx')
define(`cy',	`%r8')		C (only for mpn_add_nc and mpn_sub_nc)

ifdef(`OPERATION_add_n', `
	define(ADCSBB,	      adc)
	define(func,	      mpn_add_n)
	define(func_nc,	      mpn_add_nc)')
ifdef(`OPERATION_sub_n', `
	define(ADCSBB,	      sbb)
	define(func,	      mpn_sub_n)
	define(func_nc,	      mpn_sub_nc)')

MULFUNC_PROLOGUE(mpn_add_n mpn_add_nc mpn_sub_n mpn_sub_nc)

ABI_SUPPORT(DOS64)
ABI_SUPPORT(STD64)

ASM_START()
	TEXT
	ALIGN(16)
PROLOGUE(func)
	FUNC_ENTRY(4)
	xor	%r8, %r8
L(ent):	mov	R32(n), R32(%rax)
	shr	$2, n
	and	$3, R32(%rax)
	jz	L(b0)
	cmp	$2, R32(%rax)
	jz	L(b2)
	jg	L(b3)

L(b1):	mov	(up), %r10
	test	n, n
	jnz	L(gt1)
	neg	R32(%r8)		C set CF from argument
	ADCSBB	(vp), %r10
	mov	%r10, (rp)
	mov	R32(n), R32(%rax)	C zero rax
	adc	R32(%rax), R32(%rax)
	FUNC_EXIT()
	ret
L(gt1):	neg	R32(%r8)
	ADCSBB	(vp), %r10
	mov	8(up), %r11
	lea	16(up), up
	lea	-16(vp), vp
	lea	-16(rp), rp
	jmp	L(m1)

L(b3):	mov	(up), %rax
	mov	8(up), %r9
	mov	16(up), %r10
	test	n, n
	jnz	L(gt3)
	neg	R32(%r8)
	lea	-32(rp), rp
	jmp	L(e3)
L(gt3):	neg	R32(%r8)
	ADCSBB	(vp), %rax
	jmp	L(m3)

	nop				C alignment
	nop				C alignment
L(b0):	mov	(up), %r11
	neg	R32(%r8)
	lea	-24(vp), vp
	lea	-24(rp), rp
	lea	8(up), up
	jmp	L(m0)

L(b2):	mov	(up), %r9
	mov	8(up), %r10
	lea	-8(vp), vp
	test	n, n
	jnz	L(gt2)
	neg	R32(%r8)
	lea	-40(rp), rp
	jmp	L(e2)
L(gt2):	neg	R32(%r8)
	lea	-8(up), up
	lea	-8(rp), rp
	jmp	L(m2)

	ALIGN(8)
L(top):	mov	%r11, 24(rp)
	ADCSBB	(vp), %rax
	lea	32(rp), rp
L(m3):	mov	%rax, (rp)
L(m2):	ADCSBB	8(vp), %r9
	mov	24(up), %r11
	mov	%r9, 8(rp)
	ADCSBB	16(vp), %r10
	lea	32(up), up
L(m1):	mov	%r10, 16(rp)
L(m0):	ADCSBB	24(vp), %r11
	mov	(up), %rax
	mov	8(up), %r9
	lea	32(vp), vp
	dec	n
	mov	16(up), %r10
	jnz	L(top)

	mov	%r11, 24(rp)
L(e3):	ADCSBB	(vp), %rax
	mov	%rax, 32(rp)
L(e2):	ADCSBB	8(vp), %r9
	mov	%r9, 40(rp)
L(e1):	ADCSBB	16(vp), %r10
	mov	%r10, 48(rp)
	mov	R32(n), R32(%rax)	C zero rax
	adc	R32(%rax), R32(%rax)
	FUNC_EXIT()
	ret
EPILOGUE()
PROLOGUE(func_nc)
	FUNC_ENTRY(4)
IFDOS(`	mov	56(%rsp), %r8	')
	jmp	L(ent)
EPILOGUE()
