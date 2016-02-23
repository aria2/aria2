; Copyright 1999, 2000 Free Software Foundation, Inc.

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

	.sect .lit,lit
	.text
	.align 4
	.global ___udiv_qrnnd
	.word 0x60000
___udiv_qrnnd:
	mtsr q,lr3
	dividu gr96,lr4,lr5
	mfsr gr116,q
	jmpi lr0
	store 0,0,gr116,lr2
