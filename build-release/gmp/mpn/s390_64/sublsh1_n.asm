dnl  S/390-64 mpn_sublsh1_n

dnl  Copyright 2011 Free Software Foundation, Inc.

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

C            cycles/limb
C z900		10
C z990		 5
C z9		 ?
C z10		 ?
C z196		 ?

C TODO
C  * Optimise for small n
C  * Compute RETVAL for sublsh1_n less stupidly

C INPUT PARAMETERS
define(`rp',	`%r2')
define(`up',	`%r3')
define(`vp',	`%r4')
define(`n',	`%r5')

ifdef(`OPERATION_addlsh1_n',`
  define(ADSBR,		algr)
  define(ADSBCR,	alcgr)
  define(INITCY,	`lghi	%r13, -1')
  define(RETVAL,	`la	%r2, 2(%r1,%r13)')
  define(func, mpn_addlsh1_n)
')
ifdef(`OPERATION_sublsh1_n',`
  define(ADSBR,		slgr)
  define(ADSBCR,	slbgr)
  define(INITCY,	`lghi	%r13, 0')
  define(RETVAL,`dnl
	slgr	%r1, %r13
	lghi	%r2, 1
	algr	%r2, %r1')
  define(func, mpn_sublsh1_n)
')

ASM_START()
PROLOGUE(mpn_sublsh1_n)
	stmg	%r6, %r13, 48(%r15)

	aghi	n, 3
	lghi	%r7, 3
	srlg	%r0, n, 2
	ngr	%r7, n			C n mod 4
	je	L(b1)
	cghi	%r7, 2
	jl	L(b2)
	jne	L(b0)

L(b3):	lmg	%r5, %r7, 0(up)
	la	up, 24(up)
	lmg	%r9, %r11, 0(vp)
	la	vp, 24(vp)

	algr	%r9, %r9
	alcgr	%r10, %r10
	alcgr	%r11, %r11
	slbgr	%r1, %r1

	ADSBR	%r5, %r9
	ADSBCR	%r6, %r10
	ADSBCR	%r7, %r11
	slbgr	%r13, %r13

	stmg	%r5, %r7, 0(rp)
	la	rp, 24(rp)
	brctg	%r0, L(top)
	j	L(end)

L(b0):	lghi	%r1, -1
	INITCY
	j	L(top)

L(b1):	lg	%r5, 0(up)
	la	up, 8(up)
	lg	%r9, 0(vp)
	la	vp, 8(vp)

	algr	%r9, %r9
	slbgr	%r1, %r1
	ADSBR	%r5, %r9
	slbgr	%r13, %r13

	stg	%r5, 0(rp)
	la	rp, 8(rp)
	brctg	%r0, L(top)
	j	L(end)

L(b2):	lmg	%r5, %r6, 0(up)
	la	up, 16(up)
	lmg	%r9, %r10, 0(vp)
	la	vp, 16(vp)

	algr	%r9, %r9
	alcgr	%r10, %r10
	slbgr	%r1, %r1

	ADSBR	%r5, %r9
	ADSBCR	%r6, %r10
	slbgr	%r13, %r13

	stmg	%r5, %r6, 0(rp)
	la	rp, 16(rp)
	brctg	%r0, L(top)
	j	L(end)

L(top):	lmg	%r9, %r12, 0(vp)
	la	vp, 32(vp)

	aghi	%r1, 1			C restore carry

	alcgr	%r9, %r9
	alcgr	%r10, %r10
	alcgr	%r11, %r11
	alcgr	%r12, %r12

	slbgr	%r1, %r1		C save carry

	lmg	%r5, %r8, 0(up)
	la	up, 32(up)

	aghi	%r13, 1			C restore carry

	ADSBCR	%r5, %r9
	ADSBCR	%r6, %r10
	ADSBCR	%r7, %r11
	ADSBCR	%r8, %r12

	slbgr	%r13, %r13		C save carry

	stmg	%r5, %r8, 0(rp)
	la	rp, 32(rp)
	brctg	%r0, L(top)

L(end):	RETVAL
	lmg	%r6, %r13, 48(%r15)
	br	%r14
EPILOGUE()
