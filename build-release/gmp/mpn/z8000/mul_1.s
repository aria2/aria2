! Z8000 __gmpn_mul_1 -- Multiply a limb vector with a limb and store
! the result in a second limb vector.

! Copyright 1993, 1994, 1995, 2000 Free Software Foundation, Inc.

! This file is part of the GNU MP Library.

! The GNU MP Library is free software; you can redistribute it and/or modify
! it under the terms of the GNU Lesser General Public License as published by
! the Free Software Foundation; either version 3 of the License, or (at your
! option) any later version.

! The GNU MP Library is distributed in the hope that it will be useful, but
! WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
! or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public
! License for more details.

! You should have received a copy of the GNU Lesser General Public License
! along with the GNU MP Library.  If not, see http://www.gnu.org/licenses/.


! INPUT PARAMETERS
! res_ptr	r7
! s1_ptr	r6
! size		r5
! s2_limb	r4

	unseg
	.text
	even
	global ___gmpn_mul_1
___gmpn_mul_1:
	sub	r2,r2		! zero carry limb
	and	r4,r4
	jr	mi,Lneg

Lpos:	pop	r1,@r6
	ld	r9,r1
	mult	rr8,r4
	and	r1,r1		! shift msb of loaded limb into cy
	jr	mi,Lp		! branch if loaded limb's msb is set
	add	r8,r4		! hi_limb += sign_comp2
Lp:	add	r9,r2		! lo_limb += cy_limb
	xor	r2,r2
	adc	r2,r8
	ld	@r7,r9
	inc	r7,#2
	dec	r5
	jr	ne,Lpos
	ret t

Lneg:	pop	r1,@r6
	ld	r9,r1
	mult	rr8,r4
	add	r8,r1		! hi_limb += sign_comp1
	and	r1,r1
	jr	mi,Ln
	add	r8,r4		! hi_limb += sign_comp2
Ln:	add	r9,r2		! lo_limb += cy_limb
	xor	r2,r2
	adc	r2,r8
	ld	@r7,r9
	inc	r7,#2
	dec	r5
	jr	ne,Lneg
	ret t
