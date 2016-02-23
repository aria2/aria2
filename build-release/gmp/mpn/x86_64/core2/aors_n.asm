dnl  Intel P6-15 mpn_add_n/mpn_sub_n -- mpn add or subtract.

dnl  Copyright 2006, 2007, 2011, 2012 Free Software Foundation, Inc.

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
C Intel P4	10
C Intel core2	 2.05
C Intel NHM	 2.3
C Intel SBR	 1.9
C Intel atom	 ?
C VIA nano	 ?

C INPUT PARAMETERS
define(`rp',	`%rdi')
define(`up',	`%rsi')
define(`vp',	`%rdx')
define(`n',	`%rcx')
define(`cy',	`%r8')

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
PROLOGUE(func_nc)
	FUNC_ENTRY(4)
IFDOS(`	mov	56(%rsp), %r8	')
	jmp	L(start)
EPILOGUE()

PROLOGUE(func)
	FUNC_ENTRY(4)
	xor	%r8, %r8
L(start):
	mov	(up), %r10
	mov	(vp), %r11

	lea	-8(up,n,8), up
	lea	-8(vp,n,8), vp
	lea	-16(rp,n,8), rp
	mov	R32(%rcx), R32(%rax)
	neg	n
	and	$3, R32(%rax)
	je	L(b00)
	add	%rax, n			C clear low rcx bits for jrcxz
	cmp	$2, R32(%rax)
	jl	L(b01)
	je	L(b10)

L(b11):	shr	%r8			C set cy
	jmp	L(e11)

L(b00):	shr	%r8			C set cy
	mov	%r10, %r8
	mov	%r11, %r9
	lea	4(n), n
	jmp	L(e00)

L(b01):	shr	%r8			C set cy
	jmp	L(e01)

L(b10):	shr	%r8			C set cy
	mov	%r10, %r8
	mov	%r11, %r9
	jmp	L(e10)

L(end):	ADCSBB	%r11, %r10
	mov	%r10, 8(rp)
	mov	R32(%rcx), R32(%rax)	C clear eax, ecx contains 0
	adc	R32(%rax), R32(%rax)
	FUNC_EXIT()
	ret

	ALIGN(16)
L(top):
	mov	-24(up,n,8), %r8
	mov	-24(vp,n,8), %r9
	ADCSBB	%r11, %r10
	mov	%r10, -24(rp,n,8)
L(e00):
	mov	-16(up,n,8), %r10
	mov	-16(vp,n,8), %r11
	ADCSBB	%r9, %r8
	mov	%r8, -16(rp,n,8)
L(e11):
	mov	-8(up,n,8), %r8
	mov	-8(vp,n,8), %r9
	ADCSBB	%r11, %r10
	mov	%r10, -8(rp,n,8)
L(e10):
	mov	(up,n,8), %r10
	mov	(vp,n,8), %r11
	ADCSBB	%r9, %r8
	mov	%r8, (rp,n,8)
L(e01):
	jrcxz	L(end)
	lea	4(n), n
	jmp	L(top)

EPILOGUE()
