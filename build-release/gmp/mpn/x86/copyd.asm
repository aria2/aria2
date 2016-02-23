dnl  x86 mpn_copyd -- copy limb vector, decrementing.

dnl  Copyright 1999, 2000, 2001, 2002 Free Software Foundation, Inc.
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


C     cycles/limb  startup (approx)
C P5	  1.0	      40
C P6	  2.4	      70
C K6	  1.0	      55
C K7	  1.3	      75
C P4	  2.6	     175
C
C (Startup time includes some function call overheads.)


C void mpn_copyd (mp_ptr dst, mp_srcptr src, mp_size_t size);
C
C Copy src,size to dst,size, working from high to low addresses.
C
C The code here is very generic and can be expected to be reasonable on all
C the x86 family.

defframe(PARAM_SIZE,12)
defframe(PARAM_SRC, 8)
defframe(PARAM_DST, 4)
deflit(`FRAME',0)

	TEXT
	ALIGN(32)

PROLOGUE(mpn_copyd)
	C eax	saved esi
	C ebx
	C ecx	counter
	C edx	saved edi
	C esi	src
	C edi	dst
	C ebp

	movl	PARAM_SIZE, %ecx
	movl	%esi, %eax

	movl	PARAM_SRC, %esi
	movl	%edi, %edx

	movl	PARAM_DST, %edi
	leal	-4(%esi,%ecx,4), %esi

	leal	-4(%edi,%ecx,4), %edi

	std

	rep
	movsl

	cld

	movl	%eax, %esi
	movl	%edx, %edi

	ret

EPILOGUE()
