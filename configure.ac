#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.
#
AC_PREREQ([2.67])
LT_PREREQ([2.2.6])
AC_INIT([aria2],[1.17.1],[t-tujikawa@users.sourceforge.net],[aria2],[http://aria2.sourceforge.net/])
AC_USE_SYSTEM_EXTENSIONS
LT_INIT([disable-static])
dnl See versioning rule:
dnl  http://www.gnu.org/software/libtool/manual/html_node/Updating-version-info.html
AC_SUBST(LT_CURRENT, 0)
AC_SUBST(LT_REVISION, 0)
AC_SUBST(LT_AGE, 0)

AC_CONFIG_MACRO_DIR([m4])
AC_CANONICAL_HOST
AC_CANONICAL_TARGET
AM_INIT_AUTOMAKE()
AM_PATH_CPPUNIT(1.10.2)
AC_CONFIG_SRCDIR([src/a2io.h])
AC_CONFIG_HEADERS([config.h])

case "$host" in
  *mingw*)
    win_build=yes
    LIBS="$LIBS -lws2_32 -lwsock32 -lgdi32 -lwinmm -liphlpapi -lpsapi"
    ;;
esac

AC_DEFINE_UNQUOTED([TARGET], ["$target"], [Define target-type])

# Checks for arguments.
ARIA2_ARG_WITHOUT([libuv])
ARIA2_ARG_WITHOUT([appletls])
ARIA2_ARG_WITHOUT([gnutls])
ARIA2_ARG_WITHOUT([libnettle])
ARIA2_ARG_WITHOUT([libgmp])
ARIA2_ARG_WITHOUT([libgcrypt])
ARIA2_ARG_WITHOUT([openssl])
ARIA2_ARG_WITHOUT([sqlite3])
ARIA2_ARG_WITHOUT([libxml2])
ARIA2_ARG_WITHOUT([libexpat])
ARIA2_ARG_WITHOUT([libcares])
ARIA2_ARG_WITHOUT([libz])
ARIA2_ARG_WITH([tcmalloc])
ARIA2_ARG_WITH([jemalloc])

ARIA2_ARG_DISABLE([bittorrent])
ARIA2_ARG_DISABLE([metalink])
ARIA2_ARG_DISABLE([epoll])
ARIA2_ARG_ENABLE([libaria2])

AC_ARG_WITH([ca-bundle],
  AS_HELP_STRING([--with-ca-bundle=FILE],[Use FILE as default CA bundle.]),
  [ca_bundle=$withval], [ca_bundle=""])

AC_ARG_WITH([bashcompletiondir],
  AS_HELP_STRING([--with-bashcompletiondir=DIR],
                 [Directory to install bash_completion file]),
  [bashcompletiondir=$withval], [bashcompletiondir=$docdir/bash_completion])

AC_ARG_VAR([ARIA2_STATIC], [Set 'yes' to build a statically linked aria2])

# Checks for programs.
AC_PROG_CXX
AC_PROG_CC
AC_PROG_INSTALL
AC_PROG_MKDIR_P
AC_PROG_YACC
AM_PROG_AS

AC_CHECK_TOOL([AR], [ar], [:])
if test "x$AR" = "x:"; then
  AC_MSG_FAILURE([ar is not found in the system.])
fi
AC_SUBST([AR])

AC_PATH_PROG([A2X], [a2x])
AC_SUBST([A2X])
AM_CONDITIONAL([HAVE_A2X], [ test "x$A2X" != "x" ])

AC_PATH_PROG([ASCIIDOC], [asciidoc])
AC_SUBST([ASCIIDOC])
AM_CONDITIONAL([HAVE_ASCIIDOC], [ test "x$ASCIIDOC" != "x" ])

AC_PATH_PROGS([RST2HTML], [rst2html.py rst2html])
AC_SUBST([RST2HTML])
AM_CONDITIONAL([HAVE_RST2HTML], [ test "x$RST2HTML" != "x" ])

# Setting language choice
AC_LANG([C++])

# Check pkg-config is available
PKG_PROG_PKG_CONFIG([0.20])

# Check static build is requested
if test "x$ARIA2_STATIC" = "xyes"; then
  case "$host" in
    i686*mingw*)
      dnl Define _USE_32BIT_TIME_T because 32bit library of MinGW-w64
      dnl does not implement many 64bit version functions.
      CPPFLAGS="-D_USE_32BIT_TIME_T $CPPFLAGS"
    ;;
  esac
  # Make pkg-config produce static linking variables
  PKG_CONFIG="$PKG_CONFIG --static"
else
  dnl Make variable empty to avoid confusion
  ARIA2_STATIC=
fi

# Checks for libraries.

