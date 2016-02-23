dnl  ARM mpn_rsh1add_n and mpn_rsh1sub_n.

dnl  Contributed to the GNU project by Torbjorn Granlund.

dnl  Copyright 2012 Free Software Foundation, Inc.

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
C StrongARM	 ?
C XScale	 ?
C Cortex-A8	 ?
C Cortex-A9	3.64-3.7
C Cortex-A15	 ?

C TODO
C  * Not optimised.

define(`rp', `r0')
define(`up', `r1')
define(`vp', `r2')
define(`n',  `r3')

ifdef(`OPERATION_rsh1add_n', `
  define(`ADDSUB',	adds)
  define(`ADDSUBC',	adcs)
  define(`RSTCY',	`cmn	$1, $1')
  define(`func',	mpn_rsh1add_n)
  define(`func_nc',	mpn_rsh1add_nc)')
ifdef(`OPERATION_rsh1sub_n', `
  define(`ADDSUB',	subs)
  define(`ADDSUBC',	sbcs)
  define(`RSTCY',
	`mvn	$2, #0x80000000
	cmp	$2, $1')
  define(`func',	mpn_rsh1sub_n)
  define(`func_nc',	mpn_rsh1sub_nc)')

MULFUNC_PROLOGUE(mpn_rsh1add_n mpn_rsh1sub_n)

ASM_START()
PROLOGUE(func)
	push	{r4-r11}
	ldr	r4, [up], #4
	ldr	r8, [vp], #4
	ADDSUB	r4, r4, r8
	rrxs	r12, r7
	and	r11, r4, #1	C return value
	subs	n, n, #4
	blo	L(end)

L(top):	ldmia	up!, {r5,r6,r7}
	ldmia	vp!, {r8,r9,r10}
	cmn	r12, r12
	ADDSUBC	r5, r5, r8
	ADDSUBC	r6, r6, r9
	ADDSUBC	r7, r7, r10
	rrxs	r12, r7
	rrxs	r6, r6
	rrxs	r5, r5
	rrxs	r4, r4
	subs	n, n, #3
	stmia	rp!, {r4,r5,r6}
	mov	r4, r7
	bhs	L(top)

L(end):	cmn	n, #2
	bls	L(e2)
	ldm	up, {r5,r6}
	ldm	vp, {r8,r9}
	cmn	r12, r12
	ADDSUBC	r5, r5, r8
	ADDSUBC	r6, r6, r9
	rrxs	r12, r6
	rrxs	r5, r5
	rrxs	r4, r4
	stmia	rp!, {r4,r5}
	mov	r4, r6
	b	L(e1)

L(e2):	bne	L(e1)
	ldr	r5, [up, #0]
	ldr	r8, [vp, #0]
	cmn	r12, r12
	ADDSUBC	r5, r5, r8
	rrxs	r12, r5
	rrxs	r4, r4
	str	r4, [rp], #4
	mov	r4, r5

L(e1):	RSTCY(	r12, r1)
	rrxs	r4, r4
	str	r4, [rp, #0]
	mov	r0, r11
	pop	{r4-r11}
	bx	r14
EPILOGUE()
