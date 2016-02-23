dnl  PowerPC-64 mpn_addlsh1_n and mpn_sublsh1_n.

dnl  Copyright 2003, 2005, 2009, 2010 Free Software Foundation, Inc.

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


define(LSH,		1)
define(RSH,		63)

ifdef(`OPERATION_addlsh1_n',`
  define(ADDSUBC,	addc)
  define(ADDSUBE,	adde)
  define(INITCY,	`addic	$1, r1, 0')
  define(RETVAL,	`addze	r3, $1')
  define(func, mpn_addlsh1_n)
')
ifdef(`OPERATION_sublsh1_n',`
  define(ADDSUBC,	subfc)
  define(ADDSUBE,	subfe)
  define(INITCY,	`addic	$1, r1, -1')
  define(RETVAL,	`subfze	r3, $1
			neg	r3, r3')
  define(func, mpn_sublsh1_n)
')

MULFUNC_PROLOGUE(mpn_addlsh1_n mpn_sublsh1_n)

include_mpn(`powerpc64/mode64/aorslshC_n.asm')