# Check availability of libz
if test "x$with_libz" = "xyes"; then
  PKG_CHECK_MODULES([ZLIB], [zlib >= 1.2.3], [have_zlib=yes], [have_zlib=no])
  if test "x$have_zlib" = "xyes"; then
    LIBS="$ZLIB_LIBS $LIBS"
    CPPFLAGS="$ZLIB_CFLAGS $CPPFLAGS"
  else
    AC_MSG_WARN([$ZLIB_PKG_ERRORS])
    AC_CHECK_LIB([z], [zlibVersion], [have_zlib=yes], [have_zlib=no])
    if test "x$have_zlib" = "xyes"; then
      LIBS="-lz $LIBS"
    elif test "x$with_libz_requested" = "xyes"; then
      ARIA2_DEP_NOT_MET([libz])
    fi
  fi
  if test "x$have_zlib" = "xyes"; then
    AC_DEFINE([HAVE_ZLIB], [1], [Define to 1 if you have zlib.])
    # Android NDK arch-mips contains gzbuffer symbol but it is missing
    # in zlib.h
    AC_CHECK_DECL([gzbuffer], [have_decl_gzbuffer=yes], [],
                  [[#include <zlib.h>]])
    if test "x$have_decl_gzbuffer" = "xyes"; then
      AC_CHECK_FUNC([gzbuffer])
    fi
    AC_CHECK_FUNCS([gzsetparams])
  fi
fi

if test "x$with_libuv" = "xyes"; then
  case "$host" in
    *mingw*|*msvc*)
      old_CPPFLAGS=$CPPFLAGS
      CPPFLAGS="$CPPFLAGS -D_WIN32_WINNT=0x0600"
      AC_SEARCH_LIBS([uv_poll_init_socket], [uv], [
                      AC_CHECK_HEADER([uv.h], [have_libuv=yes], [have_libuv=no])
                      break;
                      ], [have_libuv=no])
      if test "x$have_libuv" != "xyes"; then
        CPPFLAGS=$old_CPPFLAGS
      fi
    ;;

    *darwin*)
      old_LDFLAGS=$LDFLAGS
      LDFLAGS="$LDFLAGS -framework Foundation -framework CoreServices -framework ApplicationServices"
      old_LIBS=$LIBS
      LIBS="$LIBS -lm"
      AC_SEARCH_LIBS([uv_poll_init_socket], [uv], [
                      AC_CHECK_HEADER([uv.h], [have_libuv=yes], [have_libuv=no])
                      break;
                      ], [have_libuv=no])
      if test "x$have_libuv" != "xyes"; then
        LDFLAGS=$old_LDFLAGS
        LIBS=$old_LIBS
      fi
      ;;

    *)
      dnl Yeah, sucks that luv does not bring a pkg-config or config-tool
      AC_MSG_CHECKING([for libuv])
      for combo in "" "-lrt" "-ldl -lrt" "-ldl -lrt -lpthread" "-lkvm"; do
        old_LIBS=$LIBS
        LIBS="-luv $combo $LIBS -lm"
        AC_LINK_IFELSE([AC_LANG_SOURCE([
extern "C" int uv_poll_init_socket(void);
int main() { return uv_poll_init_socket(); }
                       ])], [
                        AC_MSG_RESULT(-luv $combo -lm)
                        AC_CHECK_HEADER([uv.h], [have_libuv=yes], [have_libuv=no])
                        break;
                        ], [have_libuv=no])
        if test "x$have_libuv" = "xyes"; then
          break;
        else
          LIBS=$old_LIBS
        fi
      done
      if test "x$have_libuv" != "xyes"; then
        AC_MSG_RESULT("no")
      fi
    ;;
  esac

  if test "x$have_libuv" = "xyes"; then
    AC_DEFINE([HAVE_LIBUV], [1], [Define to 1 if you have libuv.])
  elif test "x$with_libuv_requested" = "xyes"; then
    ARIA2_DEP_NOT_MET([libuv])
  fi
fi
AM_CONDITIONAL([HAVE_LIBUV], [test "x$have_libuv" = "xyes"])

if test "x$with_libxml2" = "xyes"; then
  AM_PATH_XML2([2.6.24], [have_libxml2=yes])
  if test "x$have_libxml2" = "xyes"; then
    AC_DEFINE([HAVE_LIBXML2], [1], [Define to 1 if you have libxml2.])
    LIBS="$XML_LIBS $LIBS"
    CPPFLAGS="$XML_CPPFLAGS $CPPFLAGS"
  elif test "x$with_libxml2_requested" = "xyes"; then
    ARIA2_DEP_NOT_MET([libxml2])
  fi
fi

if test "x$with_libexpat" = "xyes" && test "x$have_libxml2" != "xyes"; then
  AM_PATH_LIBEXPAT
  if test "x$have_libexpat" = "xyes"; then
    LIBS="$EXPAT_LIBS $LIBS"
    CPPFLAGS="$EXPAT_CFLAGS $CPPFLAGS"
  elif test "x$with_libexpat_requested" = "xyes"; then
    ARIA2_DEP_NOT_MET([libexpat])
  fi
