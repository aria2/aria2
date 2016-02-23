dnl  Intel Atom mpn_bdiv_q_1, mpn_pi1_bdiv_q_1 -- schoolbook Hensel
dnl  division by 1-limb divisor, returning quotient only.

dnl  Copyright 2011 Free Software Foundation, Inc.
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

MULFUNC_PROLOGUE(mpn_bdiv_q_1 mpn_pi1_bdiv_q_1)
include_mpn(`x86/pentium/bdiv_q_1.asm')
