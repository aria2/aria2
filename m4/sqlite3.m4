AC_DEFUN([AM_PATH_SQLITE3],
[
AC_ARG_WITH([sqlite3-prefix],
            [  --with-sqlite3-prefix=PREFIX  Prefix where SQLite3 installed (optional)],
            [sqlite3_prefix=$withval],
            [sqlite3_prefix=""])
if test "x$sqlite3_prefix" = "x"; then
  sqlite3_prefix="/usr"
fi

sqlite3_prefix_lib=$sqlite3_prefix/lib
sqlite3_prefix_include=$sqlite3_prefix/include

LIBS_save=$LIBS
CPPFLAGS_save=$CPPFLAGS

LIBS="-L$sqlite3_prefix_lib $LIBS"
CPPFLAGS="-I$sqlite3_prefix_include $CPPFLAGS"

AC_CHECK_LIB([sqlite3], [sqlite3_open], [have_sqlite3=yes])
if test "x$have_sqlite3" = "xyes"; then
    AC_DEFINE([HAVE_SQLITE3], [1], [Define to 1 if you have sqlite3.])
    SQLITE3_LIBS="-L$sqlite3_prefix_lib -lsqlite3"
    SQLITE3_CPPFLAGS="-I$sqlite3_prefix_include"
    AC_SUBST(SQLITE3_LIBS)
    AC_SUBST(SQLITE3_CPPFLAGS)
fi

LIBS=$LIBS_save
CPPFLAGS=$CPPFLAGS_save

])