fi

if test "x$with_sqlite3" = "xyes"; then
  PKG_CHECK_MODULES([SQLITE3],[sqlite3],[have_sqlite3=yes],[have_sqlite3=no])
  if test "x$have_sqlite3" = "xyes"; then
    AC_DEFINE([HAVE_SQLITE3], [1], [Define to 1 if you have sqlite3.])
    LIBS="$SQLITE3_LIBS $LIBS"
    CPPFLAGS="$SQLITE3_CFLAGS $CPPFLAGS"
    AC_CHECK_FUNCS([sqlite3_open_v2])
  else
    AC_MSG_WARN([$SQLITE3_PKG_ERRORS])
    if test "x$with_sqlite3_requested" = "xyes"; then
      ARIA2_DEP_NOT_MET([sqlite3])
    fi
  fi
fi

case "$host" in
  *darwin*)
    have_osx="yes"
  ;;
esac

if test "x$with_appletls" = "xyes"; then
  AC_MSG_CHECKING([whether to enable Mac OS X native SSL/TLS])
  if test "x$have_osx" = "xyes"; then
    AC_DEFINE([HAVE_APPLETLS], [1], [Define to 1 if you have Apple TLS])
    LDFLAGS="$LDFLAGS -framework CoreFoundation -framework Security"
    have_appletls="yes"
    AC_MSG_RESULT(yes)
  else
    AC_MSG_RESULT(no)
    if test "x$with_appletls_requested" = "xyes"; then
      ARIA2_DEP_NOT_MET([appletls])
    fi
  fi
fi

if test "x$with_gnutls" = "xyes" && test "x$have_appletls" != "xyes"; then
  # gnutls >= 2.8 doesn't have libgnutls-config anymore. We require
  # 2.2.0 because we use gnutls_priority_set_direct()
  PKG_CHECK_MODULES([LIBGNUTLS], [gnutls >= 2.2.0],
                    [have_libgnutls=yes], [have_libgnutls=no])
  if test "x$have_libgnutls" = "xyes"; then
    AC_DEFINE([HAVE_LIBGNUTLS], [1], [Define to 1 if you have libgnutls.])
    LIBS="$LIBGNUTLS_LIBS $LIBS"
    CPPFLAGS="$LIBGNUTLS_CFLAGS $CPPFLAGS"
    AC_CHECK_FUNCS([gnutls_certificate_set_x509_system_trust])
  else
    AC_MSG_WARN([$LIBGNUTLS_PKG_ERRORS])
    if test "x$with_gnutls_requested" = "xyes"; then
      ARIA2_DEP_NOT_MET([gnutls])
    fi
  fi
fi

if test "x$with_openssl" = "xyes" && test "x$have_appletls" != "xyes" && test "x$have_libgnutls" != "xyes"; then
  PKG_CHECK_MODULES([OPENSSL], [openssl >= 0.9.8],
                    [have_openssl=yes], [have_openssl=no])
  if test "x$have_openssl" = "xyes"; then
    AC_DEFINE([HAVE_OPENSSL], [1], [Define to 1 if you have openssl.])
    LIBS="$OPENSSL_LIBS $LIBS"
    CPPFLAGS="$OPENSSL_CFLAGS $CPPFLAGS"

    AC_CHECK_FUNCS([EVP_DigestInit_ex], [have_digestinit_ex=yes])
    if test "x$have_digestinit_ex" = "x"; then
      AC_DEFINE([HAVE_OLD_OPENSSL], [1], [Define to 1 if you have old openssl.])
    fi
    AC_CHECK_FUNCS([EVP_sha224])
    AC_CHECK_FUNCS([EVP_sha256])
    AC_CHECK_FUNCS([EVP_sha384])
    AC_CHECK_FUNCS([EVP_sha512])
  else
    AC_MSG_WARN([$OPENSSL_PKG_ERRORS])
    if test "x$with_openssl_requested" = "xyes"; then
      ARIA2_DEP_NOT_MET([openssl])
    fi
  fi
fi

if test "x$have_openssl" != "xyes"; then
  if test "x$with_libnettle" = "xyes"; then
    AC_SEARCH_LIBS([nettle_sha1_init], [nettle],
                   [have_libnettle=yes], [have_libnettle=no])
    if test "x$have_libnettle" = "xyes"; then
      AC_DEFINE([HAVE_LIBNETTLE], [1], [Define to 1 if you have libnettle.])
    fi
  fi
  if test "x$with_libgmp" = "xyes" &&
     test "x$have_libnettle" = "xyes" &&
     test "x$enable_bittorrent" = "xyes"; then
    AC_SEARCH_LIBS([__gmpz_init], [gmp], [have_libgmp=yes], [have_libgmp=no])
    if test "x$have_libgmp" = "xyes"; then
      AC_DEFINE([HAVE_LIBGMP], [1], [Define to 1 if you have libgmp.])
    fi
  fi
  if test "x$with_libgcrypt" = "xyes" &&
     test "x$have_libnettle" != "xyes"; then
    AM_PATH_LIBGCRYPT([1.2.4], [have_libgcrypt=yes])
    if test "x$have_libgcrypt" = "xyes"; then
      AC_DEFINE([HAVE_LIBGCRYPT], [1], [Define to 1 if you have libgcrypt.])
      LIBS="$LIBGCRYPT_LIBS $LIBS"
      CPPFLAGS="$LIBGCRYPT_CFLAGS $CPPFLAGS"
    fi
  fi
