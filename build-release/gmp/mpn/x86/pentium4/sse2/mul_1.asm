dnl  mpn_mul_1 for Pentium 4 and P6 models with SSE2 (i.e., 9,D,E,F).

dnl  Copyright 2005, 2007, 2011 Free Software Foundation, Inc.
dnl
dnl  This file is part of the GNU MP Library.
dnl
dnl  The GNU MP Library is free software; you can redistribute it and/or modify
dnl  it under the terms of the GNU Lesser General Public License as published
dnl  by the Free Software Foundation; either version 3 of the License, or (at
dnl  your option) any later version.
dnl
dnl  The GNU MP Library is distributed in the hope that it will be useful, but
dnl  WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
dnl  or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public
dnl  License for more details.
dnl
dnl  You should have received a copy of the GNU Lesser General Public License
dnl  along with the GNU MP Library.  If not, see http://www.gnu.org/licenses/.


include(`../config.m4')

C                           cycles/limb
C P6 model 0-8,10-12		-
C P6 model 9   (Banias)		4.17
C P6 model 13  (Dothan)		4.17
C P4 model 0-1 (Willamette)	4
C P4 model 2   (Northwood)	4
C P4 model 3-4 (Prescott)	4.55

C TODO:
C  * Tweak eax/edx offsets in loop as to save some lea's
C  * Perhaps software pipeline small-case code

C INPUT PARAMETERS
C rp		sp + 4
C up		sp + 8
C n		sp + 12
C v0		sp + 16

	TEXT
	ALIGN(16)
PROLOGUE(mpn_mul_1)
	pxor	%mm6, %mm6
L(ent):	mov	4(%esp), %edx
	mov	8(%esp), %eax
	mov	12(%esp), %ecx
	movd	16(%esp), %mm7
	cmp	$4, %ecx
	jnc	L(big)

L(lp0):	movd	(%eax), %mm0
	lea	4(%eax), %eax
	lea	4(%edx), %edx
	pmuludq	%mm7, %mm0
	paddq	%mm0, %mm6
	movd	%mm6, -4(%edx)
	psrlq	$32, %mm6
	dec	%ecx
	jnz	L(lp0)
	movd	%mm6, %eax
	emms
	ret

L(big):	and	$3, %ecx
	je	L(0)
	cmp	$2, %ecx
	jc	L(1)
	je	L(2)
	jmp	L(3)			C FIXME: one case should fall through

L(0):	movd	(%eax), %mm3
	sub	12(%esp), %ecx		C loop count
	lea	-16(%eax), %eax
	lea	-12(%edx), %edx
	pmuludq	%mm7, %mm3
	movd	20(%eax), %mm0
	pmuludq	%mm7, %mm0
	movd	24(%eax), %mm1
	jmp	L(00)

L(1):	movd	(%eax), %mm2
	sub	12(%esp), %ecx
	lea	-12(%eax), %eax
	lea	-8(%edx), %edx
	pmuludq	%mm7, %mm2
	movd	16(%eax), %mm3
	pmuludq	%mm7, %mm3
	movd	20(%eax), %mm0
	jmp	L(01)

L(2):	movd	(%eax), %mm1
	sub	12(%esp), %ecx
	lea	-8(%eax), %eax
	lea	-4(%edx), %edx
	pmuludq	%mm7, %mm1
	movd	12(%eax), %mm2
	pmuludq	%mm7, %mm2
	movd	16(%eax), %mm3
	jmp	L(10)

L(3):	movd	(%eax), %mm0
	sub	12(%esp), %ecx
	lea	-4(%eax), %eax
	pmuludq	%mm7, %mm0
	movd	8(%eax), %mm1
	pmuludq	%mm7, %mm1
	movd	12(%eax), %mm2

	ALIGN(16)
L(top):	pmuludq	%mm7, %mm2
	paddq	%mm0, %mm6
	movd	16(%eax), %mm3
	movd	%mm6, 0(%edx)
	psrlq	$32, %mm6
L(10):	pmuludq	%mm7, %mm3
	paddq	%mm1, %mm6
	movd	20(%eax), %mm0
	movd	%mm6, 4(%edx)
	psrlq	$32, %mm6
L(01):	pmuludq	%mm7, %mm0
	paddq	%mm2, %mm6
	movd	24(%eax), %mm1
	movd	%mm6, 8(%edx)
	psrlq	$32, %mm6
L(00):	pmuludq	%mm7, %mm1
	paddq	%mm3, %mm6
	movd	28(%eax), %mm2
	movd	%mm6, 12(%edx)
	psrlq	$32, %mm6
	lea	16(%eax), %eax
	lea	16(%edx), %edx
	add	$4, %ecx
	ja	L(top)

L(end):	pmuludq	%mm7, %mm2
	paddq	%mm0, %mm6
	movd	%mm6, 0(%edx)
	psrlq	$32, %mm6
	paddq	%mm1, %mm6
	movd	%mm6, 4(%edx)
	psrlq	$32, %mm6
	paddq	%mm2, %mm6
	movd	%mm6, 8(%edx)
	psrlq	$32, %mm6
	movd	%mm6, %eax
	emms
	ret
EPILOGUE()
PROLOGUE(mpn_mul_1c)
	movd	20(%esp), %mm6
	jmp	L(ent)
EPILOGUE()
