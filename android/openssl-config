#!/bin/sh -e

if [ -z "$ANDROID_HOME" ]; then
    echo 'No $ANDROID_HOME specified.'
    exit 1
fi
PREFIX=$ANDROID_HOME/usr/local
TOOLCHAIN=$ANDROID_HOME/toolchain
PATH=$TOOLCHAIN/bin:$PATH

export CROSS_COMPILE=$TOOLCHAIN/bin/arm-linux-androideabi-
./Configure --prefix=$PREFIX android
