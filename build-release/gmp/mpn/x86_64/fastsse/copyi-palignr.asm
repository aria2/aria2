dnl  AMD64 mpn_copyi optimised for CPUs with fast SSE copying and SSSE3.

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
C AMD bd1	 1.39		 ? 1.45				Y/N
C AMD bobcat	 1.97		 ? 8.17		1.5/1.5		N
C Intel P4	 2.26		 illop				Y/N
C Intel core2	 0.52		 0.82		opt/0.74	Y
C Intel NHM	 0.52		 0.65		opt/opt		Y
C Intel SBR	 0.51		 0.55		opt/0.51	Y
C Intel atom	 1.16		 1.70		opt/opt		Y
C VIA nano	 1.09		 1.10		opt/opt		Y

C We use only 16-byte operations, except for unaligned top-most and bottom-most
C limbs.  We use the SSSE3 palignr instruction when rp - up = 8 (mod 16).  That
C instruction is better adapted to mpn_copyd's needs, we need to contort the
C code to use it here.
C
C For operands of < COPYI_SSE_THRESHOLD limbs, we use a plain 64-bit loop,
C taken from the x86_64 default code.

C INPUT PARAMETERS
define(`rp', `%rdi')
define(`up', `%rsi')
define(`n',  `%rdx')

C There are three instructions for loading an aligned 128-bit quantity.  We use
C movaps, since it has the shortest coding.
define(`movdqa', ``movaps'')

ifdef(`COPYI_SSE_THRESHOLD',`',`define(`COPYI_SSE_THRESHOLD', 7)')

ASM_START()
	TEXT
	ALIGN(64)
PROLOGUE(mpn_copyi)
	FUNC_ENTRY(3)

	cmp	$COPYI_SSE_THRESHOLD, n
	jbe	L(bc)

	bt	$3, R32(rp)		C is rp 16-byte aligned?
	jnc	L(rp_aligned)		C jump if rp aligned

	movsq				C copy one limb
	dec	n

L(rp_aligned):
	bt	$3, R32(up)
	jc	L(uent)

ifelse(eval(COPYI_SSE_THRESHOLD >= 8),1,
`	sub	$8, n',
`	jmp	L(am)')

	ALIGN(16)
L(atop):movdqa	0(up), %xmm0
	movdqa	16(up), %xmm1
	movdqa	32(up), %xmm2
	movdqa	48(up), %xmm3
	lea	64(up), up
	movdqa	%xmm0, (rp)
	movdqa	%xmm1, 16(rp)
	movdqa	%xmm2, 32(rp)
	movdqa	%xmm3, 48(rp)
	lea	64(rp), rp
L(am):	sub	$8, n
	jnc	L(atop)

	bt	$2, R32(n)
	jnc	1f
	movdqa	(up), %xmm0
	movdqa	16(up), %xmm1
	lea	32(up), up
	movdqa	%xmm0, (rp)
	movdqa	%xmm1, 16(rp)
	lea	32(rp), rp

1:	bt	$1, R32(n)
	jnc	1f
	movdqa	(up), %xmm0
	lea	16(up), up
	movdqa	%xmm0, (rp)
	lea	16(rp), rp

1:	bt	$0, n
	jnc	1f
	mov	(up), %r8
	mov	%r8, (rp)

1:	FUNC_EXIT()
	ret

L(uent):
C Code handling up - rp = 8 (mod 16)

C FIXME: The code below only handles overlap if it is close to complete, or
C quite separate: up-rp < 5 or up-up > 15 limbs
	lea	-40(up), %rax		C 40 = 5 * GMP_LIMB_BYTES
	sub	rp, %rax
	cmp	$80, %rax		C 80 = (15-5) * GMP_LIMB_BYTES
	jbe	L(bc)			C deflect to plain loop

	sub	$16, n
	jc	L(uend)

	movdqa	120(up), %xmm3

	sub	$16, n
	jmp	L(um)

	ALIGN(16)
L(utop):movdqa	120(up), %xmm3
	movdqa	%xmm0, -128(rp)
	sub	$16, n
L(um):	movdqa	104(up), %xmm2
	palignr($8, %xmm2, %xmm3)
	movdqa	88(up), %xmm1
	movdqa	%xmm3, 112(rp)
	palignr($8, %xmm1, %xmm2)
	movdqa	72(up), %xmm0
	movdqa	%xmm2, 96(rp)
	palignr($8, %xmm0, %xmm1)
	movdqa	56(up), %xmm3
	movdqa	%xmm1, 80(rp)
	palignr($8, %xmm3, %xmm0)
	movdqa	40(up), %xmm2
	movdqa	%xmm0, 64(rp)
	palignr($8, %xmm2, %xmm3)
	movdqa	24(up), %xmm1
	movdqa	%xmm3, 48(rp)
	palignr($8, %xmm1, %xmm2)
	movdqa	8(up), %xmm0
	movdqa	%xmm2, 32(rp)
	palignr($8, %xmm0, %xmm1)
	movdqa	-8(up), %xmm3
	movdqa	%xmm1, 16(rp)
	palignr($8, %xmm3, %xmm0)
	lea	128(up), up
	lea	128(rp), rp
	jnc	L(utop)

	movdqa	%xmm0, -128(rp)

L(uend):bt	$3, R32(n)
	jnc	1f
	movdqa	56(up), %xmm3
	movdqa	40(up), %xmm2
	palignr($8, %xmm2, %xmm3)
	movdqa	24(up), %xmm1
	movdqa	%xmm3, 48(rp)
	palignr($8, %xmm1, %xmm2)
	movdqa	8(up), %xmm0
	movdqa	%xmm2, 32(rp)
	palignr($8, %xmm0, %xmm1)
	movdqa	-8(up), %xmm3
	movdqa	%xmm1, 16(rp)
	palignr($8, %xmm3, %xmm0)
	lea	64(up), up
	movdqa	%xmm0, (rp)
	lea	64(rp), rp

1:	bt	$2, R32(n)
	jnc	1f
	movdqa	24(up), %xmm1
	movdqa	8(up), %xmm0
	palignr($8, %xmm0, %xmm1)
	movdqa	-8(up), %xmm3
	movdqa	%xmm1, 16(rp)
	palignr($8, %xmm3, %xmm0)
	lea	32(up), up
	movdqa	%xmm0, (rp)
	lea	32(rp), rp

1:	bt	$1, R32(n)
	jnc	1f
	movdqa	8(up), %xmm0
	movdqa	-8(up), %xmm3
	palignr($8, %xmm3, %xmm0)
	lea	16(up), up
	movdqa	%xmm0, (rp)
	lea	16(rp), rp

1:	bt	$0, n
	jnc	1f
	mov	(up), %r8
	mov	%r8, (rp)

1:	FUNC_EXIT()
	ret

C Basecase code.  Needed for good small operands speed, not for
C correctness as the above code is currently written.

L(bc):	lea	-8(rp), rp
	sub	$4, R32(n)
	jc	L(end)

	ALIGN(16)
L(top):	mov	(up), %r8
	mov	8(up), %r9
	lea	32(rp), rp
	mov	16(up), %r10
	mov	24(up), %r11
	lea	32(up), up
	mov	%r8, -24(rp)
	mov	%r9, -16(rp)
ifelse(eval(1 || COPYI_SSE_THRESHOLD >= 8),1,
`	sub	$4, R32(n)')
	mov	%r10, -8(rp)
	mov	%r11, (rp)
ifelse(eval(1 || COPYI_SSE_THRESHOLD >= 8),1,
`	jnc	L(top)')

L(end):	bt	$0, R32(n)
	jnc	1f
	mov	(up), %r8
	mov	%r8, 8(rp)
	lea	8(rp), rp
	lea	8(up), up
1:	bt	$1, R32(n)
	jnc	1f
	mov	(up), %r8
	mov	8(up), %r9
	mov	%r8, 8(rp)
	mov	%r9, 16(rp)
1:	FUNC_EXIT()
	ret
EPILOGUE()
