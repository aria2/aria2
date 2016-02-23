; Clipper __gmpn_mul_1 -- Multiply a limb vector with a limb and store
; the result in a second limb vector.

; Copyright 1995, 2000 Free Software Foundation, Inc.

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

.text
	.align	16
.globl	___gmpn_mul_1
___gmpn_mul_1:
	subq	$8,sp
	storw	r6,(sp)
	loadw	12(sp),r2
	loadw	16(sp),r3
	loadq	$0,r6		; clear carry limb

.Loop:	loadw	(r1),r4
	mulwux	r3,r4
	addw	r6,r4		; add old carry limb into low product limb
	loadq	$0,r6
	addwc	r5,r6		; propagate cy into high product limb
	storw	r4,(r0)
	addq	$4,r0
	addq	$4,r1
	subq	$1,r2
	brne	.Loop

	movw	r6,r0
	loadw	0(sp),r6
	addq	$8,sp
	ret	sp
