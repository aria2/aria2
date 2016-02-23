## Process this file with automake to generate Makefile.in

# Copyright 2000, 2001, 2002, 2012 Free Software Foundation, Inc.
#
# This file is part of the GNU MP Library.
#
# The GNU MP Library is free software; you can redistribute it and/or modify
# it under the terms of the GNU Lesser General Public License as published by
# the Free Software Foundation; either version 3 of the License, or (at your
# option) any later version.
#
# The GNU MP Library is distributed in the hope that it will be useful, but
# WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
# or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public
# License for more details.
#
# You should have received a copy of the GNU Lesser General Public License
# along with the GNU MP Library.  If not, see http://www.gnu.org/licenses/.


SUBDIRS = calc expr
EXTRA_DIST = perl primes.h

INCLUDES = -I$(top_srcdir)
LDADD = $(top_builddir)/libgmp.la

qcn_LDADD = $(LDADD) $(LIBM)
primes_LDADD = $(LDADD) $(LIBM)

# None of these programs are built by default, but "make <whatever>" will
# build them once libgmp.la is built.
#
EXTRA_PROGRAMS = factorize isprime pexpr primes qcn

CLEANFILES = $(EXTRA_PROGRAMS)

allprogs: $(EXTRA_PROGRAMS)
	cd calc; $(MAKE) $(AM_MAKEFLAGS) allprogs
	cd expr; $(MAKE) $(AM_MAKEFLAGS) allprogs
