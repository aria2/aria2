dnl Wslay - The WebSocket Library

dnl Copyright (c) 2011, 2012 Tatsuhiro Tsujikawa

dnl Permission is hereby granted, free of charge, to any person obtaining
dnl a copy of this software and associated documentation files (the
dnl "Software"), to deal in the Software without restriction, including
dnl without limitation the rights to use, copy, modify, merge, publish,
dnl distribute, sublicense, and/or sell copies of the Software, and to
dnl permit persons to whom the Software is furnished to do so, subject to
dnl the following conditions:

dnl The above copyright notice and this permission notice shall be
dnl included in all copies or substantial portions of the Software.

dnl THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
dnl EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
dnl MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
dnl NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE
dnl LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
dnl OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
dnl WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
AC_PREREQ(2.61)
AC_INIT([wslay], [1.0.1-DEV], [t-tujikawa@users.sourceforge.net])
LT_PREREQ([2.2.6])
LT_INIT()
AC_CONFIG_AUX_DIR([.])
dnl See versioning rule:
dnl  http://www.gnu.org/software/libtool/manual/html_node/Updating-version-info.html
AC_SUBST(LT_CURRENT, 0)
AC_SUBST(LT_REVISION, 1)
AC_SUBST(LT_AGE, 0)

AC_CANONICAL_BUILD
AC_CANONICAL_HOST
AC_CANONICAL_TARGET

AC_CONFIG_MACRO_DIR([m4])

AM_INIT_AUTOMAKE()
AC_CONFIG_HEADERS([config.h])

dnl Checks for programs
AC_PROG_CC
AC_PROG_CXX
AC_PROG_INSTALL
AC_PROG_LN_S
AC_PROG_MAKE_SET
PKG_PROG_PKG_CONFIG([0.20])

AC_PATH_PROG([SPHINX_BUILD], [sphinx-build])
AC_SUBST([SPHINX_BUILD])
AM_CONDITIONAL([HAVE_SPHINX_BUILD], [ test "x$SPHINX_BUILD" != "x" ])

# Checks for libraries.

# Nettle used in examples
PKG_CHECK_MODULES([NETTLE], [nettle >= 2.4], [have_nettle=yes], [have_nettle=no])

AC_CHECK_LIB([cunit], [CU_initialize_registry],
             [have_cunit=yes], [have_cunit=no])
AM_CONDITIONAL([HAVE_CUNIT], [ test "x${have_cunit}" = "xyes" ])

case "$target" in
  *mingw*)
    # Required for ntoh*/hton* functions.
    LIBS="-lws2_32 $LIBS"
    ;;
esac

# Checks for header files.
AC_CHECK_HEADERS([ \
  arpa/inet.h \
  netinet/in.h \
  stddef.h \
  stdint.h \
  stdlib.h \
  string.h \
  unistd.h \
])

# Need winsock2.h for ntoh*/hton* functions.
case "$target" in
  *mingw*)
    AC_CHECK_HEADERS([winsock2.h])
    ;;
esac

# Checks for typedefs, structures, and compiler characteristics.
AC_TYPE_SIZE_T
AC_TYPE_SSIZE_T
AC_TYPE_UINT8_T
AC_TYPE_UINT16_T
AC_TYPE_UINT32_T
AC_TYPE_UINT64_T
AC_CHECK_TYPES([ptrdiff_t])
AC_C_BIGENDIAN

# Checks for library functions.
AC_CHECK_FUNCS([ \
  memmove \
  memset \
  ntohl \
  ntohs \
  htons
])

build_examples=no
if test "x${have_nettle}" = "xyes"; then
  case $host_os in
    linux*)
      # the examples uses epoll
      build_examples=yes
      ;;
  esac
fi
AM_CONDITIONAL([ENABLE_EXAMPLES], [ test "x${build_examples}" = "xyes" ])

AC_CONFIG_FILES([
  Makefile
  lib/Makefile
  lib/libwslay.pc
  lib/includes/Makefile
  lib/includes/wslay/wslayver.h
  tests/Makefile
  examples/Makefile
  doc/Makefile
  doc/sphinx/conf.py
])
AC_OUTPUT

AC_MSG_NOTICE([summary of build options:

    version:        ${VERSION} shared $LT_CURRENT:$LT_REVISION:$LT_AGE
    Host type:      ${host}
    Install prefix: ${prefix}
    C compiler:     ${CC}
    CFlags:         ${CFLAGS}
    Library types:  Shared=${enable_shared}, Static=${enable_static}
    CUnit:          ${have_cunit}
    Nettle:         ${have_nettle}
    Build examples: ${build_examples}
])
