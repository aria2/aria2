#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.
#
AC_PREREQ([2.67])
AC_INIT([aria2],[1.36.0],[https://github.com/aria2/aria2/issues],[aria2],[https://aria2.github.io/])

AC_CANONICAL_HOST
AC_CANONICAL_TARGET

AM_INIT_AUTOMAKE([subdir-objects])
m4_ifdef([AM_SILENT_RULES], [AM_SILENT_RULES([yes])])

AC_USE_SYSTEM_EXTENSIONS

LT_PREREQ([2.2.6])
LT_INIT([disable-static])
dnl See versioning rule:
dnl  http://www.gnu.org/software/libtool/manual/html_node/Updating-version-info.html
AC_SUBST(LT_CURRENT, 0)
AC_SUBST(LT_REVISION, 0)
AC_SUBST(LT_AGE, 0)

AC_CONFIG_MACRO_DIR([m4])
AC_CONFIG_SRCDIR([src/a2io.h])
AC_CONFIG_HEADERS([config.h])

# extra flags
EXTRACFLAGS=
EXTRACXXFLAGS=
EXTRACPPFLAGS=
EXTRALDFLAGS=
EXTRALIBS=

case "$host" in
  *mingw*)
    win_build=yes
    EXTRALIBS="-lws2_32 -lwsock32 -lgdi32 -lwinmm -liphlpapi -lpsapi $EXTRALIBS"
    # Define _POSIX_C_SOURCE to 1. This makes {asc,local}time_r
    # available from <time.h> even without (un)helpful interference
    # from <pthread.h>, and also defines __USE_MINGW_ANSI_STDIO.
    EXTRACPPFLAGS="$EXTRACPPFLAGS -D_POSIX_C_SOURCE=1"
    # Build with ASLR (dynamicbase) and NX compatiblity (nxcompat)
    # Enable pie once upstream/binutils gets fixed to produce correct
    # binaries with it.
    EXTRALDFLAGS="$EXTRALDFLAGS -Wl,--dynamicbase -Wl,--nxcompat"
    ;;
esac

AC_DEFINE_UNQUOTED([BUILD], ["$build"], [Define build-type])
AC_DEFINE_UNQUOTED([HOST], ["$host"], [Define build-type])
AC_DEFINE_UNQUOTED([TARGET], ["$target"], [Define target-type])

# Checks for arguments.
ARIA2_ARG_WITH([libuv])
ARIA2_ARG_WITHOUT([appletls])
ARIA2_ARG_WITHOUT([wintls])
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
ARIA2_ARG_WITHOUT([libssh2])

ARIA2_ARG_DISABLE([ssl])
ARIA2_ARG_DISABLE([bittorrent])
ARIA2_ARG_DISABLE([metalink])
ARIA2_ARG_DISABLE([websocket])
ARIA2_ARG_DISABLE([epoll])
ARIA2_ARG_ENABLE([libaria2])
ARIA2_ARG_ENABLE([werror])

AC_ARG_WITH([ca-bundle],
  AS_HELP_STRING([--with-ca-bundle=FILE],[Use FILE as default CA bundle.]),
  [AC_DEFINE_UNQUOTED([CA_BUNDLE], ["$withval"], [Define to choose default CA bundle.])
   ca_bundle=$withval
  ], [])

AC_ARG_WITH([disk-cache],
  AS_HELP_STRING([--with-disk-cache=SIZE],[Use SIZE as the default disk-cache size.]),
  [AC_DEFINE_UNQUOTED([DEFAULT_DISK_CACHE], ["$withval"], [Define to choose default disk-cache size])],
  [])

AC_ARG_WITH([bashcompletiondir],
  AS_HELP_STRING([--with-bashcompletiondir=DIR],
                 [Directory to install bash_completion file]),
  [bashcompletiondir=$withval], [bashcompletiondir=$docdir/bash_completion])

AC_ARG_VAR([ARIA2_STATIC], [Set 'yes' to build a statically linked aria2])

AC_ARG_ENABLE([gnutls-system-crypto-policy],
    AS_HELP_STRING([--enable-gnutls-system-crypto-policy], [Enable gnutls system wide crypto policy]))

AS_IF([test "x$enable_gnutls_system_crypto_policy" = "xyes"], [
  AC_DEFINE([USE_GNUTLS_SYSTEM_CRYPTO_POLICY], [1], [Define to 1 if using gnutls system wide crypto policy .])
])

# Checks for programs.
AC_PROG_CXX
AC_PROG_CC
AC_PROG_INSTALL
AC_PROG_MKDIR_P
AC_PROG_YACC
AM_PROG_AS

# Speed GCC compilation up.
if test "$GCC" = yes; then
  EXTRACFLAGS="$EXTRACFLAGS -pipe"
fi
if test "$GXX" = yes; then
  EXTRACXXFLAGS="$EXTRACXXFLAGS -pipe"
fi

AC_CHECK_TOOL([AR], [ar], [:])
if test "x$AR" = "x:"; then
  AC_MSG_FAILURE([ar is not found in the system.])
fi
AC_SUBST([AR])

AC_PATH_PROGS([SPHINXBUILD], [sphinx-build])
AC_SUBST([SPHINXBUILD])
AM_CONDITIONAL([HAVE_SPHINXBUILD], [ test "x$SPHINXBUILD" != "x" ])

AC_PATH_PROGS([RST2HTML], [rst2html.py rst2html])
AC_SUBST([RST2HTML])
AM_CONDITIONAL([HAVE_RST2HTML], [ test "x$RST2HTML" != "x" ])

# Setting language choice
AC_LANG([C++])

# Check pkg-config is available
PKG_PROG_PKG_CONFIG([0.20])

# Check C++ compiler supports C++0x/C++11 feature
save_CXXFLAGS="$CXXFLAGS"
CXXFLAGS=

AX_CXX_COMPILE_STDCXX_11([noext], [mandatory])

CXX1XCXXFLAGS="$CXXFLAGS"
CXXFLAGS="$save_CXXFLAGS"
AC_SUBST([CXX1XCXXFLAGS])

# Check C++ compiler actually supports nullptr
save_CXXFLAGS=$CXXFLAGS
CXXFLAGS="$CXXFLAGS $CXX1XCXXFLAGS"

AC_MSG_CHECKING([whether the c++ compiler supports nullptr])
AC_COMPILE_IFELSE([AC_LANG_PROGRAM([[
]],
[[
int *a = nullptr;
]])],
[],
[AC_MSG_FAILURE([C++ compiler does not understand nullptr, perhaps C++ compiler is too old.  Try again with new one (gcc >= 4.8.3 or clang >= 3.4)])])

# i686-w64-mingw32-g++ 4.6 does not support override keyword. For
# those compilers, define CXX11_OVERRIDE to empty string. Otherwise
# define it as override. Use CXX11_OVERRIDE instead of override.
AC_MSG_CHECKING([whether the C++ compiler supports `override` keyword])
AC_COMPILE_IFELSE([AC_LANG_PROGRAM([[
struct Base {
virtual void f() = 0;
};
struct Derived : Base {
virtual void f() override {}
};
]],
[[
Derived x;
]])],
    [cxx11_override=override
     AC_MSG_RESULT([yes])],
    [AC_MSG_RESULT([no])])
AC_DEFINE_UNQUOTED([CXX11_OVERRIDE], [$cxx11_override],
                   [Define `override` keyword if the compiler supports it])

CXXFLAGS=$save_CXXFLAGS

# Check static build is requested
if test "x$ARIA2_STATIC" = "xyes"; then
  case "$host" in
    i686*mingw*)
      dnl Define _USE_32BIT_TIME_T because 32bit library of MinGW-w64
      dnl does not implement many 64bit version functions.
      EXTRACPPFLAGS="$EXTRACPPFLAGS -D_USE_32BIT_TIME_T"
    ;;
  esac
  # Make pkg-config produce static linking variables
  PKG_CONFIG="$PKG_CONFIG --static"
