#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.
#
AC_PREREQ(2.59)
AC_INIT(aria2c, 0.12.0, t-tujikawa@users.sourceforge.net)
AC_CANONICAL_HOST
AC_CANONICAL_SYSTEM
AM_INIT_AUTOMAKE()
AM_PATH_CPPUNIT(1.10.2)
AC_CONFIG_SRCDIR([src/Socket.h])
AC_CONFIG_HEADERS([config.h])

case "$target" in
	*mingw*|*cygwin*)
		WINSOCK_LIBS="-lws2_32"
		AC_SUBST(WINSOCK_LIBS)
		;;
esac

AC_DEFINE_UNQUOTED([TARGET], ["$target"], [Define target-type])

# Set localedir
localedir=${datadir}/locale
AC_SUBST(localedir)

# Checks for arguments.
AC_ARG_WITH([gnutls],  [  --with-gnutls              use gnutls library if installed. Default: yes], [with_gnutls=$withval], [with_gnutls=yes])
AC_ARG_WITH([openssl], [  --with-openssl             use openssl library if installed. Default: yes], [with_openssl=$withval], [with_openssl=yes])
AC_ARG_WITH([libxml2], [  --with-libxml2             use libxml2 library if installed. Default: yes], [with_libxml2=$withval], [with_libxml2=yes])
AC_ARG_WITH([libexpat], [  --with-libexpat            use libexpat library if installed. Default: yes], [with_libexpat=$withval], [with_libexpat=yes])
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

# Setting language choice
AC_LANG([C++])

# Check for GNU library
AC_GNU_SOURCE

# Enable system extensions
AC_USE_SYSTEM_EXTENSIONS

# Checks for libraries.
if test "x$with_libxml2" = "xyes"; then
   AM_PATH_XML2([2.6.24], [have_libxml2=yes])
   if test "x$have_libxml2" = "xyes"; then
      AC_DEFINE([HAVE_LIBXML2], [1], [Define to 1 if you have libxml2.])
   fi
fi

if test "x$with_libexpat" = "xyes" && test "x$have_libxml2" != "xyes"; then
  AM_PATH_LIBEXPAT
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
  AM_CONDITIONAL([ENABLE_MESSAGE_DIGEST], true)
  enable_message_digest=yes
else
  AM_CONDITIONAL([ENABLE_MESSAGE_DIGEST], false)
fi

if test "x$enable_bittorrent" = "xyes" && test "x$enable_message_digest" = "xyes"; then
    AC_DEFINE([ENABLE_BITTORRENT], [1], [Define to 1 if BitTorrent support is enabled.])
    AM_CONDITIONAL([ENABLE_BITTORRENT], true)
else
    enable_bittorrent=no
    AM_CONDITIONAL([ENABLE_BITTORRENT], false)
fi

if (test "x$have_libxml2" = "xyes" || test "x$have_libexpat" = "xyes") && test "x$enable_metalink" = "xyes"; then
    AC_DEFINE([ENABLE_METALINK], [1], [Define to 1 if Metalink support is enabled.])
    AM_CONDITIONAL([ENABLE_METALINK], true)
else
    enable_metalink=no
    AM_CONDITIONAL([ENABLE_METALINK], false)
fi

AM_CONDITIONAL([ENABLE_LIBXML2], [test "x$enable_metalink" = "xyes" && test "x$have_libxml2" = "xyes"])
AM_CONDITIONAL([ENABLE_LIBEXPAT], [test "x$enable_metalink" = "xyes" && test "x$have_libexpat" = "xyes"])

if test "x$have_libcares" = "xyes" || test "x$have_libares" = "xyes"; then
    AC_DEFINE([ENABLE_ASYNC_DNS], [1], [Define to 1 if asynchronous DNS support is enabled.])
    AM_CONDITIONAL([ENABLE_ASYNC_DNS], true)
else
    AM_CONDITIONAL([ENABLE_ASYNC_DNS], false)
fi

