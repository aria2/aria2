# I960 __gmpn_add_n -- Add two limb vectors of the same length > 0 and store
# sum in a third limb vector.

# Copyright 1995, 2000 Free Software Foundation, Inc.

# This file is part of the GNU MP Library.

# The GNU MP Library is free software; you can redistribute it and/or modify
# it under the terms of the GNU Lesser General Public License as published by
# the Free Software Foundation; either version 3 of the License, or (at your
# option) any later version.

# The GNU MP Library is distributed in the hope that it will be useful, but
# WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
# or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public
# License for more details.

# You should have received a copy of the GNU Lesser General Public License
# along with the GNU MP Library.  If not, see http://www.gnu.org/licenses/.

.text
	.align 4
	.globl ___gmpn_add_n
___gmpn_add_n:
	mov	0,g6		# clear carry-save register
	cmpo	1,0		# clear cy

Loop:	subo	1,g3,g3		# update loop counter
	ld	(g1),g5		# load from s1_ptr
	addo	4,g1,g1		# s1_ptr++
	ld	(g2),g4		# load from s2_ptr
	addo	4,g2,g2		# s2_ptr++
	cmpo	g6,1		# restore cy from g6, relies on cy being 0
	addc	g4,g5,g4	# main add
	subc	0,0,g6		# save cy in g6
	st	g4,(g0)		# store result to res_ptr
	addo	4,g0,g0		# res_ptr++
	cmpobne	0,g3,Loop	# when branch is taken, clears C bit

	mov	g6,g0
	ret