else
  dnl Make variable empty to avoid confusion
  ARIA2_STATIC=no
fi

# Checks for libraries.

# Check availability of cppunit
PKG_CHECK_MODULES([CPPUNIT], [cppunit >= 1.10.2],
                  [have_cppunit=yes], [have_cppunit=no])
if test "x$have_cppunit" != "xyes"; then
  AC_MSG_WARN([$CPPUNIT_PKG_ERRORS])
fi

# Check availability of libz
if test "x$with_libz" = "xyes"; then
  PKG_CHECK_MODULES([ZLIB], [zlib >= 1.2.3], [have_zlib=yes], [have_zlib=no])
  if test "x$have_zlib" != "xyes"; then
    AC_MSG_WARN([$ZLIB_PKG_ERRORS])
    AC_CHECK_LIB([z], [zlibVersion], [have_zlib=yes], [have_zlib=no])

    if test "x$have_zlib" = "xyes"; then
      ZLIB_CFLAGS=
      ZLIB_LIBS="-lz"
      AC_SUBST([ZLIB_CFLAGS])
      AC_SUBST([ZLIB_LIBS])
    elif test "x$with_libz_requested" = "xyes"; then
      ARIA2_DEP_NOT_MET([libz])
    fi
  fi
  if test "x$have_zlib" = "xyes"; then
    AC_DEFINE([HAVE_ZLIB], [1], [Define to 1 if you have zlib.])
    save_CPPFLAGS=$CPPFLAGS
    save_LIBS=$LIBS
    CPPFLAGS="$CPPFLAGS $ZLIB_CFLAGS"
    LIBS="$ZLIB_LIBS $LIBS"
    AC_CHECK_FUNCS([gzbuffer gzsetparams])
    CPPFLAGS=$save_CPPFLAGS
    LIBS=$save_LIBS
  fi
