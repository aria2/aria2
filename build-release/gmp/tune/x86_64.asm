dnl  x86 pentium time stamp counter access routine.

dnl  Copyright 1999, 2000, 2003, 2004, 2005 Free Software Foundation, Inc.
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


C void speed_cyclecounter (unsigned p[2]);
C
C Get the pentium rdtsc cycle counter, storing the least significant word in
C p[0] and the most significant in p[1].
C
C cpuid is used to serialize execution.  On big measurements this won't be
C significant but it may help make small single measurements more accurate.

PROLOGUE(speed_cyclecounter)

	C rdi	p

	movq	%rbx, %r10
	xorl	%eax, %eax
	cpuid
	rdtsc
	movl	%eax, (%rdi)
	movl	%edx, 4(%rdi)
	movq	%r10, %rbx
	ret
EPILOGUE()
