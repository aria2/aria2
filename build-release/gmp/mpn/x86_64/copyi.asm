dnl  AMD64 mpn_copyi -- copy limb vector, incrementing.

dnl  Copyright 2003, 2005, 2007, 2011, 2012 Free Software Foundation, Inc.

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
C AMD K8,K9	 1
C AMD K10	 1
C AMD bd1	 1.36
C AMD bobcat	 1.71
C Intel P4	 2-3
C Intel core2	 1
C Intel NHM	 1
C Intel SBR	 1
C Intel atom	 2
C VIA nano	 2


IFSTD(`define(`rp',`%rdi')')
IFSTD(`define(`up',`%rsi')')
IFSTD(`define(`n', `%rdx')')

IFDOS(`define(`rp',`%rcx')')
IFDOS(`define(`up',`%rdx')')
IFDOS(`define(`n', `%r8')')

ABI_SUPPORT(DOS64)
ABI_SUPPORT(STD64)

ASM_START()
	TEXT
	ALIGN(64)
	.byte	0,0,0,0,0,0
PROLOGUE(mpn_copyi)
	lea	-8(rp), rp
	sub	$4, n
	jc	L(end)

L(top):	mov	(up), %rax
	mov	8(up), %r9
	lea	32(rp), rp
	mov	16(up), %r10
	mov	24(up), %r11
	lea	32(up), up
	mov	%rax, -24(rp)
	mov	%r9, -16(rp)
	sub	$4, n
	mov	%r10, -8(rp)
	mov	%r11, (rp)
	jnc	L(top)

L(end):	shr	R32(n)
	jnc	1f
	mov	(up), %rax
	mov	%rax, 8(rp)
	lea	8(rp), rp
	lea	8(up), up
1:	shr	R32(n)
	jnc	1f
	mov	(up), %rax
	mov	8(up), %r9
	mov	%rax, 8(rp)
	mov	%r9, 16(rp)
1:	ret
EPILOGUE()