fi

have_libuv=no
if test "x$with_libuv" = "xyes"; then
  PKG_CHECK_MODULES([LIBUV], [libuv >= 1.13],
                    [have_libuv=yes], [have_libuv=no])
  if test "x$have_libuv" = "xyes"; then
    AC_DEFINE([HAVE_LIBUV], [1], [Define to 1 if you have libuv.])
  elif test "x$with_libuv_requested" = "xyes"; then
    ARIA2_DEP_NOT_MET([libuv])
  fi
fi
AM_CONDITIONAL([HAVE_LIBUV], [test "x$have_libuv" = "xyes"])

have_libxml2=no
if test "x$with_libxml2" = "xyes"; then
  PKG_CHECK_MODULES([LIBXML2],[libxml-2.0 >= 2.6.24],[have_libxml2=yes],[have_libxml2=no])
  if test "x$have_libxml2" = "xyes"; then
    AC_DEFINE([HAVE_LIBXML2], [1], [Define to 1 if you have libxml2.])
  elif test "x$with_libxml2_requested" = "xyes"; then
    ARIA2_DEP_NOT_MET([libxml2])
  fi
fi

have_libexpat=no
if test "x$with_libexpat" = "xyes" && test "x$have_libxml2" != "xyes"; then
  PKG_CHECK_MODULES([EXPAT],[expat],[have_libexpat=yes],[have_libexpat=no])
  if test "x$have_libexpat" = "xyes"; then
    AC_DEFINE([HAVE_LIBEXPAT], [1], [Define to 1 if you have libexpat.])
  elif test "x$with_libexpat_requested" = "xyes"; then
    ARIA2_DEP_NOT_MET([libexpat])
  fi
fi

have_sqlite3=no
if test "x$with_sqlite3" = "xyes"; then
  PKG_CHECK_MODULES([SQLITE3],[sqlite3],[have_sqlite3=yes],[have_sqlite3=no])
  if test "x$have_sqlite3" = "xyes"; then
    AC_DEFINE([HAVE_SQLITE3], [1], [Define to 1 if you have sqlite3.])
    save_LIBS=$LIBS
    LIBS="$SQLITE3_LIBS $LIBS"
    AC_CHECK_FUNCS([sqlite3_open_v2])
    LIBS=$save_LIBS
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


if test "x$enable_ssl" != "xyes"; then
  with_appletls=no
  with_wintls=no
  with_libnettle=no
  with_libgcrypt=no
  with_gnutls=no
  with_openssl=no
fi

have_appletls=no
if test "x$with_appletls" = "xyes"; then
  AC_MSG_CHECKING([whether to enable Mac OS X native SSL/TLS])
  if test "x$have_osx" = "xyes"; then
    AC_DEFINE([HAVE_APPLETLS], [1], [Define to 1 if you have Apple TLS])
    APPLETLS_LDFLAGS="-framework CoreFoundation -framework Security"
    AC_SUBST([APPLETLS_LDFLAGS])
    have_appletls="yes"
    have_ssl=yes
    have_nativetls=yes
    AC_MSG_RESULT(yes)
  else
    AC_MSG_RESULT(no)
    if test "x$with_appletls_requested" = "xyes"; then
      ARIA2_DEP_NOT_MET([appletls])
    fi
  fi
fi

have_wintls=no
if test "x$with_wintls" = "xyes"; then
  AC_HAVE_LIBRARY([crypt32],[have_wintls_libs=yes],[have_wintls_libs=no])
  AC_HAVE_LIBRARY([secur32],[have_wintls_libs=$have_wintls_libs],[have_wintls_libs=no])
  AC_HAVE_LIBRARY([advapi32],[have_wintls_libs=$have_wintls_libs],[have_wintls_libs=no])
  AC_CHECK_HEADER([wincrypt.h], [have_wintls_headers=yes], [have_wintls_headers=no], [[
#ifdef HAVE_WINDOWS_H
# include <windows.h>
#endif
  ]])
  AC_CHECK_HEADER([security.h], [have_wintls_headers=$have_wintls_headers], [have_wintls_headers=no], [[
#ifdef HAVE_WINDOWS_H
# include <windows.h>
#endif
#ifndef SECURITY_WIN32
#define SECURITY_WIN32 1
#endif
  ]])

  if test "x$have_wintls_libs" = "xyes" &&
     test "x$have_wintls_headers" = "xyes"; then
    AC_DEFINE([SECURITY_WIN32], [1], [Use security.h in WIN32 mode])
    WINTLS_LIBS="-lcrypt32 -lsecur32 -ladvapi32"
    AC_SUBST([WINTLS_LIBS])
    have_wintls=yes
    have_ssl=yes
    have_nativetls=yes
  else
    have_wintls=no
  fi
  if test "x$have_wintls" != "xyes"; then
    if test "x$with_wintls_requested" = "xyes"; then
      ARIA2_DEP_NOT_MET([wintls])
    fi
  fi
