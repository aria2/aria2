dnl  AMD64 mpn_copyd optimised for CPUs with fast SSE copying and SSSE3.

dnl  Copyright 2012 Free Software Foundation, Inc.

dnl  Contributed to the GNU project by Torbjorn Granlund.

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

C	     cycles/limb     cycles/limb     cycles/limb      good
C              aligned	      unaligned	      best seen	     for cpu?
C AMD K8,K9	 2.0		 illop		1.0/1.0		N
C AMD K10	 0.85		 illop				Y/N
C AMD bd1	 1.39		 1.40				Y
C AMD bobcat	 1.97		 8.35		1.5/1.5		N
C Intel P4	 2.26		 illop				Y/N
C Intel core2	 0.52		0.68-0.80	opt/0.68	Y
C Intel NHM	 0.52		 0.64		opt/opt		Y
C Intel SBR	 0.51		 0.54		opt/0.51	Y
C Intel atom	 1.16		 1.66		opt/opt		Y
C VIA nano	 1.09		 1.07		opt/opt		Y

C We use only 16-byte operations, except for unaligned top-most and bottom-most
C limbs.  We use the SSSE3 palignr instruction when rp - up = 8 (mod 16).
C
C For operands of < COPYD_SSE_THRESHOLD limbs, we use a plain 64-bit loop,
C taken from the x86_64 default code.

