#!/bin/sh -e

if [ -z "$ANDROID_HOME" ]; then
    echo 'No $ANDROID_HOME specified.'
    exit 1
fi
PREFIX=$ANDROID_HOME/usr/local
TOOLCHAIN=$ANDROID_HOME/toolchain
PATH=$TOOLCHAIN/bin:$PATH

HOST=arm-linux-androideabi

CC=$HOST-gcc \
AR=$HOST-ar \
LD=$HOST-ld \
RANLIB=$HOST-ranlib \
STRIP=$HOST-strip \
./configure \
    --prefix=$PREFIX \
    --libdir=$PREFIX/lib \
    --includedir=$PREFIX/include \
    --static