fi

have_libgnutls=no
if test "x$with_gnutls" = "xyes" && test "x$have_ssl" != "xyes"; then
  # gnutls >= 2.8 doesn't have libgnutls-config anymore. We require
  # 2.2.0 because we use gnutls_priority_set_direct()
  PKG_CHECK_MODULES([LIBGNUTLS], [gnutls >= 2.2.0],
                    [have_libgnutls=yes], [have_libgnutls=no])
  if test "x$have_libgnutls" = "xyes"; then
    have_ssl=yes
    AC_DEFINE([HAVE_LIBGNUTLS], [1], [Define to 1 if you have libgnutls.])
    save_LIBS=$LIBS
    LIBS="$LIBGNUTLS_LIBS $LIBS"
    AC_CHECK_FUNCS([gnutls_certificate_set_x509_system_trust])
    LIBS=$save_LIBS
  else
    AC_MSG_WARN([$LIBGNUTLS_PKG_ERRORS])
    if test "x$with_gnutls_requested" = "xyes"; then
      ARIA2_DEP_NOT_MET([gnutls])
    fi
  fi
fi

have_openssl=no
if test "x$with_openssl" = "xyes" && test "x$have_ssl" != "xyes"; then
  PKG_CHECK_MODULES([OPENSSL], [openssl >= 0.9.8],
                    [have_openssl=yes], [have_openssl=no])
  if test "x$have_openssl" = "xyes"; then
    have_ssl=yes
    AC_DEFINE([HAVE_OPENSSL], [1], [Define to 1 if you have openssl.])

    save_LIBS=$LIBS
    LIBS="$OPENSSL_LIBS $LIBS"
    AC_CHECK_FUNCS([EVP_DigestInit_ex], [have_digestinit_ex=yes])
    if test "x$have_digestinit_ex" = "x"; then
      AC_DEFINE([HAVE_OLD_OPENSSL], [1], [Define to 1 if you have old openssl.])
    fi
    AC_CHECK_FUNCS([EVP_sha224])
    AC_CHECK_FUNCS([EVP_sha256])
    AC_CHECK_FUNCS([EVP_sha384])
    AC_CHECK_FUNCS([EVP_sha512])
    LIBS=$save_LIBS
  else
    AC_MSG_WARN([$OPENSSL_PKG_ERRORS])
    if test "x$with_openssl_requested" = "xyes"; then
      ARIA2_DEP_NOT_MET([openssl])
    fi
  fi
fi

have_libnettle=no
have_libgmp=no
have_libgcrypt=no
if test "x$have_openssl" != "xyes"; then
  if test "x$with_libnettle" = "xyes" &&
     test "x$have_nativetls" != "xyes"; then
    PKG_CHECK_MODULES([LIBNETTLE], [nettle],
                      [have_libnettle=yes], [have_libnettle=no])
    if test "x$have_libnettle" = "xyes"; then
      AC_DEFINE([HAVE_LIBNETTLE], [1], [Define to 1 if you have libnettle.])
    elif test "x$with_libnettle_requested" = "xyes"; then
      ARIA2_DEP_NOT_MET([nettle])
    fi
  fi
  if test "x$with_libgmp" = "xyes" &&
     (test "x$have_libnettle" = "xyes" ||
      test "x$have_nativetls" = "xyes") &&
     test "x$enable_bittorrent" = "xyes"; then
    AC_CHECK_LIB([gmp], [__gmpz_init], [have_libgmp=yes], [have_libgmp=no])
    if test "x$have_libgmp" = "xyes"; then
      LIBGMP_CFLAGS=
      LIBGMP_LIBS=-lgmp
      AC_SUBST([LIBGMP_CFLAGS])
      AC_SUBST([LIBGMP_LIBS])
      AC_DEFINE([HAVE_LIBGMP], [1], [Define to 1 if you have libgmp.])
      save_LIBS=$LIBS
      LIBS="$LIBGMP_LIBS $LIBS"
      AC_CHECK_FUNCS([__gmpz_powm_sec], [have_mpz_powm_sec=yes])
      LIBS=$save_LIBS
      if test "x$have_mpz_powm_sec" = "xyes"; then
        AC_DEFINE([HAVE_GMP_SEC], [1], [Define to 1 if you have a GMP with sec functions.])
      fi
    else
      AC_MSG_WARN([libgmp not found])
      if test "x$with_libgmp_requested" = "xyes"; then
        ARIA2_DEP_NOT_MET([libgmp])
      fi
    fi
  fi
  if test "x$with_libgcrypt" = "xyes" &&
     test "x$have_nativetls" != "xyes" &&
     test "x$have_libnettle" != "xyes"; then
    m4_ifdef([AM_PATH_LIBGCRYPT], [AM_PATH_LIBGCRYPT([1.2.4], [have_libgcrypt=yes])], [
    AC_MSG_WARN([configure was generated without libgcrypt detection.  libgcrypt detection is disabled])
      LIBGCRYPT_CFLAGS=
      LIBGCRYPT_LIBS=
      AC_SUBST([LIBGCRYPT_CFLAGS])
      AC_SUBST([LIBGCRYPT_LIBS])
    ])
    if test "x$have_libgcrypt" = "xyes"; then
      AC_DEFINE([HAVE_LIBGCRYPT], [1], [Define to 1 if you have libgcrypt.])
    fi
  fi
