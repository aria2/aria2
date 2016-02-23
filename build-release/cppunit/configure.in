dnl Process this file with autoconf to produce a configure script.
AC_INIT(Makefile.am)

# autoconf 2.50 or higher to rebuild aclocal.m4, because the
# AC_CREATE_PREFIX_CONFIG_H macro needs the AS_DIRNAME macro.
AC_PREREQ(2.50)

# Making releases:
#   CPPUNIT_MICRO_VERSION += 1;
#   CPPUNIT_INTERFACE_AGE += 1;
#   CPPUNIT_BINARY_AGE += 1;
# if any functions have been added, set CPPUNIT_INTERFACE_AGE to 0.
# if backwards compatibility has been broken,
# set CPPUNIT_BINARY_AGE and CPPUNIT_INTERFACE_AGE to 0.
#
CPPUNIT_MAJOR_VERSION=1
CPPUNIT_MINOR_VERSION=12
CPPUNIT_MICRO_VERSION=1
CPPUNIT_INTERFACE_AGE=0
CPPUNIT_BINARY_AGE=0
CPPUNIT_VERSION=$CPPUNIT_MAJOR_VERSION.$CPPUNIT_MINOR_VERSION.$CPPUNIT_MICRO_VERSION
AC_SUBST(CPPUNIT_MAJOR_VERSION)
AC_SUBST(CPPUNIT_MINOR_VERSION)
AC_SUBST(CPPUNIT_MICRO_VERSION)
AC_SUBST(CPPUNIT_INTERFACE_AGE)
AC_SUBST(CPPUNIT_BINARY_AGE)
AC_SUBST(CPPUNIT_VERSION)

# libtool versioning
LT_RELEASE=$CPPUNIT_MAJOR_VERSION.$CPPUNIT_MINOR_VERSION
LT_CURRENT=`expr $CPPUNIT_MICRO_VERSION - $CPPUNIT_INTERFACE_AGE`
LT_REVISION=$CPPUNIT_INTERFACE_AGE
LT_AGE=`expr $CPPUNIT_BINARY_AGE - $CPPUNIT_INTERFACE_AGE`
AC_SUBST(LT_RELEASE)
AC_SUBST(LT_CURRENT)
AC_SUBST(LT_REVISION)
AC_SUBST(LT_AGE)

AC_CONFIG_AUX_DIR(config)
AM_CONFIG_HEADER(config/config.h)
AM_INIT_AUTOMAKE(cppunit, $CPPUNIT_VERSION)

# General "with" options
# ----------------------------------------------------------------------------

dnl Checks for programs.

AC_PROG_MAKE_SET
AC_PROG_INSTALL

# The libtool macro AC_PROG_LIBTOOL checks for the C preprocessor.
# Configure gets confused if we try to check for a C preprocessor
# without first checking for the C compiler
# (see http://sources.redhat.com/ml/autoconf/2001-07/msg00036.html),
# so we invoke AC_PROG_CC explicitly.
AC_PROG_CC
AC_PROG_CXX
AC_LANG(C++)

AC_LIBTOOL_WIN32_DLL
AC_PROG_LIBTOOL

# check for dlopen,dlsym... or shl_load, shl_findsym...
AC_LTDL_DLLIB

# check for doxygen
BB_ENABLE_DOXYGEN


# Check for headers
# Note that the fourth argument to AC_CHECK_HEADERS is non-empty to force
# the configure probe to try compiling "#include <header>".  See autoconf docs.
# ----------------------------------------------------------------------------

AC_CHECK_HEADERS(cmath,[],[],[/**/])

# Check for compiler characteristics 
# ----------------------------------------------------------------------------

AC_CXX_RTTI
AX_CXX_GCC_ABI_DEMANGLE
AC_CXX_STRING_COMPARE_STRING_FIRST


# Check for library functions
# ----------------------------------------------------------------------------
AC_CXX_HAVE_SSTREAM
AC_CXX_HAVE_STRSTREAM
AX_CXX_HAVE_ISFINITE
AC_CHECK_FUNCS(finite)

cppunit_val='CPPUNIT_HAVE_RTTI'
AC_ARG_ENABLE(typeinfo-name,
[  --disable-typeinfo-name disable use of RTTI for class names],
[  
    test x$enableval = 'xno' && cppunit_val='0' 
])

AC_DEFINE_UNQUOTED(USE_TYPEINFO_NAME,$cppunit_val,
[Define to 1 to use type_info::name() for class names])


# Doesn't work. It's supposed to add "#define CPPUNIT_NO_TESTPLUGIN" if
# --disable-test-plugin was used on the command line.
#
# 
#
#AC_ARG_ENABLE(test-plugin,
#[  --disable-test-plugin disable support for test plug-ins],
#[  
#	if test -n "$enable_test_plugin"; then
#	   enable_test_plugin=${enable_test_plugin_default-yes}
#	fi
#	if test "$enable_test_plugin" = no; then
#echo "test-plug in disabled"
#	fi
#])
#
#testplugin_val=1
#AC_DEFINE_UNQUOTED(NO_TESTPLUGIN,$testplugin_val,
#[defined to disable TestPlugIn])


AC_OUTPUT([
  Makefile 
  cppunit.pc
  cppunit.spec
  cppunit-config
  src/Makefile
  src/DllPlugInTester/Makefile
  src/cppunit/Makefile
  include/Makefile
  include/cppunit/Makefile
  include/cppunit/config/Makefile
  include/cppunit/extensions/Makefile
  include/cppunit/plugin/Makefile
  include/cppunit/portability/Makefile
  include/cppunit/tools/Makefile
  include/cppunit/ui/Makefile
  include/cppunit/ui/mfc/Makefile
  include/cppunit/ui/qt/Makefile
  include/cppunit/ui/text/Makefile
  doc/Makefile
  doc/Doxyfile
  examples/Makefile
  examples/simple/Makefile
  examples/hierarchy/Makefile
  examples/cppunittest/Makefile
  examples/ClockerPlugIn/Makefile
  examples/DumperPlugIn/Makefile
  examples/money/Makefile
],[chmod a+x cppunit-config])

AC_CREATE_PREFIX_CONFIG_H([include/cppunit/config-auto.h], 
        $PACKAGE, [config/config.h])
