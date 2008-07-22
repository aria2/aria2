#!/bin/sh

exec 2>&1

BUILD_TEST_DIR=/tmp/aria2_build_test

if [ ! -d "$BUILD_TEST_DIR" ]; then
    mkdir "$BUILD_TEST_DIR" \
	|| echo "Failed to create directory $BUILD_TEST_DIR" \
	&& exit -1
fi

echo -n "Starting build test "
echo `date`

# build CONFIGURE_OPTS BIN_SUFFIX DESC
build()
{
    echo -n "new build() started at "
    echo `date`
    echo "*** configure opts=$1"
    BIN_NAME="aria2c_$2"
    if [ -f "$BUILD_TEST_DIR/$BIN_NAME" ]; then
	echo "$BIN_NAME exists, skipping"
	return
    fi
    ./configure $1 \
	&& cp config.log "$BUILD_TEST_DIR/config.log_$2" \
	&& LANG=C make -j2 check > "$BUILD_TEST_DIR/aria2c_$2.log" \
	&& cp src/aria2c "$BUILD_TEST_DIR/aria2c_$2"
}

clear()
{
    for file in `ls $BUILD_TEST_DIR`; do
	rm -f "$BUILD_TEST_DIR/$file";
    done
}

case "$1" in
    clear)
	clear
	;;
    *)
	# Library combinations
	build "--without-gnutls" "openssl"
	build "--without-gnutls --without-openssl" "nossl"
	build "--without-libcares" "nocares"
	build "--without-libxml2" "nolibxml2"
	build "--without-libxml2 --without-libexpat" "noxml"
	build "--without-libz" "nozlib"
	# Feature combinations
	build "--disable-bittorrent" "nobt"
	build "--disable-metalink" "noml"
	build "--disable-bittorrent --disable-metalink" "nobt_noml"
	build "--disable-epoll" "noepoll"
	build "--disable-epoll --without-libcares" "noepoll_nocares"
	;;
esac
