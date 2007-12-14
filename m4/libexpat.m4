AC_DEFUN([AM_PATH_LIBEXPAT],
[
AC_ARG_WITH([libexpat-prefix],
            [  --with-libexpat-prefix=PREFIX  Prefix where libexpat installed (optional)],
            [libexpat_prefix=$withval],
            [libexpat_prefix=""])

if test "x$libexpat_prefix" = "x"; then
  libexpat_prefix="/usr"
fi

libexpat_prefix_lib=$libexpat_prefix/lib
libexpat_prefix_include=$libexpat_prefix/include

LIBS_save=$LIBS
CPPFLAGS_save=$CPPFLAGS

LIBS="-L$libexpat_prefix_lib $LIBS"
CPPFLAGS="-I$libexpat_prefix_include $CPPFLAGS"

AC_CHECK_LIB([expat], [XML_ParserCreate], [have_libexpat=yes])
if test "x$have_libexpat" = "xyes"; then
    AC_DEFINE([HAVE_LIBEXPAT], [1], [Define to 1 if you have libexpat.])
    LIBEXPAT_LIBS="-L$libexpat_prefix_lib -lexpat"
    LIBEXPAT_CPPFLAGS="-I$libexpat_prefix_include"
    AC_SUBST(LIBEXPAT_LIBS)
    AC_SUBST(LIBEXPAT_CPPFLAGS)
fi

LIBS=$LIBS_save
CPPFLAGS=$CPPFLAGS_save

])
