AC_DEFUN([AM_PATH_LIBEXPAT],
[
LIBS_save=$LIBS
CPPFLAGS_save=$CPPFLAGS

LIBS="-lexpat $LIBS"
AC_CHECK_LIB([expat], [XML_ParserCreate], [have_libexpat=yes])
if test "x$have_libexpat" = "xyes"; then
    AC_DEFINE([HAVE_LIBEXPAT], [1], [Define to 1 if you have libexpat.])
    EXPAT_LIBS=-lexpat
    EXPAT_CFLAGS=
fi

LIBS=$LIBS_save
CPPFLAGS=$CPPFLAGS_save

])
