; 29000 mpn_add_n -- Add two limb vectors of the same length > 0 and store
; sum in a third limb vector.

; Copyright 1992, 1994, 2000 Free Software Foundation, Inc.

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
; res_ptr	lr2
; s1_ptr	lr3
; s2_ptr	lr4
; size		lr5

; We use the loadm/storem instructions and operate on chunks of 8
; limbs/per iteration, until less than 8 limbs remain.

; The 29k has no addition or subtraction instructions that doesn't
; affect carry, so we need to save and restore that as soon as we
; adjust the pointers.  gr116 is used for this purpose.  Note that
; gr116==0 means that carry should be set.

	.sect .lit,lit
	.text
	.align	4
	.global	___gmpn_add_n
	.word	0x60000
___gmpn_add_n:
	srl	gr117,lr5,3
	sub	gr118,gr117,1
	jmpt	gr118,Ltail
	 constn	gr116,-1		; init cy reg
	sub	gr117,gr117,2		; count for jmpfdec

; Main loop working 8 limbs/iteration.
Loop:	mtsrim	cr,(8-1)
	loadm	0,0,gr96,lr3
	add	lr3,lr3,32
	mtsrim	cr,(8-1)
	loadm	0,0,gr104,lr4
	add	lr4,lr4,32

	subr	gr116,gr116,0		; restore carry
	addc	gr96,gr96,gr104
	addc	gr97,gr97,gr105
	addc	gr98,gr98,gr106
	addc	gr99,gr99,gr107
	addc	gr100,gr100,gr108
	addc	gr101,gr101,gr109
	addc	gr102,gr102,gr110
	addc	gr103,gr103,gr111
	subc	gr116,gr116,gr116	; gr116 = not(cy)

	mtsrim	cr,(8-1)
	storem	0,0,gr96,lr2
	jmpfdec	gr117,Loop
	 add	lr2,lr2,32

; Code for the last up-to-7 limbs.
; This code might look very strange, but it's hard to write it
; differently without major slowdown.

	and	lr5,lr5,(8-1)
Ltail:	sub	gr118,lr5,1		; count for CR
	jmpt	gr118,Lend
	 sub	gr117,lr5,2		; count for jmpfdec

	mtsr	cr,gr118
	loadm	0,0,gr96,lr3
	mtsr	cr,gr118
	loadm	0,0,gr104,lr4

	subr	gr116,gr116,0		; restore carry

	jmpfdec	gr117,L1
	 addc	gr96,gr96,gr104
	jmp	Lstore
	 mtsr	cr,gr118
L1:	jmpfdec	gr117,L2
	 addc	gr97,gr97,gr105
	jmp	Lstore
	 mtsr	cr,gr118
L2:	jmpfdec	gr117,L3
	 addc	gr98,gr98,gr106
	jmp	Lstore
	 mtsr	cr,gr118
L3:	jmpfdec	gr117,L4
	 addc	gr99,gr99,gr107
	jmp	Lstore
	 mtsr	cr,gr118
L4:	jmpfdec	gr117,L5
	 addc	gr100,gr100,gr108
	jmp	Lstore
	 mtsr	cr,gr118
L5:	jmpfdec	gr117,L6
	 addc	gr101,gr101,gr109
	jmp	Lstore
	 mtsr	cr,gr118
L6:	addc	gr102,gr102,gr110

Lstore:	storem	0,0,gr96,lr2
	subc	gr116,gr116,gr116	; gr116 = not(cy)

Lend:	jmpi	lr0
	 add	gr96,gr116,1
