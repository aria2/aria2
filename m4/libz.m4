AC_DEFUN([AM_PATH_LIBZ],
[
AC_ARG_WITH([libz-prefix],
            [  --with-libz-prefix=PREFIX  Prefix where libz installed (optional)],
            [libz_prefix=$withval],
            [libz_prefix=""])

if test "x$libz_prefix" = "x"; then
  libz_prefix="/usr"
fi

libz_prefix_lib=$libz_prefix/lib
libz_prefix_include=$libz_prefix/include

LIBS_save=$LIBS
CPPFLAGS_save=$CPPFLAGS

LIBS="-L$libz_prefix_lib $LIBS"
CPPFLAGS="-I$libz_prefix_include $CPPFLAGS"

AC_CHECK_LIB([z], [zlibVersion], [have_libz=yes])
if test "x$have_libz" = "xyes"; then
    AC_DEFINE([HAVE_LIBZ], [1], [Define to 1 if you have libz.])
    LIBZ_LIBS="-L$libz_prefix_lib -lz"
    LIBZ_CPPFLAGS="-I$libz_prefix_include"
    AC_SUBST(LIBZ_LIBS)
    AC_SUBST(LIBZ_CPPFLAGS)
fi

LIBS=$LIBS_save
CPPFLAGS=$CPPFLAGS_save

])
