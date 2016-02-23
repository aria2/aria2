; mc88110 __gmpn_mul_1 -- Multiply a limb vector with a single limb and
; store the product in a second limb vector.

; Copyright 1992, 1994, 1995, 2000 Free Software Foundation, Inc.

; This file is part of the GNU MP Library.

; The GNU MP Library is free software; you can redistribute it and/or modify
; it under the terms of the GNU Lesser General Public License as published by
; the Free Software Foundation; either version 3 of the License, or (at your
; option) any later version.

; The GNU MP Library is distributed in the hope that it will be useful, but
; WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
; or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public
; License for more details.

; You should have received a copy of the GNU Lesser General Public License
; along with the GNU MP Library.  If not, see http://www.gnu.org/licenses/.


; INPUT PARAMETERS
; res_ptr	r2
; s1_ptr	r3
; size		r4
; s2_limb	r5

	text
	align	16
	global	___gmpn_mul_1
___gmpn_mul_1:
	; Make S1_PTR and RES_PTR point at the end of their blocks
	; and negate SIZE.
	lda	 r3,r3[r4]
	lda	 r8,r2[r4]		; RES_PTR in r8 since r2 is retval
	subu	 r4,r0,r4

	addu.co	 r2,r0,r0		; r2 = cy = 0

	ld	 r6,r3[r4]
	addu	 r4,r4,1
	mulu.d	 r10,r6,r5
	bcnd.n	 eq0,r4,Lend
	 subu	 r8,r8,8

Loop:	ld	 r6,r3[r4]
	addu.cio r9,r11,r2
	or	 r2,r10,r0		; could be avoided if unrolled
	addu	 r4,r4,1
	mulu.d	 r10,r6,r5
	bcnd.n	 ne0,r4,Loop
	 st	 r9,r8[r4]

Lend:	addu.cio r9,r11,r2
	st	 r9,r8,4
	jmp.n	 r1
	 addu.ci r2,r10,r0
