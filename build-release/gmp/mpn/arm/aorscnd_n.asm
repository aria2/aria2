dnl  ARM mpn_addcnd_n, mpn_subcnd_n

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
C Cortex-A9	 2.5	slightly fluctuating
C Cortex-A15	 ?

define(`rp',	`r0')
define(`up',	`r1')
define(`vp',	`r2')
define(`n',	`r3')

define(`cnd',	`r12')

ifdef(`OPERATION_addcnd_n', `
	define(`ADDSUB',      adds)
	define(`ADDSUBC',      adcs)
	define(`INITCY',      `cmn	r0, #0')
	define(`RETVAL',      `adc	r0, n, #0')
	define(func,	      mpn_addcnd_n)')
ifdef(`OPERATION_subcnd_n', `
	define(`ADDSUB',      subs)
	define(`ADDSUBC',      sbcs)
	define(`INITCY',      `cmp	r0, #0')
	define(`RETVAL',      `adc	r0, n, #0
			      rsb	r0, r0, #1')
	define(func,	      mpn_subcnd_n)')

MULFUNC_PROLOGUE(mpn_addcnd_n mpn_subcnd_n)

ASM_START()
PROLOGUE(func)
	push	{r4-r11}
	ldr	cnd, [sp, #32]

	INITCY				C really only needed for n = 0 (mod 4)

	teq	cnd, #0			C could use this for clearing/setting cy
	mvnne	cnd, #0			C conditionally set to 0xffffffff

	ands	r4, n, #3
	beq	L(top)
	cmp	r4, #2
	bcc	L(b1)
	beq	L(b2)

L(b3):	ldm	vp!, {r4,r5,r6}
	ldm	up!, {r8,r9,r10}
	and	r4, r4, cnd
	and	r5, r5, cnd
	and	r6, r6, cnd
	ADDSUB	r8, r8, r4
	ADDSUBC	r9, r9, r5
	ADDSUBC	r10, r10, r6
	stm	rp!, {r8,r9,r10}
	sub	n, n, #3
	teq	n, #0
	bne	L(top)
	b	L(end)

L(b2):	ldm	vp!, {r4,r5}
	ldm	up!, {r8,r9}
	and	r4, r4, cnd
	and	r5, r5, cnd
	ADDSUB	r8, r8, r4
	ADDSUBC	r9, r9, r5
	stm	rp!, {r8,r9}
	sub	n, n, #2
	teq	n, #0
	bne	L(top)
	b	L(end)

L(b1):	ldr	r4, [vp], #4
	ldr	r8, [up], #4
	and	r4, r4, cnd
	ADDSUB	r8, r8, r4
	str	r8, [rp], #4
	sub	n, n, #1
	teq	n, #0
	beq	L(end)

L(top):	ldm	vp!, {r4,r5,r6,r7}
	ldm	up!, {r8,r9,r10,r11}
	and	r4, r4, cnd
	and	r5, r5, cnd
	and	r6, r6, cnd
	and	r7, r7, cnd
	ADDSUBC	r8, r8, r4
	ADDSUBC	r9, r9, r5
	ADDSUBC	r10, r10, r6
	ADDSUBC	r11, r11, r7
	sub	n, n, #4
	stm	rp!, {r8,r9,r10,r11}
	teq	n, #0
	bne	L(top)

L(end):	RETVAL
	pop	{r4-r11}
	bx	r14
EPILOGUE()
