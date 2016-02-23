dnl  Alpha time stamp counter access routine.

dnl  Copyright 2000, 2005 Free Software Foundation, Inc.
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


C void speed_cyclecounter (unsigned int p[2]);
C

C The rpcc instruction returns a 64-bit value split into two 32-bit fields.
C The lower 32 bits are set by the hardware, and the upper 32 bits are set
C by the operating system.  The real per-process cycle count is the sum of
C these halves.

C Unfortunately, some operating systems don't get this right.  NetBSD 1.3 is
C known to sometimes put garbage in the upper half.  Whether newer NetBSD
C versions get it right, is unknown to us.

C rpcc measures cycles elapsed in the user program and hence should be very
C accurate even on a busy system.  Losing cache contents due to task
C switching may have an effect though.

ASM_START()
PROLOGUE(speed_cyclecounter)
	rpcc	r0
	srl	r0,32,r1
	addq	r1,r0,r0
	stl	r0,0(r16)
	stl	r31,4(r16)		C zero upper return word
	ret	r31,(r26),1
EPILOGUE(speed_cyclecounter)
ASM_END()
