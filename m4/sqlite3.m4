AC_DEFUN([AM_PATH_SQLITE3],
[
AC_ARG_WITH([sqlite3-prefix],
            [  --with-sqlite3-prefix=PREFIX  Prefix where SQLite3 installed (optional)],
            [sqlite3_prefix=$withval],
            [sqlite3_prefix=""])
if test "x$sqlite3_prefix" = "x"; then
  sqlite3_prefix="/usr"
fi

LIBS_save=$LIBS
CPPFLAGS_save=$CPPFLAGS

PKG_CHECK_MODULES([SQLITE3],[sqlite3],[have_sqlite3=yes],[have_sqlite3=no])

if test "x$have_sqlite3" != "xyes"; then
  AC_MSG_WARN([$SQLITE3_PKG_ERRORS])

  sqlite3_prefix_lib=$sqlite3_prefix/lib
  sqlite3_prefix_include=$sqlite3_prefix/include

  LIBS="-L$sqlite3_prefix_lib $LIBS"
  CPPFLAGS="-I$sqlite3_prefix_include $CPPFLAGS"

  AC_CHECK_LIB([sqlite3], [sqlite3_open], [have_sqlite3=yes])
  if test "x$have_sqlite3" = "xyes"; then
    SQLITE3_LIBS="-L$sqlite3_prefix_lib -lsqlite3"
    SQLITE3_CFLAGS="-I$sqlite3_prefix_include"
  fi
fi
if test "x$have_sqlite3" = "xyes"; then
    AC_CHECK_FUNCS([sqlite3_open_v2])
    AC_DEFINE([HAVE_SQLITE3], [1], [Define to 1 if you have sqlite3.])
    AC_SUBST(SQLITE3_LIBS)
    AC_SUBST(SQLITE3_CFLAGS)
fi

LIBS=$LIBS_save
CPPFLAGS=$CPPFLAGS_save

])
