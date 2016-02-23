dnl  Intel Pentium-III mpn_popcount, mpn_hamdist -- population count and
dnl  hamming distance.

dnl  Copyright 2000, 2002, 2004, 2007 Free Software Foundation, Inc.
dnl
dnl  This file is part of the GNU MP Library.
dnl
dnl  The GNU MP Library is free software; you can redistribute it and/or
dnl  modify it under the terms of the GNU Lesser General Public License as
dnl  published by the Free Software Foundation; either version 3 of the
dnl  License, or (at your option) any later version.
dnl
dnl  The GNU MP Library is distributed in the hope that it will be useful,
dnl  but WITHOUT ANY WARRANTY; without even the implied warranty of
dnl  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
dnl  Lesser General Public License for more details.
dnl
dnl  You should have received a copy of the GNU Lesser General Public License
dnl  along with the GNU MP Library.  If not, see http://www.gnu.org/licenses/.

include(`../config.m4')


C			     popcount	     hamdist
C P3 generic			6.5		7
C P3 model 9  (Banias)		?		?
C P3 model 13 (Dothan)		5.75		6


MULFUNC_PROLOGUE(mpn_popcount mpn_hamdist)
include_mpn(`x86/k7/mmx/popham.asm')