fi

have_libssh2=no
if test "x$with_libssh2" = "xyes"; then
  PKG_CHECK_MODULES([LIBSSH2], [libssh2], [have_libssh2=yes], [have_libssh2=no])
  if test "x$have_libssh2" = "xyes"; then
    AC_DEFINE([HAVE_LIBSSH2], [1], [Define to 1 if you have libssh2.])
  else
    AC_MSG_WARN([$LIBSSH2_PKG_ERRORS])
    if test "x$with_libssh2_requested" = "xyes"; then
      ARIA2_DEP_NOT_MET([libssh2])
    fi
  fi
fi

have_libcares=no
if test "x$with_libcares" = "xyes"; then
  PKG_CHECK_MODULES([LIBCARES], [libcares >= 1.7.0], [have_libcares=yes],
                    [have_libcares=no])
  if test "x$have_libcares" = "xyes"; then
    AC_DEFINE([HAVE_LIBCARES], [1], [Define to 1 if you have libcares.])
    save_LIBS=$LIBS
    save_CPPFLAGS=$CPPFLAGS
    LIBS="$LIBCARES_LIBS $LIBS"
    CPPFLAGS="$LIBCARES_CFLAGS $CPPFLAGS"
    AC_CHECK_TYPES([ares_addr_node], [], [], [[#include <ares.h>]])
    AC_CHECK_FUNCS([ares_set_servers])
    LIBS=$save_LIBS
    CPPFLAGS=$save_CPPFLAGS

    # -DCARES_STATICLIB is appended by pkg-config file libcares.pc
  else
    AC_MSG_WARN([$LIBCARES_PKG_ERRORS])
    if test "x$with_libcares_requested" = "xyes"; then
      ARIA2_DEP_NOT_MET([libcares])
    fi
  fi
fi

use_md=""
if test "x$have_appletls" = "xyes"; then
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
      else
        AC_DEFINE([USE_INTERNAL_MD], [1], [What message digest implementation to use])
        use_md="internal"
      fi
    fi
  fi
fi

# Define variables based on the result of the checks for libraries.
if test "x$have_ssl" = "xyes"; then
  AC_DEFINE([ENABLE_SSL], [1], [Define to 1 if ssl support is enabled.])
  AM_CONDITIONAL([ENABLE_SSL], true)
else
  have_ssl="no"
  AM_CONDITIONAL([ENABLE_SSL], false)
fi


AM_CONDITIONAL([HAVE_OSX], [ test "x$have_osx" = "xyes" ])
AM_CONDITIONAL([HAVE_APPLETLS], [ test "x$have_appletls" = "xyes" ])
AM_CONDITIONAL([HAVE_WINTLS], [ test "x$have_wintls" = "xyes" ])
AM_CONDITIONAL([USE_APPLE_MD], [ test "x$use_md" = "xapple" ])
AM_CONDITIONAL([USE_WINDOWS_MD], [ test "x$use_md" = "xwindows" ])
AM_CONDITIONAL([HAVE_LIBGNUTLS], [ test "x$have_libgnutls" = "xyes" ])
AM_CONDITIONAL([HAVE_LIBNETTLE], [ test "x$have_libnettle" = "xyes" ])
AM_CONDITIONAL([USE_LIBNETTLE_MD], [ test "x$use_md" = "xlibnettle"])
AM_CONDITIONAL([HAVE_LIBGMP], [ test "x$have_libgmp" = "xyes" ])
AM_CONDITIONAL([HAVE_LIBGCRYPT], [ test "x$have_libgcrypt" = "xyes" ])
AM_CONDITIONAL([USE_LIBGCRYPT_MD], [ test "x$use_md" = "xlibgcrypt"])
AM_CONDITIONAL([HAVE_OPENSSL], [ test "x$have_openssl" = "xyes" ])
AM_CONDITIONAL([USE_OPENSSL_MD], [ test "x$use_md" = "xopenssl"])
AM_CONDITIONAL([USE_INTERNAL_MD], [ test "x$use_md" = "xinternal"])

if test "x$have_libgmp" = "xyes" ||
   test "x$have_libgcrypt" = "xyes" ||
   test "x$have_openssl" = "xyes"; then
  AM_CONDITIONAL([USE_INTERNAL_BIGNUM], false)
else
  AC_DEFINE([USE_INTERNAL_BIGNUM], [1], [Define to 1 if internal BIGNUM support is enabled.])
  AM_CONDITIONAL([USE_INTERNAL_BIGNUM], true)
fi

if test "x$have_libnettle" = "xyes" ||
   test "x$have_libgcrypt" = "xyes" ||
   test "x$have_openssl" = "xyes"; then
  AM_CONDITIONAL([USE_INTERNAL_ARC4], false)
else
  AC_DEFINE([USE_INTERNAL_ARC4], [1], [Define to 1 if internal ARC4 support is enabled.])
  AM_CONDITIONAL([USE_INTERNAL_ARC4], true)
fi

if test "x$enable_bittorrent" = "xyes"; then
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
else
  enable_xml_rpc=no
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

# Set conditional for libssh2
AM_CONDITIONAL([HAVE_LIBSSH2], [test "x$have_libssh2" = "xyes"])

case "$host" in
  *solaris*)
    save_LIBS=$LIBS
    LIBS=
    AC_SEARCH_LIBS([getaddrinfo], [nsl socket], [], [$save_LIBS])
    EXTRALIBS="$LIBS $EXTRALIBS"
    LIBS=$save_LIBS
    ;;
esac

# Checks for header files.
AC_FUNC_ALLOCA
AC_HEADER_STDC

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
                  sys/resource.h \
                  sys/signal.h \
                  sys/socket.h \
                  sys/time.h \
                  sys/types.h \
                  sys/uio.h \
                  sys/utsname.h \
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
AC_CHECK_TYPES([ptrdiff_t])
AC_CHECK_TYPE([struct timespec], [have_timespec=yes], [have_timespec=no])
AC_C_BIGENDIAN
AC_SYS_LARGEFILE

if test "x$have_timespec" = "xyes"; then
  AC_DEFINE([HAVE_A2_STRUCT_TIMESPEC], [1],
            [Define to 1 if the system has the type `struct timespec'.])
fi

# Checks for library functions.
AM_GNU_GETTEXT([external])
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
                posix_fadvise \
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

have_tcmalloc=no
if test "x$with_tcmalloc" = "xyes"; then
  dnl Important: put malloc libs at the very end.
  dnl Only newish versions have a .pc, thus try CHECK_LIB as well.
  PKG_CHECK_MODULES([TCMALLOC], [libtcmalloc_minimal], [have_tcmalloc=yes], [have_tcmalloc=no])
  if test "x$have_tcmalloc" != "xyes"; then
    AC_CHECK_LIB([tcmalloc_minimal], [malloc], [have_tcmalloc=yes], [have_tcmalloc=no])
    if test "x$have_tcmalloc" = "xyes"; then
      TCMALLOC_CFLAGS=
      TCMALLOC_LIBS="-ltcmalloc_minimal"
      AC_SUBST([TCMALLOC_CFLAGS])
      AC_SUBST([TCMALLOC_LIBS])
    else
      if test "x$with_tcmalloc_requested" = "xyes"; then
        ARIA2_DEP_NOT_MET([tcmalloc_minimal])
      fi
    fi
  fi
fi

have_jemalloc=no
if test "x$with_jemalloc" = "xyes"; then
  dnl Important: put malloc libs at the very end.
  dnl Usually jemalloc does not come with a .pc, as the official source does not
  dnl generate one.
  PKG_CHECK_MODULES([JEMALLOC], [jemalloc], [have_jemalloc=yes], [have_jemalloc=no])
  if test "x$have_jemalloc" != "xyes"; then
    AC_CHECK_LIB([jemalloc], [malloc], [have_jemalloc=yes], [have_jemalloc=no])
    if test "x$have_jemalloc" = "xyes"; then
      JEMALLOC_CFLAGS=
      JEMALLOC_LIBS="-ljemalloc"
      AC_SUBST([JEMALLOC_CFLAGS])
      AC_SUBST([JEMALLOC_LIBS])
    else
      if test "x$with_jemalloc_requested" = "xyes"; then
        ARIA2_DEP_NOT_MET([jemalloc (unprefixed)])
      fi
    fi
  fi
fi

have_epoll=no
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
   test "x$have_osx" = "xyes" ||
   test "x$win_build" = "xyes"; then
  AC_DEFINE([HAVE_SOME_FALLOCATE], [1],
            [Define to 1 if *_fallocate is available.])
fi
AM_CONDITIONAL([HAVE_SOME_FALLOCATE],
  [test "x$have_posix_fallocate" = "xyes" || test "x$have_fallocate" = "xyes" \
  || test "x$have_osx" = "xyes" || test "x$win_build" = "xyes"])

# mingw needs this
save_CPPFLAGS=$CPPFLAGS
CPPFLAGS="$CPPFLAGS $EXTRACPPFLAGS"
AC_MSG_CHECKING([for asctime_r])
AC_LINK_IFELSE([AC_LANG_PROGRAM([[
    #include <time.h>
  ]], [[
    struct tm r; char *c;
    asctime_r(&r, c);
  ]])],
  [AM_CONDITIONAL([HAVE_ASCTIME_R], true)
   AC_MSG_RESULT([yes])
   AC_DEFINE([HAVE_ASCTIME_R], [1], [Define to 1 if you have the `asctime_r' function or macro.])],
  [AC_MSG_RESULT([no])
   AM_CONDITIONAL([HAVE_ASCTIME_R], false)])

AC_MSG_CHECKING([for localtime_r])
AC_LINK_IFELSE([AC_LANG_PROGRAM([[
    #include <time.h>
  ]], [[
    time_t t; struct tm r;
    localtime_r(&t, &r);
  ]])],
  [AM_CONDITIONAL([HAVE_LOCALTIME_R], true)
   AC_MSG_RESULT([yes])
   AC_DEFINE([HAVE_LOCALTIME_R], [1], [Define to 1 if you have the `localtime_r' function or macro.])],
  [AC_MSG_RESULT([no])
   AM_CONDITIONAL([HAVE_LOCALTIME_R], false)])
CPPFLAGS=$save_CPPFLAGS

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
AC_CHECK_FUNCS([strptime],
        [AM_CONDITIONAL([HAVE_STRPTIME], true)],
        [AM_CONDITIONAL([HAVE_STRPTIME], false)])
AC_CHECK_FUNCS([daemon], [have_daemon=yes])
AM_CONDITIONAL([HAVE_DAEMON], [test "x$have_daemon" = "xyes"])

AC_CHECK_FUNCS([poll], [have_poll=yes])
AM_CONDITIONAL([HAVE_POLL], [test "x$have_poll" = "xyes"])

case "$host" in
  *mingw*)
    AM_CONDITIONAL([MINGW_BUILD], true)
    dnl defined in ws2tcpip.h, but only if _WIN32_WINNT >= 0x0501
    AM_CONDITIONAL([HAVE_GETADDRINFO], true)
    dnl defined in ws2tcpip.h, but missing in C:\mingw\lib\libws2_32.a
    AM_CONDITIONAL([HAVE_GAI_STRERROR], false)
    ;;
  *)
    AM_CONDITIONAL([MINGW_BUILD], false)
    ;;
esac

case "$host" in
  *android*)
    # timegm is not declared with Android NDK r12b
    AM_CONDITIONAL([HAVE_TIMEGM], false)
    ;;
  *)
    AC_CHECK_FUNCS([timegm],
        [AM_CONDITIONAL([HAVE_TIMEGM], true)],
        [AM_CONDITIONAL([HAVE_TIMEGM], false)])
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

if test "x$enable_websocket" = "xyes"; then
  AC_CONFIG_SUBDIRS([deps/wslay])
  enable_websocket=yes
  AC_DEFINE([ENABLE_WEBSOCKET], [1],
            [Define 1 if WebSocket support is enabled.])
  # $(top_srcdir) for `make distcheck`
  WSLAY_CFLAGS="-I\$(top_builddir)/deps/wslay/lib/includes -I\$(top_srcdir)/deps/wslay/lib/includes"
  WSLAY_LIBS="\$(top_builddir)/deps/wslay/lib/libwslay.la"
  AC_SUBST([WSLAY_CFLAGS])
  AC_SUBST([WSLAY_LIBS])
fi
AM_CONDITIONAL([ENABLE_WEBSOCKET], [test "x$enable_websocket" = "xyes"])

AM_CONDITIONAL([ENABLE_LIBARIA2], [test "x$enable_libaria2" = "xyes"])

AC_SUBST([bashcompletiondir])

if test "x$ARIA2_STATIC" = "xyes"; then
  # -static-libgcc and -static-libstdc++ are linker flags and not for
  # libtool.
  EXTRALDFLAGS="$EXTRALDFLAGS -all-static"
  dnl For non-MinGW build, we need additional libs for static build.
  case "$host" in
    *mingw*|*msvc*|*darwin*)
    ;;

    *)
      EXTRALIBS="$EXTRALIBS -lpthread -ldl -lrt"
    ;;
  esac
fi

if test "x$win_build" = "xyes" && test "x$enable_libaria2" = "xyes"; then
  # Creating dll needs this
  EXTRALDFLAGS="$EXTRALDFLAGS -no-undefined"
fi

AC_SUBST([EXTRACFLAGS])
AC_SUBST([EXTRACXXFLAGS])
AC_SUBST([EXTRACPPFLAGS])
AC_SUBST([EXTRALDFLAGS])
AC_SUBST([EXTRALIBS])

save_CXXFLAGS=$CXXFLAGS
CXXFLAGS=

if test "x$enable_werror" = "xyes"; then
  AX_CHECK_COMPILE_FLAG([-Wall], [CXXFLAGS="$CXXFLAGS -Wall"])
  AX_CHECK_COMPILE_FLAG([-Werror], [CXXFLAGS="$CXXFLAGS -Werror"])
  AX_CHECK_COMPILE_FLAG([-Wformat-security], [CXXFLAGS="$CXXFLAGS -Wformat-security"])
fi

WARNCXXFLAGS=$CXXFLAGS
CXXFLAGS=$save_CXXFLAGS

AC_SUBST([WARNCXXFLAGS])

AC_CONFIG_FILES([Makefile
                src/Makefile
                src/libaria2.pc
                src/includes/Makefile
                test/Makefile
                po/Makefile.in
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

AC_MSG_NOTICE([summary of build options:

Build:          $build
Host:           $host
Target:         $target
Install prefix: $prefix
CC:             $CC
CXX:            $CXX
CPP:            $CPP
CXXFLAGS:       $CXXFLAGS
CFLAGS:         $CFLAGS
CPPFLAGS:       $CPPFLAGS
LDFLAGS:        $LDFLAGS
LIBS:           $LIBS
DEFS:           $DEFS
CXX1XCXXFLAGS:  $CXX1XCXXFLAGS
EXTRACXXFLAGS:  $EXTRACXXFLAGS
EXTRACFLAGS:    $EXTRACFLAGS
EXTRACPPFLAGS:  $EXTRACPPFLAGS
EXTRALDFLAGS:   $EXTRALDFLAGS
EXTRALIBS:      $EXTRALIBS
WARNCXXFLAGS:   $WARNCXXFLAGS
LibUV:          $have_libuv (CFLAGS='$LIBUV_CFLAGS' LIBS='$LIBUV_LIBS')
SQLite3:        $have_sqlite3 (CFLAGS='$SQLITE3_CFLAGS' LIBS='$SQLITE3_LIBS')
SSL Support:    $have_ssl
AppleTLS:       $have_appletls (LDFLAGS='$APPLETLS_LDFLAGS')
WinTLS:         $have_wintls (LIBS='$WINTLS_LIBS')
GnuTLS:         $have_libgnutls (CFLAGS='$LIBGNUTLS_CFLAGS' LIBS='$LIBGNUTLS_LIBS')
OpenSSL:        $have_openssl (CFLAGS='$OPENSSL_CFLAGS' LIBS='$OPENSSL_LIBS')
CA Bundle:      $ca_bundle
LibNettle:      $have_libnettle (CFLAGS='$LIBNETTLE_CFLAGS' LIBS='$LIBNETTLE_LIBS')
LibGmp:         $have_libgmp (CFLAGS='$LIBGMP_CFLAGS' LIBS='$LIBGMP_LIBS')
LibGcrypt:      $have_libgcrypt (CFLAGS='$LIBGCRYPT_CFLAGS' LIBS='$LIBGCRYPT_LIBS')
LibXML2:        $have_libxml2 (CFLAGS='$LIBXML2_CFLAGS' LIBS='$LIBXML2_LIBS')
LibExpat:       $have_libexpat (CFLAGS='$EXPAT_CFLAGS' LIBS='$EXPAT_LIBS')
LibCares:       $have_libcares (CFLAGS='$LIBCARES_CFLAGS' LIBS='$LIBCARES_LIBS')
Zlib:           $have_zlib (CFLAGS='$ZLIB_CFLAGS' LIBS='$ZLIB_LIBS')
Libssh2:        $have_libssh2 (CFLAGS='$LIBSSH2_CFLAGS' LIBS='$LIBSSH2_LIBS')
Tcmalloc:       $have_tcmalloc (CFLAGS='$TCMALLOC_CFLAGS' LIBS='$TCMALLOC_LIBS')
Jemalloc:       $have_jemalloc (CFLAGS='$JEMALLOC_CFLAGS' LIBS='$JEMALLOC_LIBS')
Epoll:          $have_epoll
Bittorrent:     $enable_bittorrent
Metalink:       $enable_metalink
XML-RPC:        $enable_xml_rpc
Message Digest: $use_md
WebSocket:      $enable_websocket (CFLAGS='$WSLAY_CFLAGS' LIBS='$WSLAY_LIBS')
Libaria2:       $enable_libaria2 (shared=${enable_shared} static=${enable_static})
bash_completion dir: $bashcompletiondir
Static build:   $ARIA2_STATIC
])
