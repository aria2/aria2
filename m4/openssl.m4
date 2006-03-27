AC_DEFUN([AM_PATH_OPENSSL],
[
AC_ARG_WITH([openssl-prefix],
            [  --with-openssl-prefix=PREFIX  Prefix where OpenSSL installed (optional)],
            [openssl_prefix=$withval],
            [openssl_prefix=""])

if test "x$openssl_prefix" = "x"; then
  openssl_prefix=$prefix
fi

openssl_prefix_lib=$openssl_prefix/lib
openssl_prefix_include=$openssl_prefix/include

LIBS_save=$LIBS
CPPFLAGS_save=$CPPFLAGS

LIBS="-L$openssl_prefix_lib $LIBS"
CPPFLAGS="-I$openssl_prefix_include $CPPFLAGS"

AC_CHECK_LIB([ssl], [SSL_library_init], [have_openssl=yes])

if test "x$have_openssl" = "xyes"; then
  have_openssl=no
  AC_CHECK_LIB([crypto], [main], [have_openssl=yes])
  if test "x$have_openssl" = "xyes"; then
    AC_DEFINE([HAVE_LIBSSL], [1], [Define to 1 if you have openssl.])
    OPENSSL_LIBS="-L$openssl_prefix_lib -lssl -lcrypto"
    OPENSSL_CFLAGS="-I$openssl_prefix_include"
    AC_SUBST(OPENSSL_LIBS)
    AC_SUBST(OPENSSL_CFLAGS)
  fi
fi

LIBS=$LIBS_save
CPPFLAGS=$CPPFLAGS_save
])
