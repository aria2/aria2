dnl  Intel Pentium-4 mpn_add_n -- mpn addition.

dnl  Copyright 2001, 2002 Free Software Foundation, Inc.
dnl
dnl  This file is part of the GNU MP Library.
dnl
dnl  The GNU MP Library is free software; you can redistribute it and/or
dnl  modify it under the terms of the GNU Lesser General Public License as
dnl  published by the Free Software Foundation; either version 3 of the
dnl  License, or (at your option) any later version.
dnl
dnl  The GNU MP Library is distributed in the hope that it will be useful,
dnl  but WITHOUT ANY WARRANTY; without even the implied warranty of
dnl  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
dnl  Lesser General Public License for more details.
dnl
dnl  You should have received a copy of the GNU Lesser General Public License
dnl  along with the GNU MP Library.  If not, see http://www.gnu.org/licenses/.

include(`../config.m4')


C					cycles/limb
C			     dst!=src1,2  dst==src1  dst==src2
C P6 model 0-8,10-12		-
C P6 model 9   (Banias)		?
C P6 model 13  (Dothan)		?
C P4 model 0-1 (Willamette)	?
C P4 model 2   (Northwood)	4	     6		6
C P4 model 3-4 (Prescott)	4.25	     7.5	7.5

defframe(PARAM_CARRY,20)
defframe(PARAM_SIZE, 16)
defframe(PARAM_SRC2, 12)
defframe(PARAM_SRC1, 8)
defframe(PARAM_DST,  4)

dnl  re-use parameter space
define(SAVE_EBX,`PARAM_SRC1')

	TEXT
	ALIGN(8)

PROLOGUE(mpn_add_nc)
deflit(`FRAME',0)
	movd	PARAM_CARRY, %mm0
	jmp	L(start_nc)
EPILOGUE()

	ALIGN(8)
PROLOGUE(mpn_add_n)
deflit(`FRAME',0)
	pxor	%mm0, %mm0
L(start_nc):
	mov	PARAM_SRC1, %eax
	mov	%ebx, SAVE_EBX
	mov	PARAM_SRC2, %ebx
	mov	PARAM_DST, %edx
	mov	PARAM_SIZE, %ecx

	lea	(%eax,%ecx,4), %eax	C src1 end
	lea	(%ebx,%ecx,4), %ebx	C src2 end
	lea	(%edx,%ecx,4), %edx	C dst end
	neg	%ecx			C -size

L(top):
	C eax	src1 end
	C ebx	src2 end
	C ecx	counter, limbs, negative
	C edx	dst end
	C mm0	carry bit

	movd	(%eax,%ecx,4), %mm1
	movd	(%ebx,%ecx,4), %mm2
	paddq	%mm2, %mm1

	paddq	%mm1, %mm0
	movd	%mm0, (%edx,%ecx,4)

	psrlq	$32, %mm0

	add	$1, %ecx
	jnz	L(top)

	movd	%mm0, %eax
	mov	SAVE_EBX, %ebx
	emms
	ret

EPILOGUE()
