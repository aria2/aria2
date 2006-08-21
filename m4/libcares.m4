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
CPPFLAGS="-I$libcares_prefix_include $CPPFLAGS"

AC_CHECK_LIB([cares], [ares_init], [have_libcares=yes])
if test "x$have_libcares" = "xyes"; then
    AC_DEFINE([HAVE_LIBCARES], [1], [Define to 1 if you have libcares.])
    LIBCARES_LIBS="-L$libcares_prefix_lib -lcares"
    LIBCARES_CPPFLAGS="-I$libcares_prefix_include"
    AC_SUBST(LIBCARES_LIBS)
    AC_SUBST(LIBCARES_CPPFLAGS)
fi

LIBS=$LIBS_save
CPPFLAGS=$CPPFLAGS_save

])