fi

if test "x$with_libcares" = "xyes"; then
  PKG_CHECK_MODULES([LIBCARES], [libcares >= 1.7.0], [have_libcares=yes],
                    [have_libcares=no])
  if test "x$have_libcares" = "xyes"; then
    AC_DEFINE([HAVE_LIBCARES], [1], [Define to 1 if you have libcares.])
    LIBS="$LIBCARES_LIBS $LIBS"
    CPPFLAGS="$LIBCARES_CFLAGS $CPPFLAGS"
    AC_CHECK_TYPES([ares_addr_node], [], [], [[#include <ares.h>]])
    AC_CHECK_FUNCS([ares_set_servers])

    if test "x$ARIA2_STATIC" = "xyes"; then
      CPPFLAGS="-DCARES_STATICLIB $CPPFLAGS"
    fi
  else
    AC_MSG_WARN([$LIBCARES_PKG_ERRORS])
    if test "x$with_libcares_requested" = "xyes"; then
      ARIA2_DEP_NOT_MET([libcares])
    fi
  fi
fi

use_md=""
if test "x$have_osx" == "xyes"; then
  use_md="apple"
  AC_DEFINE([USE_APPLE_MD], [1], [What message digest implementation to use])
else
  if test "x$have_libnettle" = "xyes"; then
    AC_DEFINE([USE_LIBNETTLE_MD], [1], [What message digest implementation to use])
    use_md="libnettle"
  else
    if test "x$have_libgcrypt" = "xyes"; then
      AC_DEFINE([USE_LIBGCRYPT_MD], [1], [What message digest implementation to use])
      use_md="libgcrypt"
    else
      if test "x$have_openssl" = "xyes"; then
        AC_DEFINE([USE_OPENSSL_MD], [1], [What message digest implementation to use])
        use_md="openssl"
      fi
    fi
  fi
fi

# Define variables based on the result of the checks for libraries.
if test "x$have_appletls" = "xyes" || test "x$have_libgnutls" = "xyes" || test "x$have_openssl" = "xyes"; then
  have_ssl="yes"
  AC_DEFINE([ENABLE_SSL], [1], [Define to 1 if ssl support is enabled.])
  AM_CONDITIONAL([ENABLE_SSL], true)
  AC_SUBST([ca_bundle])
else
  AM_CONDITIONAL([ENABLE_SSL], false)
fi


AM_CONDITIONAL([HAVE_OSX], [ test "x$have_osx" = "xyes" ])
AM_CONDITIONAL([HAVE_APPLETLS], [ test "x$have_appletls" = "xyes" ])
AM_CONDITIONAL([USE_APPLE_MD], [ test "x$use_md" = "xapple" ])
AM_CONDITIONAL([HAVE_LIBGNUTLS], [ test "x$have_libgnutls" = "xyes" ])
AM_CONDITIONAL([HAVE_LIBNETTLE], [ test "x$have_libnettle" = "xyes" ])
AM_CONDITIONAL([USE_LIBNETTLE_MD], [ test "x$use_md" = "xlibnettle"])
AM_CONDITIONAL([HAVE_LIBGMP], [ test "x$have_libgmp" = "xyes" ])
AM_CONDITIONAL([HAVE_LIBGCRYPT], [ test "x$have_libgcrypt" = "xyes" ])
AM_CONDITIONAL([USE_LIBGCRYPT_MD], [ test "x$use_md" = "xlibgcrypt"])
AM_CONDITIONAL([HAVE_OPENSSL], [ test "x$have_openssl" = "xyes" ])
AM_CONDITIONAL([USE_OPENSSL_MD], [ test "x$use_md" = "xopenssl"])

if test "x$use_md" != "x"; then
  AC_DEFINE([ENABLE_MESSAGE_DIGEST], [1],
            [Define to 1 if message digest support is enabled.])
  AM_CONDITIONAL([ENABLE_MESSAGE_DIGEST], true)
  enable_message_digest=yes
else
  AM_CONDITIONAL([ENABLE_MESSAGE_DIGEST], false)
fi

if test "x$have_libnettle" = "xyes" && test "x$have_libgmp" = "xyes" ||
   test "x$have_libgcrypt" = "xyes" || test "x$have_openssl" = "xyes"; then
  enable_bignum=yes
fi

if test "x$enable_bittorrent" = "xyes" &&
   test "x$enable_message_digest" = "xyes" &&
   test "x$enable_bignum" = "xyes"; then
  AC_DEFINE([ENABLE_BITTORRENT], [1],
            [Define to 1 if BitTorrent support is enabled.])
  AM_CONDITIONAL([ENABLE_BITTORRENT], true)
else
  if test "x$enable_bittorrent_requested" = "xyes"; then
    ARIA2_FET_NOT_SUPPORTED([bittorrent])
  fi
  enable_bittorrent=no
  AM_CONDITIONAL([ENABLE_BITTORRENT], false)
fi

if (test "x$have_libxml2" = "xyes" || test "x$have_libexpat" = "xyes") &&
   test "x$enable_metalink" = "xyes"; then
  AC_DEFINE([ENABLE_METALINK], [1],
            [Define to 1 if Metalink support is enabled.])
  AM_CONDITIONAL([ENABLE_METALINK], true)
else
  if test "x$enable_metalink_requested" = "xyes"; then
    ARIA2_FET_NOT_SUPPORTED([metalink])
  fi
  enable_metalink=no
  AM_CONDITIONAL([ENABLE_METALINK], false)
fi

AM_CONDITIONAL([HAVE_SOME_XMLLIB],
  [test "x$have_libxml2" = "xyes" || test "x$have_libexpat" = "xyes"])

if test "x$have_libxml2" = "xyes" || test "x$have_libexpat" = "xyes"; then
  enable_xml_rpc=yes
fi

if test "x$enable_xml_rpc" = "xyes"; then
  AC_DEFINE([ENABLE_XML_RPC], [1],
            [Define to 1 if XML-RPC support is enabled.])
fi
AM_CONDITIONAL([ENABLE_XML_RPC], [test "x$enable_xml_rpc" = "xyes"])

AM_CONDITIONAL([HAVE_LIBXML2], [test "x$have_libxml2" = "xyes"])
AM_CONDITIONAL([HAVE_LIBEXPAT], [test "x$have_libexpat" = "xyes"])

if test "x$have_libcares" = "xyes"; then
  AC_DEFINE([ENABLE_ASYNC_DNS], [1],
            [Define to 1 if asynchronous DNS support is enabled.])
  AM_CONDITIONAL([ENABLE_ASYNC_DNS], true)
else
  AM_CONDITIONAL([ENABLE_ASYNC_DNS], false)
fi

# Set conditional for libz
AM_CONDITIONAL([HAVE_ZLIB], [test "x$have_zlib" = "xyes"])

# Set conditional for sqlite3
AM_CONDITIONAL([HAVE_SQLITE3], [test "x$have_sqlite3" = "xyes"])

AC_SEARCH_LIBS([clock_gettime], [rt])

case "$host" in
  *solaris*)
    AC_SEARCH_LIBS([getaddrinfo], [nsl socket])
    ;;
esac

# Checks for header files.
AC_FUNC_ALLOCA
AC_HEADER_STDC

case "$host" in
	*mingw*)
    AC_CHECK_HEADERS([windows.h \
                      winsock2.h \
                      ws2tcpip.h \
                      mmsystem.h \
                      io.h \
                      iphlpapi.h\
                      winioctl.h \
                      share.h], [], [],
                      [[
#ifdef HAVE_WS2TCPIP_H
# include <ws2tcpip.h>
#endif
#ifdef HAVE_WINSOCK2_H
# include <winsock2.h>
#endif
#ifdef HAVE_WINDOWS_H
# include <windows.h>
#endif
                      ]])
		;;
esac

AC_CHECK_HEADERS([argz.h \
                  arpa/inet.h \
                  fcntl.h \
                  float.h \
                  inttypes.h \
                  langinfo.h \
                  libintl.h \
                  limits.h \
                  locale.h \
                  malloc.h \
                  netdb.h \
                  netinet/in.h \
                  netinet/tcp.h \
                  poll.h \
                  port.h \
                  signal.h \
                  stddef.h \
                  stdint.h \
                  stdio_ext.h \
                  stdlib.h \
                  string.h \
                  strings.h \
                  sys/ioctl.h \
                  sys/param.h \
                  sys/signal.h \
                  sys/socket.h \
                  sys/time.h \
                  sys/types.h \
                  sys/uio.h \
                  termios.h \
                  unistd.h \
                  utime.h \
                  wchar.h \
                  ifaddrs.h \
                  pwd.h])

# Checks for typedefs, structures, and compiler characteristics.
AC_HEADER_STDBOOL
AC_C_CONST
AC_C_INLINE
AC_TYPE_INT16_T
AC_TYPE_INT32_T
AC_TYPE_INT64_T
AC_TYPE_INT8_T
AC_TYPE_MODE_T
AC_TYPE_OFF_T
AC_TYPE_SIZE_T
AC_TYPE_SSIZE_T
AC_HEADER_TIME
AC_STRUCT_TM
AC_TYPE_UINT16_T
AC_TYPE_UINT32_T
AC_TYPE_UINT64_T
AC_TYPE_UINT8_T
AC_TYPE_PID_T
AC_C_VOLATILE
AC_CHECK_TYPES([ptrdiff_t, struct timespec])
AC_C_BIGENDIAN
AC_SYS_LARGEFILE

# Checks for library functions.
AM_GNU_GETTEXT
AM_GNU_GETTEXT_VERSION([0.18])
AC_FUNC_ERROR_AT_LINE
AC_PROG_GCC_TRADITIONAL

AC_FUNC_MEMCMP
AC_FUNC_MKTIME
AC_FUNC_SELECT_ARGTYPES
AC_FUNC_STAT
AC_FUNC_STRFTIME
AC_FUNC_VPRINTF
AC_FUNC_FORK
AC_FUNC_STRTOD
# Don't use AC_FUNC_MMAP becaue it fails on some platforms (e.g.,
# OpenWRT) which have mmap and it works in the way we use in aria2.
# Instead use mmap in AC_CHECK_FUNCS list.
AC_CHECK_FUNCS([__argz_count \
                __argz_next \
                __argz_stringify \
                atexit \
                ftruncate \
                getcwd \
                gethostbyaddr \
                gethostbyname \
		getifaddrs \
                getpagesize \
                memchr \
                memmove \
                mempcpy \
                memset \
                mkdir \
                mmap \
                munmap \
                nl_langinfo \
                posix_memalign \
		pow \
                putenv \
                rmdir \
                select \
                setlocale \
                sigaction \
                sleep \
                socket \
                stpcpy \
                strcasecmp \
                strchr \
                strcspn \
                strdup \
                strerror \
                strncasecmp \
                strstr \
                strtol \
                strtoul \
                strtoull \
                tzset \
                unsetenv \
                usleep \
		utime \
		utimes])

dnl Put tcmalloc/jemalloc checks after the posix_memalign check.
dnl These libraries may implement posix_memalign, while the usual CRT may not
dnl (e.g. mingw). Since we aren't including the corresponding library headers
dnl this will lead to undefined posix_memalign() errors when compiling
if test "x$with_tcmalloc_requested" = "xyes" &&
   test "x$with_jemalloc_requested" = "xyes"; then
  AC_MSG_FAILURE([Cannot use both, tcmalloc and jemalloc!])
fi

if test "x$with_tcmalloc" = "xyes"; then
  dnl Important: put malloc libs at the very end.
  dnl Only newish versions have a .pc, thus try CHECK_LIB as well.
  PKG_CHECK_MODULES([TCMALLOC], [libtcmalloc_minimal], [have_tcmalloc=yes], [have_tcmalloc=no])
  if test "x$have_tcmalloc" = "xyes"; then
    CPPFLAGS="$TCMALLOC_CFLAGS $CPPFLAGS"
    LIBS="$LIBS $TCMALLOC_LIBS"
  else
    AC_CHECK_LIB([tcmalloc_minimal], [malloc], [have_tcmalloc=yes], [have_tcmalloc=no])
    if test "x$have_tcmalloc" = "xyes"; then
      LIBS="$LIBS -ltcmalloc_minimal"
    else
      if test "x$with_tcmalloc_requested" = "xyes"; then
        ARIA2_DEP_NOT_MET([tcmalloc_minimal])
      fi
    fi
  fi
fi

if test "x$have_tcmalloc" != "xyes" && test "x$with_jemalloc" = "xyes"; then
  dnl Important: put malloc libs at the very end.
  dnl Usually jemalloc does not come with a .pc, as the official source does not
  dnl generate one.
  PKG_CHECK_MODULES([JEMALLOC], [jemalloc], [have_jemalloc=yes], [have_jemalloc=no])
  if test "x$have_jemalloc" = "xyes"; then
    CPPFLAGS="$JEMALLOC_CFLAGS $CPPFLAGS"
    LIBS="$LIBS $JEMALLOC_LIBS"
  else
    AC_CHECK_LIB([jemalloc], [malloc], [have_jemalloc=yes], [have_jemalloc=no])
    if test "x$have_jemalloc" = "xyes"; then
      LIBS="$LIBS -ljemalloc"
    else
      if test "x$with_jemalloc_requested" = "xyes"; then
        ARIA2_DEP_NOT_MET([jemalloc (unprefixed)])
      fi
    fi
  fi
fi


if test "x$enable_epoll" = "xyes"; then
  AC_CHECK_FUNCS([epoll_create], [have_epoll=yes])
  if test "x$have_epoll" = "xyes"; then
    AC_DEFINE([HAVE_EPOLL], [1], [Define to 1 if epoll is available.])
  fi
fi
AM_CONDITIONAL([HAVE_EPOLL], [test "x$have_epoll" = "xyes"])

AC_CHECK_FUNCS([posix_fallocate],[have_posix_fallocate=yes])
ARIA2_CHECK_FALLOCATE
if test "x$have_posix_fallocate" = "xyes" ||
   test "x$have_fallocate" = "xyes" ||
   test "x$win_build" = "xyes"; then
  AC_DEFINE([HAVE_SOME_FALLOCATE], [1],
            [Define to 1 if *_fallocate is available.])
fi
AM_CONDITIONAL([HAVE_SOME_FALLOCATE],
  [test "x$have_posix_fallocate" = "xyes" || test "x$have_fallocate" = "xyes" \
  || test "x$win_build" = "xyes"])


AC_CHECK_FUNCS([asctime_r],
	[AM_CONDITIONAL([HAVE_ASCTIME_R], true)],
	[AM_CONDITIONAL([HAVE_ASCTIME_R], false)])
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
AC_CHECK_FUNCS([localtime_r],
	[AM_CONDITIONAL([HAVE_LOCALTIME_R], true)],
	[AM_CONDITIONAL([HAVE_LOCALTIME_R], false)])
AC_CHECK_FUNCS([strptime],
	[AM_CONDITIONAL([HAVE_STRPTIME], true)],
	[AM_CONDITIONAL([HAVE_STRPTIME], false)])
AC_CHECK_FUNCS([timegm],
	[AM_CONDITIONAL([HAVE_TIMEGM], true)],
	[AM_CONDITIONAL([HAVE_TIMEGM], false)])
AC_CHECK_FUNCS([daemon], [have_daemon=yes])
AM_CONDITIONAL([HAVE_DAEMON], [test "x$have_daemon" = "xyes"])

AC_CHECK_FUNCS([clock_gettime], [have_clock_gettime=yes])

if test "x$have_clock_gettime" != "xyes"; then
   AC_CHECK_FUNCS([mach_absolute_time], [have_mach_absolute_time=yes])
fi
AM_CONDITIONAL([HAVE_MACH_ABSOLUTE_TIME],
		[test "x$have_mach_absolute_time" = "xyes"])

AC_CHECK_FUNCS([poll], [have_poll=yes])
AM_CONDITIONAL([HAVE_POLL], [test "x$have_poll" = "xyes"])

case "$host" in
  *mingw*)
    AM_CONDITIONAL([MINGW_BUILD], true)
    dnl defined in ws2tcpip.h, but only if _WIN32_WINNT >= 0x0501
    AM_CONDITIONAL([HAVE_GETADDRINFO], true)
    dnl defined in ws2tcpip.h, but missing in C:\mingw\lib\libws2_32.a
    AM_CONDITIONAL([HAVE_GAI_STRERROR], false)
    if test "x$have_clock_gettime" != "xyes"; then
        AM_CONDITIONAL([HAVE_TIMEGETTIME], true)
    fi
    ;;
  *)
    AM_CONDITIONAL([MINGW_BUILD], false)
    AM_CONDITIONAL([HAVE_TIMEGETTIME], false)
    ;;
