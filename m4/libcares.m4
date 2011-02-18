AC_DEFUN([AM_PATH_LIBCARES],
[
AC_ARG_WITH([libcares-prefix],
            [  --with-libcares-prefix=PREFIX  Prefix where libcares installed (optional)],
            [libcares_prefix=$withval],
            [libcares_prefix=""])

if test "x$libcares_prefix" = "x"; then
  libcares_prefix="/usr"
fi

libcares_prefix_lib=$libcares_prefix/lib
libcares_prefix_include=$libcares_prefix/include

LIBS_save=$LIBS
CPPFLAGS_save=$CPPFLAGS
PKG_CONFIG_PATH_save=$PKG_CONFIG_PATH

PKG_CONFIG_PATH="$libcares_prefix/lib/pkgconfig:$PKG_CONFIG_PATH"
PKG_CHECK_MODULES([LIBCARES], [libcares >= 1.7.0], [have_libcares=yes],
                  [have_libcares=no])

if test "x$have_libcares" = "xyes"; then
  LIBS="$LIBCARES_LIBS $LIBS"
  CPPFLAGS="$LIBCARES_CFLAGS $CPPFLAGS"
fi

if test "x$have_libcares" != "xyes"; then
  LIBS="-L$libcares_prefix_lib $LIBS"
  CPPFLAGS="-I$libcares_prefix_include -Wall $CPPFLAGS"

  AC_CHECK_LIB([cares], [ares_init], [have_libcares=yes])

  if test "x$have_libcares" != "xyes"; then
    AC_CHECK_LIB([cares], [ares_init], [have_libcares=yes need_librt=yes], [],
                 [-lrt])
  fi
  if test "x$have_libcares" = "xyes"; then
    LIBCARES_LIBS="-L$libcares_prefix_lib -lcares"
    if test "x$need_librt" = "xyes"; then
      LIBCARES_LIBS="$LIBCARES_LIBS -lrt"
    fi
    LIBCARES_CFLAGS="-I$libcares_prefix_include"

    LIBS="$LIBCARES_LIBS $LIBS_save"
    CPPFLAGS="$LIBCARES_CFLAGS $CPPFLAGS_save"
  fi
fi

if test "x$have_libcares" = "xyes"; then
  AC_DEFINE([HAVE_LIBCARES], [1], [Define to 1 if you have libcares.])
  AC_CHECK_TYPES([ares_addr_node], [], [], [[#include <ares.h>]])
  AC_CHECK_FUNCS([ares_set_servers])
fi

LIBS=$LIBS_save
CPPFLAGS=$CPPFLAGS_save
PKG_CONFIG_PATH=$PKG_CONFIG_PATH_save
])
