AC_DEFUN([AM_PATH_LIBARES],
[
AC_ARG_WITH([libares-prefix],
            [  --with-libares-prefix=PREFIX  Prefix where libares installed (optional)],
            [libares_prefix=$withval],
            [libares_prefix=""])

if test "x$libares_prefix" = "x"; then
  libares_prefix="/usr"
fi

libares_prefix_lib=$libares_prefix/lib
libares_prefix_include=$libares_prefix/include

LIBS_save=$LIBS
CPPFLAGS_save=$CPPFLAGS

LIBS="-L$libares_prefix_lib $LIBS"
CPPFLAGS="-I$libares_prefix_include $CPPFLAGS"

AC_CHECK_LIB([ares], [ares_init], [have_libares=yes])
if test "x$have_libares" = "xyes"; then
    AC_DEFINE([HAVE_LIBARES], [1], [Define to 1 if you have libares.])
    LIBARES_LIBS="-L$libares_prefix_lib -lares"
    LIBARES_CPPFLAGS="-I$libares_prefix_include"
    AC_SUBST(LIBARES_LIBS)
    AC_SUBST(LIBARES_CPPFLAGS)
fi

LIBS=$LIBS_save
CPPFLAGS=$CPPFLAGS_save

])
