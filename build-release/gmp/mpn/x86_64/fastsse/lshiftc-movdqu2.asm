dnl  AMD64 mpn_lshiftc optimised for CPUs with fast SSE including fast movdqu.

dnl  Contributed to the GNU project by Torbjorn Granlund.

dnl  Copyright 2010, 2011, 2012 Free Software Foundation, Inc.

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


C	     cycles/limb     cycles/limb     cycles/limb    good
C              aligned	      unaligned	      best seen	   for cpu?
C AMD K8,K9	 3		 3		 ?	  no, use shl/shr
C AMD K10	 1.8-2.0	 1.8-2.0	 ?	  yes
C AMD bd1	 1.9		 1.9		 ?	  yes
C AMD bobcat	 3.67		 3.67			  yes, bad for n < 20
C Intel P4	 4.75		 4.75		 ?	  no, slow movdqu
C Intel core2	 2.27		 2.27		 ?	  no, use shld/shrd
C Intel NHM	 2.15		 2.15		 ?	  no, use shld/shrd
C Intel SBR	 1.45		 1.45		 ?	  yes, bad for n = 4-6
C Intel atom	12.9		12.9		 ?	  no
C VIA nano	 6.18		 6.44		 ?	  no, slow movdqu

C We try to do as many aligned 16-byte operations as possible.  The top-most
C and bottom-most writes might need 8-byte operations.
C
C This variant rely on fast load movdqu, and uses it even for aligned operands,
C in order to avoid the need for two separate loops.
C
C TODO
C  * Could 2-limb wind-down code be simplified?
C  * Improve basecase code, using shld/shrd for SBR, discrete integer shifts
C    for other affected CPUs.

C INPUT PARAMETERS
define(`rp',  `%rdi')
define(`ap',  `%rsi')
define(`n',   `%rdx')
define(`cnt', `%rcx')

ASM_START()
	TEXT
	ALIGN(64)
PROLOGUE(mpn_lshiftc)
	FUNC_ENTRY(4)
	movd	R32(%rcx), %xmm4
	mov	$64, R32(%rax)
	sub	R32(%rcx), R32(%rax)
	movd	R32(%rax), %xmm5

	neg	R32(%rcx)
	mov	-8(ap,n,8), %rax
	shr	R8(%rcx), %rax

	pcmpeqb	%xmm3, %xmm3		C set to 111...111

	cmp	$3, n
	jle	L(bc)

	lea	(rp,n,8), R32(%rcx)
	bt	$3, R32(%rcx)
	jnc	L(rp_aligned)

C Do one initial limb in order to make rp aligned
	movq	-8(ap,n,8), %xmm0
	movq	-16(ap,n,8), %xmm1
	psllq	%xmm4, %xmm0
	psrlq	%xmm5, %xmm1
	por	%xmm1, %xmm0
	pxor	%xmm3, %xmm0
	movq	%xmm0, -8(rp,n,8)
	dec	n

L(rp_aligned):
	lea	1(n), %r8d

	and	$6, R32(%r8)
	jz	L(ba0)
	cmp	$4, R32(%r8)
	jz	L(ba4)
	jc	L(ba2)
L(ba6):	add	$-4, n
	jmp	L(i56)
L(ba0):	add	$-6, n
	jmp	L(i70)
L(ba4):	add	$-2, n
	jmp	L(i34)
L(ba2):	add	$-8, n
	jle	L(end)

	ALIGN(16)
L(top):	movdqu	40(ap,n,8), %xmm1
	movdqu	48(ap,n,8), %xmm0
	psllq	%xmm4, %xmm0
	psrlq	%xmm5, %xmm1
	por	%xmm1, %xmm0
	pxor	%xmm3, %xmm0
	movdqa	%xmm0, 48(rp,n,8)
L(i70):
	movdqu	24(ap,n,8), %xmm1
	movdqu	32(ap,n,8), %xmm0
	psllq	%xmm4, %xmm0
	psrlq	%xmm5, %xmm1
	por	%xmm1, %xmm0
	pxor	%xmm3, %xmm0
	movdqa	%xmm0, 32(rp,n,8)
L(i56):
	movdqu	8(ap,n,8), %xmm1
	movdqu	16(ap,n,8), %xmm0
	psllq	%xmm4, %xmm0
	psrlq	%xmm5, %xmm1
	por	%xmm1, %xmm0
	pxor	%xmm3, %xmm0
	movdqa	%xmm0, 16(rp,n,8)
L(i34):
	movdqu	-8(ap,n,8), %xmm1
	movdqu	(ap,n,8), %xmm0
	psllq	%xmm4, %xmm0
	psrlq	%xmm5, %xmm1
	por	%xmm1, %xmm0
	pxor	%xmm3, %xmm0
	movdqa	%xmm0, (rp,n,8)
	sub	$8, n
	jg	L(top)

L(end):	bt	$0, R32(n)
	jc	L(end8)

	movdqu	(ap), %xmm1
	pxor	%xmm0, %xmm0
	punpcklqdq  %xmm1, %xmm0
	psllq	%xmm4, %xmm1
	psrlq	%xmm5, %xmm0
	por	%xmm1, %xmm0
	pxor	%xmm3, %xmm0
	movdqa	%xmm0, (rp)
	FUNC_EXIT()
	ret

C Basecase
	ALIGN(16)
L(bc):	dec	R32(n)
	jz	L(end8)

	movq	(ap,n,8), %xmm1
	movq	-8(ap,n,8), %xmm0
	psllq	%xmm4, %xmm1
	psrlq	%xmm5, %xmm0
	por	%xmm1, %xmm0
	pxor	%xmm3, %xmm0
	movq	%xmm0, (rp,n,8)
	sub	$2, R32(n)
	jl	L(end8)
	movq	8(ap), %xmm1
	movq	(ap), %xmm0
	psllq	%xmm4, %xmm1
	psrlq	%xmm5, %xmm0
	por	%xmm1, %xmm0
	pxor	%xmm3, %xmm0
	movq	%xmm0, 8(rp)

L(end8):movq	(ap), %xmm0
	psllq	%xmm4, %xmm0
	pxor	%xmm3, %xmm0
	movq	%xmm0, (rp)
	FUNC_EXIT()
	ret
EPILOGUE()