esac

AC_CHECK_FUNCS([port_associate], [have_port_associate=yes])
AM_CONDITIONAL([HAVE_PORT_ASSOCIATE], [test "x$have_port_associate" = "xyes"])

AC_CHECK_FUNCS([kqueue], [have_kqueue=yes])
AM_CONDITIONAL([HAVE_KQUEUE], [test "x$have_kqueue" = "xyes"])
if test "x$have_kqueue" = "xyes"; then
    AC_MSG_CHECKING([whether struct kevent.udata is intptr_t])
    AC_COMPILE_IFELSE([AC_LANG_PROGRAM([[
#include <sys/types.h>
#include <sys/event.h>
#include <sys/time.h>
]],
[[
struct kevent event;
event.udata = reinterpret_cast<intptr_t>(&event);
]])],
    [kevent_udata_intptr_t=yes], [kevent_udata_intptr_t=no])
    AC_MSG_RESULT([$kevent_udata_intptr_t])
    if test "x$kevent_udata_intptr_t" = "xyes"; then
       AC_DEFINE([KEVENT_UDATA_INTPTR_T], [1], [Define to 1 if struct kevent.udata is intptr_t])
    fi
fi

AC_CHECK_MEMBER([struct sockaddr_in.sin_len],
                [AC_DEFINE([HAVE_SOCKADDR_IN_SIN_LEN],[1],
                  [Define to 1 if struct sockaddr_in has sin_len member.])],
                [],
                [[
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
]])

