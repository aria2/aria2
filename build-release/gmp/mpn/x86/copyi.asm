dnl  x86 mpn_copyi -- copy limb vector, incrementing.

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
C P5	  1.0	      35
C P6	  0.75	      45
C K6	  1.0	      30
C K7	  1.3	      65
C P4	  1.0	     120
C
C (Startup time includes some function call overheads.)


C void mpn_copyi (mp_ptr dst, mp_srcptr src, mp_size_t size);
C
C Copy src,size to dst,size, working from low to high addresses.
C
C The code here is very generic and can be expected to be reasonable on all
C the x86 family.
C
C P6 -  An MMX based copy was tried, but was found to be slower than a rep
C       movs in all cases.  The fastest MMX found was 0.8 cycles/limb (when
C       fully aligned).  A rep movs seems to have a startup time of about 15
C       cycles, but doing something special for small sizes could lead to a
C       branch misprediction that would destroy any saving.  For now a plain
C       rep movs seems ok.
C
C K62 - We used to have a big chunk of code doing an MMX copy at 0.56 c/l if
C       aligned or a 1.0 rep movs if not.  But that seemed excessive since
C       it only got an advantage half the time, and even then only showed it
C       above 50 limbs or so.

defframe(PARAM_SIZE,12)
defframe(PARAM_SRC, 8)
defframe(PARAM_DST, 4)
deflit(`FRAME',0)

	TEXT
	ALIGN(32)

	C eax	saved esi
	C ebx
	C ecx	counter
	C edx	saved edi
	C esi	src
	C edi	dst
	C ebp

PROLOGUE(mpn_copyi)

	movl	PARAM_SIZE, %ecx
	movl	%esi, %eax

	movl	PARAM_SRC, %esi
	movl	%edi, %edx

	movl	PARAM_DST, %edi

	cld	C better safe than sorry, see mpn/x86/README

	rep
	movsl

	movl	%eax, %esi
	movl	%edx, %edi

	ret

EPILOGUE()
