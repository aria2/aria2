#!/bin/bash

# Generate an OSX universal binary (32+64b intel) DMG with Installer & DMG
#
# This script is supposed to be run from the aria2 source directory.
# It has to be run on an OSX 10.6 host with Developer Tools installed.
#
# Additionally, Macports must be installed with the following packages:
#
# autoconf autoconf213 autogen +universal automake coreutils +universal
# gettext +universal gmake +universal icu +universal 
# libgcrypt +universal libxml2 +universal m4 +universal
# openssl +universal pkgconfig +universal c-ares +universal zlib +universal
#
# Author: renaud gaudin

# path definitions.
A2_VERSION=`cat config.h |grep "define VERSION" | cut -f 2 -d '"' -`
CURRENT_DIR=`pwd`
PACKAGE_ROOT=${CURRENT_DIR}/aria_build
BUILD_TARGET=${PACKAGE_ROOT}/usr
PACKAGE_RESOURCES=${CURRENT_DIR}/osx_resources
TARGET_NAME=aria2-${A2_VERSION}
PKG_NAME=${TARGET_NAME}.pkg
DMG_NAME=${TARGET_NAME}.dmg
UNINST_NAME="Uninstall aria2.applescript"
DMG_SKEL=aria2_dmg

# build aria2 mostly static
mkdir -p $BUILD_TARGET

export ZLIB_LIBS="/opt/local/lib/libz.a"
export OPENSSL_LIBS="-L/usr/lib -lssl -L/usr/lib -lcrypto"
export LIBCARES_LIBS="/opt/local/lib/libcares.a"
export CPPFLAGS="-I/opt/local/include"
export LDFLAGS="-static-libstdc++"
export LIBS="/opt/local/lib/libintl.a /opt/local/lib/libcrypto.a /opt/local/lib/libiconv.a"
CC="gcc -arch i386 -arch x86_64" CXX="g++ -arch i386 -arch x86_64" CPP="gcc -E" CXXCPP="g++ -E" ./configure --without-libxml2 --without-gnutls --prefix=$BUILD_TARGET --without-libgcrypt --without-libnettle --without-libgmp --enable-bittorrent --enable-metalink --enable-epoll --with-libexpat --with-openssl
make

# install into our target
make install

# create pkg installer
mkdir -p ${PACKAGE_RESOURCES}
cp -av README.html ${PACKAGE_RESOURCES}/Welcome.html
cp -av COPYING ${PACKAGE_RESOURCES}/License.txt
cp -av NEWS ${PACKAGE_RESOURCES}/ReadMe.txt

rm -rf ${PKG_NAME}
/Developer/Applications/Utilities/PackageMaker.app/Contents/MacOS/PackageMaker --root ${PACKAGE_ROOT} --id aria2 --version ${A2_VERSION} --title "aria2" --domain system --resources ${PACKAGE_RESOURCES} --out ${PKG_NAME}

# create uninstaller tool
DEL_STR=`find ${PACKAGE_ROOT}/ -type f | sed -e "s,$PACKAGE_ROOT,sudo rm -f ," | tr '\n' ' ; '`
echo "(do shell script \"mkdir -p /var/db/sudo/$USER; touch /var/db/sudo/$USER\" with administrator privileges) & (do shell script \"${DEL_STR}\")" > "$UNINST_NAME"

# create dmg with installer and uninstall inside
rm -rf ${DMG_SKEL}
mkdir -p ${DMG_SKEL}
cp -av ${PKG_NAME} ${DMG_SKEL}/
cp -av "${UNINST_NAME}" ${DMG_SKEL}/
rm -f ${DMG_NAME}
hdiutil create -megabytes 20 -fs HFS+ -volname aria2 -nospotlight -srcfolder ${DMG_SKEL} ${DMG_NAME}