AC_CHECK_MEMBER([struct sockaddr_in6.sin6_len],
                [AC_DEFINE([HAVE_SOCKADDR_IN6_SIN6_LEN],[1],
                  [Define to 1 if struct sockaddr_in6 has sin6_len member.])],
                [],
                [[
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
]])

# Check struct option.name is assignable from const char*.  struct
# option.name in opensolaris is of type char*. In Linux, it is const
# char*
AC_MSG_CHECKING([whether struct option.name is assignable from const char*])
AC_COMPILE_IFELSE([AC_LANG_PROGRAM([[
#include <unistd.h>
#include <getopt.h>
]],
[[
const char* s = "const char";
option op;
op.name = s;
]])],
[have_option_const_name=yes], [have_option_const_name=no])
AC_MSG_RESULT([$have_option_const_name])
if test "x$have_option_const_name" = "xyes"; then
  AC_DEFINE([HAVE_OPTION_CONST_NAME], [1], [Define 1 if struct option.name is const char*])
fi

AC_CONFIG_SUBDIRS([deps/wslay])
if test "x$enable_message_digest" = "xyes"; then
  enable_websocket=yes
  AC_DEFINE([ENABLE_WEBSOCKET], [1],
            [Define 1 if WebSocket support is enabled.])
  WSLAY_LIBS="\$(top_builddir)/deps/wslay/lib/libwslay.la"
  AC_SUBST([WSLAY_LIBS])
  # $(top_srcdir) for `make distcheck`
  CPPFLAGS="-I\$(top_builddir)/deps/wslay/lib/includes -I\$(top_srcdir)/deps/wslay/lib/includes $CPPFLAGS"
