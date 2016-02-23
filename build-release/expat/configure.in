dnl   configuration script for expat
dnl   Process this file with autoconf to produce a configure script.
dnl
dnl   Copyright 2000 Clark Cooper
dnl
dnl   This file is part of EXPAT.
dnl
dnl   EXPAT is free software; you can redistribute it and/or modify it
dnl   under the terms of the License (based on the MIT/X license) contained
dnl   in the file COPYING that comes with this distribution.
dnl

dnl Ensure that Expat is configured with autoconf 2.58 or newer
AC_PREREQ(2.58)

dnl Get the version number of Expat, using m4's esyscmd() command to run
dnl the command at m4-generation time. This allows us to create an m4
dnl symbol holding the correct version number. AC_INIT() requires the
dnl version number at m4-time, rather than when ./configure is run, so
dnl all this must happen as part of m4, not as part of the shell code
dnl contained in ./configure.
dnl
dnl NOTE: esyscmd() is a GNU M4 extension. Thus, we wrap it in an appropriate
dnl test. I believe this test will work, but I don't have a place with non-
dnl GNU M4 to test it right now.
define([expat_version], ifdef([__gnu__],
                              [esyscmd(conftools/get-version.sh lib/expat.h)],
                              [2.1.x]))
AC_INIT(expat, expat_version, expat-bugs@libexpat.org)
undefine([expat_version])

AC_CONFIG_SRCDIR(Makefile.in)
AC_CONFIG_AUX_DIR(conftools)
AC_CONFIG_MACRO_DIR([m4])


dnl
dnl Increment LIBREVISION if source code has changed at all
dnl
dnl If the API has changed, increment LIBCURRENT and set LIBREVISION to 0
dnl
dnl If the API changes compatibly (i.e. simply adding a new function
dnl without changing or removing earlier interfaces), then increment LIBAGE.
dnl 
dnl If the API changes incompatibly set LIBAGE back to 0
dnl

LIBCURRENT=7
LIBREVISION=0
LIBAGE=6

AC_CONFIG_HEADER(expat_config.h)

sinclude(conftools/ac_c_bigendian_cross.m4)

AC_LIBTOOL_WIN32_DLL
AC_PROG_LIBTOOL

AC_SUBST(LIBCURRENT)
AC_SUBST(LIBREVISION)
AC_SUBST(LIBAGE)

dnl Checks for programs.
AC_PROG_CC
AC_PROG_CXX
AC_PROG_INSTALL

if test "$GCC" = yes ; then
    dnl
    dnl Be careful about adding the -fexceptions option; some versions of
    dnl GCC don't support it and it causes extra warnings that are only
    dnl distracting; avoid.
    dnl
    OLDCFLAGS="$CFLAGS -Wall -Wmissing-prototypes -Wstrict-prototypes"
    CFLAGS="$OLDCFLAGS -fexceptions"
    AC_MSG_CHECKING(whether $CC accepts -fexceptions)
    AC_TRY_LINK( , ,
                   AC_MSG_RESULT(yes),
                   AC_MSG_RESULT(no); CFLAGS="$OLDCFLAGS")
    CXXFLAGS=`echo "$CFLAGS" | sed 's/ -Wmissing-prototypes -Wstrict-prototypes//'`
fi

dnl Checks for header files.
AC_HEADER_STDC

dnl Checks for typedefs, structures, and compiler characteristics.

dnl Note: Avoid using AC_C_BIGENDIAN because it does not
dnl work in a cross compile.
AC_C_BIGENDIAN_CROSS

AC_C_CONST
AC_TYPE_SIZE_T
AC_CHECK_FUNCS(memmove bcopy)

dnl Only needed for xmlwf:
AC_CHECK_HEADERS(fcntl.h unistd.h)
AC_TYPE_OFF_T
AC_FUNC_MMAP

if test "$ac_cv_func_mmap_fixed_mapped" = "yes"; then
    FILEMAP=unixfilemap
else
    FILEMAP=readfilemap
fi
AC_SUBST(FILEMAP)

dnl Needed for the test support code; this was found at
dnl http://lists.gnu.org/archive/html/bug-autoconf/2002-07/msg00028.html

# AC_CPP_FUNC
# ------------------ #
# Checks to see if ANSI C99 CPP variable __func__ works.
# If not, perhaps __FUNCTION__ works instead. 
# If not, we'll just define __func__ to "". 
AC_DEFUN([AC_CPP_FUNC],
[AC_REQUIRE([AC_PROG_CC_STDC])dnl
AC_CACHE_CHECK([for an ANSI C99-conforming __func__], ac_cv_cpp_func,
[AC_COMPILE_IFELSE([AC_LANG_PROGRAM([],
[[char *foo = __func__;]])],
  [ac_cv_cpp_func=yes], 
  [AC_COMPILE_IFELSE([AC_LANG_PROGRAM([],
[[char *foo = __FUNCTION__;]])],
  [ac_cv_cpp_func=__FUNCTION__], 
  [ac_cv_cpp_func=no])])])
if test $ac_cv_cpp_func = __FUNCTION__; then
  AC_DEFINE(__func__,__FUNCTION__,
            [Define to __FUNCTION__ or "" if `__func__' does not conform to 
ANSI C.])
elif test $ac_cv_cpp_func = no; then
  AC_DEFINE(__func__,"",
            [Define to __FUNCTION__ or "" if `__func__' does not conform to 
ANSI C.])
fi
])# AC_CPP_FUNC

AC_CPP_FUNC


dnl Some basic configuration:
AC_DEFINE([XML_NS], 1,
          [Define to make XML Namespaces functionality available.])
AC_DEFINE([XML_DTD], 1,
          [Define to make parameter entity parsing functionality available.])
AC_DEFINE([XML_CONTEXT_BYTES], 1024,
          [Define to specify how much context to retain around the current parse point.])

AC_CONFIG_FILES([Makefile expat.pc])
AC_OUTPUT

abs_srcdir="`cd $srcdir && pwd`"
abs_builddir="`pwd`"
if test "$abs_srcdir" != "$abs_builddir"; then
  make mkdir-init
fi
