dnl  Alpha mpn_addlsh1_n/mpn_sublsh1_n -- rp[] = up[] +- (vp[] << 1).

dnl  Copyright 2003 Free Software Foundation, Inc.

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

C      cycles/limb
C EV4:    12.5
C EV5:     6.25
C EV6:     4.375 (i.e., worse than separate mpn_lshift and mpn_add_n at 3.875)

C TODO
C  * Write special version for ev6, as this is a slowdown for 100 < n < 2200
C    compared to separate mpn_lshift and mpn_add_n.
C  * Use addq instead of sll for left shift, and similarly cmplt instead of srl
C    for right shift.

dnl  INPUT PARAMETERS
define(`rp',`r16')
define(`up',`r17')
define(`vp',`r18')
define(`n', `r19')

define(`u0', `r8')
define(`u1', `r1')
define(`u2', `r2')
define(`u3', `r3')
define(`v0', `r4')
define(`v1', `r5')
define(`v2', `r6')
define(`v3', `r7')

define(`cy0', `r0')
define(`cy1', `r20')
define(`cy', `r22')
define(`rr', `r24')
define(`ps', `r25')
define(`sl', `r28')

ifdef(`OPERATION_addlsh1_n',`
  define(ADDSUB,       addq)
  define(CARRY,       `cmpult $1,$2,$3')
  define(func, mpn_addlsh1_n)
')
ifdef(`OPERATION_sublsh1_n',`
  define(ADDSUB,       subq)
  define(CARRY,       `cmpult $2,$1,$3')
  define(func, mpn_sublsh1_n)
')

MULFUNC_PROLOGUE(mpn_addlsh1_n mpn_sublsh1_n)

ASM_START()
PROLOGUE(func)
	lda	n, -4(n)
	bis	r31, r31, cy1
	and	n, 3, r1
	beq	r1, $Lb00
	cmpeq	r1, 1, r2
	bne	r2, $Lb01
	cmpeq	r1, 2, r2
	bne	r2, $Lb10
$Lb11:	C n = 3, 7, 11, ...
	ldq	v0, 0(vp)
	ldq	u0, 0(up)
	ldq	v1, 8(vp)
	ldq	u1, 8(up)
	ldq	v2, 16(vp)
	ldq	u2, 16(up)
	lda	vp, 24(vp)
	lda	up, 24(up)
	bge	n, $Loop
	br	r31, $Lcj3
$Lb10:	C n = 2, 6, 10, ...
	bis	r31, r31, cy0
	ldq	v1, 0(vp)
	ldq	u1, 0(up)
	ldq	v2, 8(vp)
	ldq	u2, 8(up)
	lda	rp, -8(rp)
	blt	n, $Lcj2
	ldq	v3, 16(vp)
	ldq	u3, 16(up)
	lda	vp, 48(vp)
	lda	up, 16(up)
	br	r31, $LL10
$Lb01:	C n = 1, 5, 9, ...
	ldq	v2, 0(vp)
	ldq	u2, 0(up)
	lda	rp, -16(rp)
	blt	n, $Lcj1
	ldq	v3, 8(vp)
	ldq	u3, 8(up)
	ldq	v0, 16(vp)
	ldq	u0, 16(up)
	lda	vp, 40(vp)
	lda	up, 8(up)
	lda	rp, 32(rp)
	br	r31, $LL01
$Lb00:	C n = 4, 8, 12, ...
	bis	r31, r31, cy0
	ldq	v3, 0(vp)
	ldq	u3, 0(up)
	ldq	v0, 8(vp)
	ldq	u0, 8(up)
	ldq	v1, 16(vp)
	ldq	u1, 16(up)
	lda	vp, 32(vp)
	lda	rp, 8(rp)
	br	r31, $LL00x
	ALIGN(16)
C 0
$Loop:	sll	v0, 1, sl	C left shift vlimb
	ldq	v3, 0(vp)
C 1
	ADDSUB	u0, sl, ps	C ulimb + (vlimb << 1)
	ldq	u3, 0(up)
C 2
	ADDSUB	ps, cy1, rr	C consume carry from previous operation
	srl	v0, 63, cy0	C carry out #1
C 3
	CARRY(	ps, u0, cy)	C carry out #2
	stq	rr, 0(rp)
C 4
	addq	cy, cy0, cy0	C combine carry out #1 and #2
	CARRY(	rr, ps, cy)	C carry out #3
C 5
	addq	cy, cy0, cy0	C final carry out
	lda	vp, 32(vp)	C bookkeeping
C 6
$LL10:	sll	v1, 1, sl
	ldq	v0, -24(vp)
C 7
	ADDSUB	u1, sl, ps
	ldq	u0, 8(up)
C 8
	ADDSUB	ps, cy0, rr
	srl	v1, 63, cy1
C 9
	CARRY(	ps, u1, cy)
	stq	rr, 8(rp)
C 10
	addq	cy, cy1, cy1
	CARRY(	rr, ps, cy)
C 11
	addq	cy, cy1, cy1
	lda	rp, 32(rp)	C bookkeeping
C 12
$LL01:	sll	v2, 1, sl
	ldq	v1, -16(vp)
C 13
	ADDSUB	u2, sl, ps
	ldq	u1, 16(up)
C 14
	ADDSUB	ps, cy1, rr
	srl	v2, 63, cy0
C 15
	CARRY(	ps, u2, cy)
	stq	rr, -16(rp)
C 16
	addq	cy, cy0, cy0
	CARRY(	rr, ps, cy)
C 17
	addq	cy, cy0, cy0
$LL00x:	lda	up, 32(up)	C bookkeeping
C 18
	sll	v3, 1, sl
	ldq	v2, -8(vp)
C 19
	ADDSUB	u3, sl, ps
	ldq	u2, -8(up)
C 20
	ADDSUB	ps, cy0, rr
	srl	v3, 63, cy1
C 21
	CARRY(	ps, u3, cy)
	stq	rr, -8(rp)
C 22
	addq	cy, cy1, cy1
	CARRY(	rr, ps, cy)
C 23
	addq	cy, cy1, cy1
	lda	n, -4(n)	C bookkeeping
C 24
	bge	n, $Loop

$Lcj3:	sll	v0, 1, sl
	ADDSUB	u0, sl, ps
	ADDSUB	ps, cy1, rr
	srl	v0, 63, cy0
	CARRY(	ps, u0, cy)
	stq	rr, 0(rp)
	addq	cy, cy0, cy0
	CARRY(	rr, ps, cy)
	addq	cy, cy0, cy0

$Lcj2:	sll	v1, 1, sl
	ADDSUB	u1, sl, ps
	ADDSUB	ps, cy0, rr
	srl	v1, 63, cy1
	CARRY(	ps, u1, cy)
	stq	rr, 8(rp)
	addq	cy, cy1, cy1
	CARRY(	rr, ps, cy)
	addq	cy, cy1, cy1

$Lcj1:	sll	v2, 1, sl
	ADDSUB	u2, sl, ps
	ADDSUB	ps, cy1, rr
	srl	v2, 63, cy0
	CARRY(	ps, u2, cy)
	stq	rr, 16(rp)
	addq	cy, cy0, cy0
	CARRY(	rr, ps, cy)
	addq	cy, cy0, cy0

	ret	r31,(r26),1
EPILOGUE()
ASM_END()