fi
AM_CONDITIONAL([ENABLE_WEBSOCKET], [test "x$enable_websocket" = "xyes"])

AM_CONDITIONAL([ENABLE_LIBARIA2], [test "x$enable_libaria2" = "xyes"])

AC_SUBST([bashcompletiondir])

case "$host" in
  *android*)
    LIBS="$LIBS -lstdc++ -lsupc++"
    case "$host" in
      arm-*)
        android_arm=yes
        ;;
      mipsel-*)
        android_mips=yes
        ;;
      i686-*)
        android_x86=yes
        ;;
    esac
    ;;
  *)
    ;;
esac

AM_CONDITIONAL([ANDROID_ARM], [test "x$android_arm" = "xyes"])
AM_CONDITIONAL([ANDROID_MIPS], [test "x$android_mips" = "xyes"])
AM_CONDITIONAL([ANDROID_X86], [test "x$android_x86" = "xyes"])

if test "x$ARIA2_STATIC" = "xyes"; then
  LDFLAGS="$LDFLAGS -all-static -static-libgcc -static-libstdc++"
  dnl For non-MinGW build, we need additional libs for static build.
  if test "x$win_build" != "xyes"; then
    LIBS="$LIBS -lpthread -ldl -lrt"
  fi
fi

AC_CONFIG_FILES([Makefile
		src/Makefile
		src/libaria2.pc
		src/includes/Makefile
		test/Makefile
		po/Makefile.in
		intl/Makefile
		lib/Makefile
		doc/Makefile
		doc/manual-src/Makefile
		doc/manual-src/en/Makefile
		doc/manual-src/en/conf.py
		doc/manual-src/ru/Makefile
		doc/manual-src/ru/conf.py
		doc/manual-src/pt/Makefile
		doc/manual-src/pt/conf.py
		deps/Makefile])