# Checks for header files.
AC_FUNC_ALLOCA
#AC_HEADER_STDC
AC_CHECK_HEADERS([winsock2.h ws2tcpip.h argz.h arpa/inet.h fcntl.h inttypes.h io.h langinfo.h libgen.h libintl.h limits.h locale.h malloc.h netdb.h netinet/in.h stddef.h stdint.h stdio_ext.h stdlib.h string.h strings.h sys/param.h sys/socket.h sys/time.h unistd.h])

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
AM_GNU_GETTEXT_VERSION([0.17])
AC_FUNC_ERROR_AT_LINE

case "$target" in
	*mingw*)
		;;
	*)
AC_FUNC_MALLOC
		AC_FUNC_REALLOC
		;;
esac

AC_FUNC_MEMCMP
AC_FUNC_MMAP
AC_FUNC_SELECT_ARGTYPES
AC_FUNC_STAT
AC_FUNC_VPRINTF
AC_CHECK_FUNCS([__argz_count __argz_next __argz_stringify daemon ftruncate getcwd getpagesize inet_ntoa memchr mempcpy memset mkdir munmap nl_langinfo posix_memalign random rmdir select setlocale sigaction sleep socket srandom stpcpy strcasecmp strchr strcspn strdup strerror strstr strtol strtoul unsetenv usleep])

AC_CHECK_FUNCS([basename],
	[AM_CONDITIONAL([HAVE_BASENAME], true)],
	[AM_CONDITIONAL([HAVE_BASENAME], false)])
AC_CHECK_FUNCS([gai_strerror],
	[AM_CONDITIONAL([HAVE_GAI_STRERROR], true)],
 	[AM_CONDITIONAL([HAVE_GAI_STRERROR], false)])
AC_CHECK_FUNCS([getaddrinfo],
	[AM_CONDITIONAL([HAVE_GETADDRINFO], true)],
	[AM_CONDITIONAL([HAVE_GETADDRINFO], false)])
AC_CHECK_FUNCS([gettimeofday],
	[AM_CONDITIONAL([HAVE_GETTIMEOFDAY], true)],
	[AM_CONDITIONAL([HAVE_GETTIMEOFDAY], false)])
AC_CHECK_FUNCS([inet_aton],
	[AM_CONDITIONAL([HAVE_INET_ATON], true)],
	[AM_CONDITIONAL([HAVE_INET_ATON], false)])
AC_CHECK_FUNCS([localtime_r],
	[AM_CONDITIONAL([HAVE_LOCALTIME_R], true)],
	[AM_CONDITIONAL([HAVE_LOCALTIME_R], false)])
AC_CHECK_FUNCS([strptime],
	[AM_CONDITIONAL([HAVE_STRPTIME], true)],
	[AM_CONDITIONAL([HAVE_STRPTIME], false)])
AC_CHECK_FUNCS([timegm],
	[AM_CONDITIONAL([HAVE_TIMEGM], true)],
	[AM_CONDITIONAL([HAVE_TIMEGM], false)])

case "$target" in
	*mingw*)
		dnl defined in ws2tcpip.h, but only if _WIN32_WINNT >= 0x0501
		AM_CONDITIONAL([HAVE_GETADDRINFO], true)
		dnl defined in ws2tcpip.h, but missing in C:\mingw\lib\libws2_32.a
		AM_CONDITIONAL([HAVE_GAI_STRERROR], false)
		;;
esac

AC_CONFIG_FILES([Makefile
		src/Makefile
		test/Makefile
		po/Makefile.in
		m4/Makefile
		intl/Makefile
		doc/Makefile])
AC_OUTPUT

echo " "
echo "Build:          $build"
echo "Target:         $target"
echo "Install prefix: $prefix"
echo "CFLAGS:         $CFLAGS"
echo "CPPFLAGS:       $CPPFLAGS"
echo "LDFLAGS:        $LDFLAGS"
echo "LIBS:           $LIBS"
echo "GnuTLS:         $have_libgnutls"
echo "OpenSSL:        $have_openssl"
echo "LibXML2:        $have_libxml2"
echo "LibExpat:       $have_libexpat"
echo "LibAres:        $have_libares"
echo "LibCares:       $have_libcares"
echo "Bittorrent:     $enable_bittorrent"
echo "Metalink:       $enable_metalink"
