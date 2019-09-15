#!/bin/sh

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

if [ -z "$ANDROID_HOME" ]; then
    echo 'No $ANDROID_HOME specified.'
    exit 1
fi
if [ -z "$NDK" ]; then
    echo 'No $NDK specified.'
    exit 1
fi
PREFIX=$ANDROID_HOME/usr/local
TOOLCHAIN=$NDK/toolchains/llvm/prebuilt/linux-x86_64

./configure \
    --host=aarch64-linux-android \
    --build=`dpkg-architecture -qDEB_BUILD_GNU_TYPE` \
    --disable-nls \
    --without-gnutls \
    --with-openssl \
    --without-sqlite3 \
    --without-libxml2 \
    --with-libexpat \
    --with-libcares \
    --with-libz \
    --with-libssh2 \
    AR=$TOOLCHAIN/bin/aarch64-linux-android-ar \
    AS=$TOOLCHAIN/bin/aarch64-linux-android-as \
    CC=$TOOLCHAIN/bin/aarch64-linux-android29-clang \
    CXX=$TOOLCHAIN/bin/aarch64-linux-android29-clang++ \
    LD=$TOOLCHAIN/bin/aarch64-linux-android-ld \
    RANLIB=$TOOLCHAIN/bin/aarch64-linux-android-ranlib \
    STRIP=$TOOLCHAIN/bin/aarch64-linux-android-strip \
    CXXFLAGS="-Os -g" \
    CFLAGS="-Os -g" \
    CPPFLAGS="-fPIE" \
    LDFLAGS="-fPIE -pie -L$PREFIX/lib -static-libstdc++" \
    PKG_CONFIG_LIBDIR="$PREFIX/lib/pkgconfig"
