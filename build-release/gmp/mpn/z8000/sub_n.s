! Z8000 __gmpn_sub_n -- Subtract two limb vectors of the same length > 0 and
! store difference in a third limb vector.

! Copyright 1993, 1994, 2000 Free Software Foundation, Inc.

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
! s2_ptr	r5
! size		r4

! If we are really crazy, we can use push to write a few result words
! backwards, using push just because it is faster than reg+disp.  We'd
! then add 2x the number of words written to r7...

	unseg
	.text
	even
	global ___gmpn_sub_n
___gmpn_sub_n:
	pop	r0,@r6
	pop	r1,@r5
	sub	r0,r1
	ld	@r7,r0
	dec	r4
	jr	eq,Lend
Loop:	pop	r0,@r6
	pop	r1,@r5
	sbc	r0,r1
	inc	r7,#2
	ld	@r7,r0
	dec	r4
	jr	ne,Loop
Lend:	ld	r2,r4		! use 0 already in r4
	adc	r2,r2
	ret	t
