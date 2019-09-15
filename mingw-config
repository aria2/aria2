#!/bin/sh -e

# aria2 - The high speed download utility
#
# Copyright (C) 2012 Tatsuhiro Tsujikawa
#
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 2 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
#
# In addition, as a special exception, the copyright holders give
# permission to link the code of portions of this program with the
# OpenSSL library under certain conditions as described in each
# individual source file, and distribute linked combinations
# including the two.
# You must obey the GNU General Public License in all respects
# for all of the code used other than OpenSSL.  If you modify
# file(s) with this exception, you may extend this exception to your
# version of the file(s), but you are not obligated to do so.  If you
# do not wish to do so, delete this exception statement from your
# version.  If you delete this exception statement from all source
# files in the program, then also delete it here.

# This is the configure script wrapper for cross-compiling MinGW32
# build on Debian Linux using mingw-w64. Some environment variables
# can be adjusted to change build settings:
#
# HOST: cross-compile to build programs to run on HOST. It defaults to
#       i686-w64-mingw32. To build 64 bit binary, specify
#       x86_64-w64-mingw32.
#
# PREFIX: Prefix to the directory where dependent libraries are
#       installed.  It defaults to /usr/local/$HOST. -I$PREFIX/include
#       will be added to CPPFLAGS. -L$PREFIX/lib will be added to
#       LDFLAGS. $PREFIX/lib/pkgconfig will be set to
#       PKG_CONFIG_LIBDIR.
#
# In this configuration, the following dependent libraries are used:
#
# * c-ares
# * gmp
# * expat
# * sqlite3
# * zlib
# * libssh2
# * cppunit

test -z "$HOST" && HOST=i686-w64-mingw32
test -z "$PREFIX" && PREFIX=/usr/local/$HOST

./configure \
    --host=$HOST \
    --prefix=$PREFIX \
    --without-included-gettext \
    --disable-nls \
    --with-libcares \
    --without-gnutls \
    --without-openssl \
    --with-sqlite3 \
    --without-libxml2 \
    --with-libexpat \
    --with-libz \
    --with-libgmp \
    --with-libssh2 \
    --without-libgcrypt \
    --without-libnettle \
    --with-cppunit-prefix=$PREFIX \
    ARIA2_STATIC=yes \
    CPPFLAGS="-I$PREFIX/include" \
    LDFLAGS="-L$PREFIX/lib" \
    PKG_CONFIG="/usr/bin/pkg-config" \
    PKG_CONFIG_PATH="$PREFIX/lib/pkgconfig"
