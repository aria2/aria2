AC_DEFUN([AM_PATH_LIBZ],
[
AC_ARG_WITH([libz-prefix],
            [  --with-libz-prefix=PREFIX  Prefix where libz installed (optional)],
            [libz_prefix=$withval],
            [libz_prefix=""])

if test "x$libz_prefix" = "x"; then
  libz_prefix="/usr"
fi

LIBS_save=$LIBS
CPPFLAGS_save=$CPPFLAGS
PKG_CONFIG_PATH_save=$PKG_CONFIG_PATH

PKG_CONFIG_PATH="$libz_prefix/lib/pkgconfig:$PKG_CONFIG_PATH"
PKG_CHECK_MODULES([ZLIB], [zlib >= 1.2.3], [have_libz=yes], [have_libz=no])

if test "x$have_libz" != "xyes"; then
  libz_prefix_lib=$libz_prefix/lib
  libz_prefix_include=$libz_prefix/include

  LIBS="-L$libz_prefix_lib $LIBS"
  CPPFLAGS="-I$libz_prefix_include $CPPFLAGS"

  AC_CHECK_LIB([z], [zlibVersion], [have_libz=yes])
  if test "x$have_libz" = "xyes"; then
    ZLIB_LIBS="-L$libz_prefix_lib -lz"
    ZLIB_CFLAGS="-I$libz_prefix_include"
  fi
fi

if test "x$have_libz" = "xyes"; then
  AC_DEFINE([HAVE_LIBZ], [1], [Define to 1 if you have libz.])
fi

LIBS=$LIBS_save
CPPFLAGS=$CPPFLAGS_save
PKG_CONFIG_PATH=$PKG_CONFIG_PATH_save
])