AC_OUTPUT

echo " "
echo "Build:          $build"
echo "Host:           $host"
echo "Target:         $target"
echo "Install prefix: $prefix"
echo "CC:             $CC"
echo "CXX:            $CXX"
echo "CPP:            $CPP"
echo "CXXFLAGS:       $CXXFLAGS"
echo "CFLAGS:         $CFLAGS"
echo "CPPFLAGS:       $CPPFLAGS"
echo "LDFLAGS:        $LDFLAGS"
echo "LIBS:           $LIBS"
echo "DEFS:           $DEFS"
echo "LibUV:          $have_libuv"
echo "SQLite3:        $have_sqlite3"
echo "SSL Support:    $have_ssl"
echo "AppleTLS:       $have_appletls"
echo "GnuTLS:         $have_libgnutls"
echo "OpenSSL:        $have_openssl"
echo "CA Bundle:      $ca_bundle"
echo "LibXML2:        $have_libxml2"
echo "LibExpat:       $have_libexpat"
echo "LibCares:       $have_libcares"
echo "Zlib:           $have_zlib"
echo "Epoll:          $have_epoll"
echo "Bittorrent:     $enable_bittorrent"
echo "Metalink:       $enable_metalink"
echo "XML-RPC:        $enable_xml_rpc"
echo "Message Digest: $enable_message_digest"
echo "WebSocket:      $enable_websocket"
echo "Libaria2:       $enable_libaria2"
if test "x$enable_libaria2" = "xyes"; then
  echo "Library types:  Shared=${enable_shared}, Static=${enable_static}"
fi
echo "bash_completion dir: $bashcompletiondir"
echo "Static build:   $ARIA2_STATIC"
