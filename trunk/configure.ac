#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.
#
AC_PREREQ(2.59)
AC_INIT(aria2c, 0.5.0, tujikawa@rednoah.com)
AM_INIT_AUTOMAKE()
AM_PATH_CPPUNIT(1.10.2)
AC_CONFIG_SRCDIR([src/Socket.h])
AC_CONFIG_HEADERS([config.h])

# Set localedir
localedir=${datadir}/locale
AC_SUBST(localedir)

# Checks for arguments.
AC_ARG_WITH([gnutls],  [  --with-gnutls              use gnutls library if installed. Default: yes], [with_gnutls=$enableval], [with_gnutls=yes])
AC_ARG_WITH([openssl], [  --with-openssl             use openssl library if installed. Default: yes], [with_openssl=$enableval], [with_openssl=yes])


# Checks for programs.
AC_PROG_CXX
AC_PROG_CC
AC_PROG_INSTALL
AC_PROG_RANLIB
AC_PROG_YACC

# Checks for libraries.
if test "x$with_gnutls" = "xyes"; then
  AM_PATH_LIBGNUTLS([1.2.9], [have_libgnutls=yes])
  if test "x$have_libgnutls" = "xyes"; then
    AC_DEFINE([HAVE_LIBGNUTLS], [1], [Define to 1 if you have libgnutls.])
  fi
fi

if test "x$have_libgnutls" = "xyes"; then
  AM_PATH_LIBGCRYPT([1.2.2], [have_libgcrypt=yes])
  if test "x$have_libgcrypt" = "xyes"; then
    AC_DEFINE([HAVE_LIBGCRYPT], [1], [Define to 1 if you have libgcrypt.])
  fi
fi

if test "x$with_openssl" = "xyes" && test "x$have_libgnutls" != "xyes"; then
#  AC_CHECK_LIB([ssl], [SSL_library_init], [have_openssl=yes])
#  if test "x$have_openssl" = "xyes"; then
#    AC_DEFINE([HAVE_LIBSSL], [1], [Define to 1 if you have openssl.])
#    OPENSSL_LIBS="-lssl"
#    OPENSSL_CFLAGS=""
#    AC_SUBST(OPENSSL_LIBS)
#    AC_SUBST(OPENSSL_CFLAGS)
#  fi
  AM_PATH_OPENSSL
fi

if test "x$have_libgnutls" = "xyes" || test "x$have_openssl" = "xyes"; then
  AC_DEFINE([ENABLE_SSL], [1], [Define to 1 if ssl support is enabled.])
fi

if test "x$have_libgcrypt" = "xyes" || test "x$have_openssl" = "xyes"; then
  AC_DEFINE([ENABLE_SHA1DIGEST], [1], [Define to 1 if SHA1 Digest support is enabled.])
  AC_DEFINE([ENABLE_BITTORRENT], [1], [Define to 1 if BitTorrent support is enabled.])
fi


# Checks for header files.
AC_FUNC_ALLOCA
AC_HEADER_STDC
AC_CHECK_HEADERS([argz.h arpa/inet.h fcntl.h inttypes.h langinfo.h libintl.h limits.h locale.h malloc.h netdb.h netinet/in.h stddef.h stdint.h stdio_ext.h stdlib.h string.h strings.h sys/param.h sys/socket.h sys/time.h unistd.h])

# Checks for typedefs, structures, and compiler characteristics.
AC_HEADER_STDBOOL
AC_C_CONST
AC_C_INLINE
AC_TYPE_SIZE_T
AC_HEADER_TIME
AC_STRUCT_TM
AC_C_VOLATILE

# Checks for library functions.
AM_GNU_GETTEXT
AM_GNU_GETTEXT_VERSION(0.12.1)
AC_FUNC_ERROR_AT_LINE

AC_FUNC_MALLOC
AC_FUNC_MEMCMP
AC_FUNC_MMAP
AC_FUNC_REALLOC
AC_FUNC_SELECT_ARGTYPES
AC_FUNC_STAT
AC_FUNC_VPRINTF
AC_CHECK_FUNCS([__argz_count __argz_next __argz_stringify ftruncate getcwd getpagesize gettimeofday inet_ntoa memchr mempcpy memset mkdir munmap nl_langinfo rmdir select setlocale socket stpcpy strcasecmp strchr strcspn strdup strerror strstr strtol strtoul])
AC_CONFIG_FILES([Makefile
		src/Makefile
		test/Makefile
		po/Makefile.in
		m4/Makefile
		intl/Makefile])
AC_OUTPUT