C INPUT PARAMETERS
define(`rp', `%rdi')
define(`up', `%rsi')
define(`n',  `%rdx')

C There are three instructions for loading an aligned 128-bit quantity.  We use
C movaps, since it has the shortest coding.
define(`movdqa', ``movaps'')

ifdef(`COPYD_SSE_THRESHOLD',`',`define(`COPYD_SSE_THRESHOLD', 7)')

ASM_START()
	TEXT
	ALIGN(64)
PROLOGUE(mpn_copyd)
	FUNC_ENTRY(3)

	lea	-8(up,n,8), up
	lea	-8(rp,n,8), rp

	cmp	$COPYD_SSE_THRESHOLD, n
	jbe	L(bc)

	bt	$3, R32(rp)		C is rp 16-byte aligned?
	jc	L(rp_aligned)		C jump if rp aligned

	mov	(up), %rax		C copy one limb
	mov	%rax, (rp)
	lea	-8(up), up
	lea	-8(rp), rp
	dec	n

L(rp_aligned):
	bt	$3, R32(up)
	jnc	L(uent)

ifelse(eval(COPYD_SSE_THRESHOLD >= 8),1,
`	sub	$8, n',
`	jmp	L(am)')

	ALIGN(16)
L(atop):movdqa	-8(up), %xmm0
	movdqa	-24(up), %xmm1
	movdqa	-40(up), %xmm2
	movdqa	-56(up), %xmm3
	lea	-64(up), up
	movdqa	%xmm0, -8(rp)
	movdqa	%xmm1, -24(rp)
	movdqa	%xmm2, -40(rp)
	movdqa	%xmm3, -56(rp)
	lea	-64(rp), rp
L(am):	sub	$8, n
	jnc	L(atop)

	bt	$2, R32(n)
	jnc	1f
	movdqa	-8(up), %xmm0
	movdqa	-24(up), %xmm1
	lea	-32(up), up
	movdqa	%xmm0, -8(rp)
	movdqa	%xmm1, -24(rp)
	lea	-32(rp), rp

1:	bt	$1, R32(n)
	jnc	1f
	movdqa	-8(up), %xmm0
	lea	-16(up), up
	movdqa	%xmm0, -8(rp)
	lea	-16(rp), rp

1:	bt	$0, n
	jnc	1f
	mov	(up), %r8
	mov	%r8, (rp)

1:	FUNC_EXIT()
	ret

L(uent):sub	$16, n
	movdqa	(up), %xmm0
	jc	L(uend)

	ALIGN(16)
L(utop):sub	$16, n
	movdqa	-16(up), %xmm1
	palignr($8, %xmm1, %xmm0)
	movdqa	%xmm0, -8(rp)
	movdqa	-32(up), %xmm2
	palignr($8, %xmm2, %xmm1)
	movdqa	%xmm1, -24(rp)
	movdqa	-48(up), %xmm3
	palignr($8, %xmm3, %xmm2)
	movdqa	%xmm2, -40(rp)
	movdqa	-64(up), %xmm0
	palignr($8, %xmm0, %xmm3)
	movdqa	%xmm3, -56(rp)
	movdqa	-80(up), %xmm1
	palignr($8, %xmm1, %xmm0)
	movdqa	%xmm0, -72(rp)
	movdqa	-96(up), %xmm2
	palignr($8, %xmm2, %xmm1)
	movdqa	%xmm1, -88(rp)
	movdqa	-112(up), %xmm3
	palignr($8, %xmm3, %xmm2)
	movdqa	%xmm2, -104(rp)
	movdqa	-128(up), %xmm0
	palignr($8, %xmm0, %xmm3)
	movdqa	%xmm3, -120(rp)
	lea	-128(up), up
	lea	-128(rp), rp
	jnc	L(utop)

L(uend):bt	$3, R32(n)
	jnc	1f
	movdqa	-16(up), %xmm1
	palignr($8, %xmm1, %xmm0)
	movdqa	%xmm0, -8(rp)
	movdqa	-32(up), %xmm0
	palignr($8, %xmm0, %xmm1)
	movdqa	%xmm1, -24(rp)
	movdqa	-48(up), %xmm1
	palignr($8, %xmm1, %xmm0)
	movdqa	%xmm0, -40(rp)
	movdqa	-64(up), %xmm0
	palignr($8, %xmm0, %xmm1)
	movdqa	%xmm1, -56(rp)
	lea	-64(up), up
	lea	-64(rp), rp

1:	bt	$2, R32(n)
	jnc	1f
	movdqa	-16(up), %xmm1
	palignr($8, %xmm1, %xmm0)
	movdqa	%xmm0, -8(rp)
	movdqa	-32(up), %xmm0
	palignr($8, %xmm0, %xmm1)
	movdqa	%xmm1, -24(rp)
	lea	-32(up), up
	lea	-32(rp), rp

1:	bt	$1, R32(n)
	jnc	1f
	movdqa	-16(up), %xmm1
	palignr($8, %xmm1, %xmm0)
	movdqa	%xmm0, -8(rp)
	lea	-16(up), up
	lea	-16(rp), rp

1:	bt	$0, n
	jnc	1f
	mov	(up), %r8
	mov	%r8, (rp)

1:	FUNC_EXIT()
	ret

C Basecase code.  Needed for good small operands speed, not for
C correctness as the above code is currently written.

L(bc):	sub	$4, R32(n)
	jc	L(end)

	ALIGN(16)
L(top):	mov	(up), %r8
	mov	-8(up), %r9
	lea	-32(rp), rp
	mov	-16(up), %r10
	mov	-24(up), %r11
	lea	-32(up), up
	mov	%r8, 32(rp)
	mov	%r9, 24(rp)
ifelse(eval(COPYD_SSE_THRESHOLD >= 8),1,
`	sub	$4, R32(n)')
	mov	%r10, 16(rp)
	mov	%r11, 8(rp)
ifelse(eval(COPYD_SSE_THRESHOLD >= 8),1,
`	jnc	L(top)')

L(end):	bt	$0, R32(n)
	jnc	1f
	mov	(up), %r8
	mov	%r8, (rp)
	lea	-8(rp), rp
	lea	-8(up), up
1:	bt	$1, R32(n)
	jnc	1f
	mov	(up), %r8
	mov	-8(up), %r9
	mov	%r8, (rp)
	mov	%r9, -8(rp)
1:	FUNC_EXIT()
	ret
EPILOGUE()
