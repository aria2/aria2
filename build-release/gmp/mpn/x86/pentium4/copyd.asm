dnl  Pentium-4 mpn_copyd -- copy limb vector, decrementing.
dnl

dnl  Copyright 1999, 2000, 2001 Free Software Foundation, Inc.
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


dnl  The std/rep/movsl/cld is very slow for small blocks on pentium4.  Its
dnl  startup time seems to be about 165 cycles.  It then needs 2.6 c/l.
dnl  We therefore use an open-coded 2 c/l copying loop.

dnl  Ultimately, we may want to use 64-bit movq or 128-bit movdqu in some
dnl  nifty unrolled arrangement.  Clearly, that could reach much higher
dnl  speeds, at least for large blocks.

include(`../config.m4')


defframe(PARAM_SIZE, 12)
defframe(PARAM_SRC, 8)
defframe(PARAM_DST,  4)

	TEXT
	ALIGN(8)

PROLOGUE(mpn_copyd)
deflit(`FRAME',0)

	movl	PARAM_SIZE, %ecx

	movl	PARAM_SRC, %eax
	movl	PARAM_DST, %edx
	movl	%ebx, PARAM_SIZE
	addl	$-1, %ecx
	js	L(end)

L(loop):
	movl	(%eax,%ecx,4), %ebx
	movl	%ebx, (%edx,%ecx,4)
	addl	$-1, %ecx

	jns	L(loop)
L(end):
	movl	PARAM_SIZE, %ebx
	ret

EPILOGUE()
