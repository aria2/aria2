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

LIBS="-L$libcares_prefix_lib $LIBS"
CPPFLAGS="-I$libcares_prefix_include -Wall $CPPFLAGS"

AC_CHECK_LIB([cares], [ares_init], [have_libcares=yes])

if test "x$have_libcares" != "xyes"; then
    AC_CHECK_LIB([cares], [ares_init], [have_libcares=yes need_librt=yes], [],
                 [-lrt])
fi

if test "x$have_libcares" = "xyes"; then

    AC_MSG_CHECKING([whether ares_host_callback accepts timeouts(c-ares >= 1.5)])
    AC_COMPILE_IFELSE([AC_LANG_PROGRAM([[
    #include <ares.h>

    void callback(void* arg, int status, int timeouts, struct hostent* host);
    ]],
    [[
    ares_channel channel;
    ares_gethostbyname(channel, "foo", 0, callback, 0);
    ]])],
    [have_libcares1_5=yes],
    [have_libcares1_5=no])
    AC_MSG_RESULT([$have_libcares1_5])

    if test "x$have_libcares1_5" = "xyes"; then
        AC_DEFINE([HAVE_LIBCARES1_5], [1], [Define 1 if ares_host_callback accepts timeouts(c-ares >= 1.5)])
    fi

    AC_DEFINE([HAVE_LIBCARES], [1], [Define to 1 if you have libcares.])
    LIBCARES_LIBS="-L$libcares_prefix_lib -lcares"
    if test "x$need_librt" = "xyes"; then
      LIBCARES_LIBS="$LIBCARES_LIBS -lrt"
    fi
    LIBCARES_CPPFLAGS="-I$libcares_prefix_include"
    AC_SUBST(LIBCARES_LIBS)
    AC_SUBST(LIBCARES_CPPFLAGS)
fi

LIBS=$LIBS_save
CPPFLAGS=$CPPFLAGS_save

])
