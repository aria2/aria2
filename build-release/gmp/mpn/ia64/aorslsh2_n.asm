dnl  IA-64 mpn_addlsh1_n/mpn_sublsh1_n -- rp[] = up[] +- (vp[] << 1).

dnl  Contributed to the GNU project by Torbjorn Granlund.

dnl  Copyright 2003, 2004, 2005, 2010 Free Software Foundation, Inc.

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

C           cycles/limb
C Itanium:      3.0
C Itanium 2:    1.5


define(LSH,		2)
define(RSH,		62)

ifdef(`OPERATION_addlsh2_n',`
  define(ADDSUB,       add)
  define(ADDP,         1)
  define(CND,	       ltu)
  define(INCR,	       1)
  define(LIM,	       -1)
  define(func, mpn_addlsh2_n)
')
ifdef(`OPERATION_sublsh2_n',`
  define(ADDSUB,       sub)
  define(CND,	       gtu)
  define(INCR,	       -1)
  define(LIM,	       0)
  define(func, mpn_sublsh2_n)
')


MULFUNC_PROLOGUE(mpn_addlsh2_n mpn_sublsh2_n)

include_mpn(`ia64/aorslshC_n.asm')
