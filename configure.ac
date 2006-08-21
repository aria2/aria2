#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.
#
AC_PREREQ(2.59)
AC_INIT(aria2c, 0.7.1, tujikawa@rednoah.com)
AM_INIT_AUTOMAKE()
AM_PATH_CPPUNIT(1.10.2)
AC_CONFIG_SRCDIR([src/Socket.h])
AC_CONFIG_HEADERS([config.h])

# Set localedir
localedir=${datadir}/locale
AC_SUBST(localedir)

# Checks for arguments.
AC_ARG_WITH([gnutls],  [  --with-gnutls              use gnutls library if installed. Default: yes], [with_gnutls=$withval], [with_gnutls=yes])
AC_ARG_WITH([openssl], [  --with-openssl             use openssl library if installed. Default: yes], [with_openssl=$withval], [with_openssl=yes])
AC_ARG_WITH([libxml2], [  --with-libxml2             use libxml2 library if installed. Default: yes], [with_libxml2=$withval], [with_libxml2=yes])
AC_ARG_WITH([libares], [  --with-libares             use ares library if installed. Default: yes], [with_libares=$withval], [with_libares=yes])
AC_ARG_WITH([libcares], [  --with-libcares            use c-ares library if installed. Default: yes], [with_libcares=$withval], [with_libcares=yes])

AC_ARG_ENABLE([bittorrent], [ --enable-bittorrent        enable BitTorrent support. Default: yes], [enable_bittorrent=$enableval], [enable_bittorrent=yes])
AC_ARG_ENABLE([metalink], [ --enable-metalink          enable Metalink support. Default: yes], [enable_metalink=$enableval], [enable_metalink=yes])

# Checks for programs.
AC_PROG_CXX
AC_PROG_CC
AC_PROG_INSTALL
AC_PROG_RANLIB
AC_PROG_YACC

# Checks for libraries.
if test "x$with_libxml2" = "xyes"; then
   AM_PATH_XML2([2.6.26], [have_libxml2=yes])
   if test "x$have_libxml2" = "xyes"; then
      AC_DEFINE([HAVE_LIBXML2], [1], [Define to 1 if you have libxml2.])
   fi
fi

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
  AM_PATH_OPENSSL
fi

if test "x$with_libcares" = "xyes"; then
  AM_PATH_LIBCARES
fi

if test "x$have_libcares" = "x"; then
  if test "x$with_libares" = "xyes"; then
    AM_PATH_LIBARES
  fi
fi

# Define variables based on the result of the checks for libraries.
if test "x$have_libgnutls" = "xyes" || test "x$have_openssl" = "xyes"; then
  AC_DEFINE([ENABLE_SSL], [1], [Define to 1 if ssl support is enabled.])
fi

if test "x$have_libgcrypt" = "xyes" || test "x$have_openssl" = "xyes"; then
  AC_DEFINE([ENABLE_MESSAGE_DIGEST], [1], [Define to 1 if message digest support is enabled.])
  enable_message_digest=yes
fi

if test "x$enable_bittorrent" = "xyes" && test "x$enable_message_digest" = "xyes"; then
    AC_DEFINE([ENABLE_BITTORRENT], [1], [Define to 1 if BitTorrent support is enabled.])
    AM_CONDITIONAL([ENABLE_BITTORRENT], true)
else
    AM_CONDITIONAL([ENABLE_BITTORRENT], false)
fi

if test "x$have_libxml2" = "xyes" && test "x$enable_metalink" = "xyes"; then
    AC_DEFINE([ENABLE_METALINK], [1], [Define to 1 if Metalink support is enabled.])
    AM_CONDITIONAL([ENABLE_METALINK], true)
else
    AM_CONDITIONAL([ENABLE_METALINK], false)
fi

if test "x$have_libcares" = "xyes" || test "x$have_libares" = "xyes"; then
    AC_DEFINE([ENABLE_ASYNC_DNS], [1], [Define to 1 if asynchronous DNS support is enabled.])
    AM_CONDITIONAL([ENABLE_ASYNC_DNS], true)
else
    AM_CONDITIONAL([ENABLE_ASYNC_DNS], false)
fi

# Checks for header files.
AC_FUNC_ALLOCA
#AC_HEADER_STDC
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
