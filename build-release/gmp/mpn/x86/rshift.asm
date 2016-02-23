dnl  x86 mpn_rshift -- mpn right shift.

dnl  Copyright 1992, 1994, 1996, 1999, 2000, 2001, 2002 Free Software
dnl  Foundation, Inc.
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


C     cycles/limb
C P54	 7.5
C P55	 7.0
C P6	 2.5
C K6	 4.5
C K7	 5.0
C P4	16.5


C mp_limb_t mpn_rshift (mp_ptr dst, mp_srcptr src, mp_size_t size,
C                       unsigned shift);

defframe(PARAM_SHIFT,16)
defframe(PARAM_SIZE, 12)
defframe(PARAM_SRC,  8)
defframe(PARAM_DST,  4)

	TEXT
	ALIGN(8)
PROLOGUE(mpn_rshift)

	pushl	%edi
	pushl	%esi
	pushl	%ebx
deflit(`FRAME',12)

	movl	PARAM_DST,%edi
	movl	PARAM_SRC,%esi
	movl	PARAM_SIZE,%edx
	movl	PARAM_SHIFT,%ecx

	leal	-4(%edi,%edx,4),%edi
	leal	(%esi,%edx,4),%esi
	negl	%edx

	movl	(%esi,%edx,4),%ebx	C read least significant limb
	xorl	%eax,%eax
	shrdl(	%cl, %ebx, %eax)	C compute carry limb
	incl	%edx
	jz	L(end)
	pushl	%eax			C push carry limb onto stack
	testb	$1,%dl
	jnz	L(1)			C enter loop in the middle
	movl	%ebx,%eax

	ALIGN(8)
L(oop):	movl	(%esi,%edx,4),%ebx	C load next higher limb
	shrdl(	%cl, %ebx, %eax)	C compute result limb
	movl	%eax,(%edi,%edx,4)	C store it
	incl	%edx
L(1):	movl	(%esi,%edx,4),%eax
	shrdl(	%cl, %eax, %ebx)
	movl	%ebx,(%edi,%edx,4)
	incl	%edx
	jnz	L(oop)

	shrl	%cl,%eax		C compute most significant limb
	movl	%eax,(%edi)		C store it

	popl	%eax			C pop carry limb

	popl	%ebx
	popl	%esi
	popl	%edi
	ret

L(end):	shrl	%cl,%ebx		C compute most significant limb
	movl	%ebx,(%edi)		C store it

	popl	%ebx
	popl	%esi
	popl	%edi
	ret

EPILOGUE()
