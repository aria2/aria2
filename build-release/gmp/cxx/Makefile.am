## Process this file with automake to generate Makefile.in

# Copyright 2001, 2002, 2003, 2012 Free Software Foundation, Inc.
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


INCLUDES = -D__GMP_WITHIN_GMPXX -I$(top_srcdir)

if WANT_CXX
noinst_LTLIBRARIES = libcxx.la
endif

libcxx_la_SOURCES = \
  isfuns.cc ismpf.cc ismpq.cc ismpz.cc ismpznw.cc limits.cc \
  osdoprnti.cc osfuns.cc osmpf.cc osmpq.cc osmpz.cc
