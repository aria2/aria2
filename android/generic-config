#!/bin/sh -e

if [ -z "$ANDROID_HOME" ]; then
    echo 'No $ANDROID_HOME specified.'
    exit 1
fi
PREFIX=$ANDROID_HOME/usr/local
TOOLCHAIN=$ANDROID_HOME/toolchain
PATH=$TOOLCHAIN/bin:$PATH

./configure \
    --host=arm-linux-androideabi \
    --build=`dpkg-architecture -qDEB_BUILD_GNU_TYPE` \
    --prefix=$PREFIX \
    --disable-shared \
    --enable-static
