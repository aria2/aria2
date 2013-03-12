dnl ARIA2_ARG_WITH(PACKAGE)
dnl wrapper for AC_ARG_WITH with default value 'no'.
dnl If --with-$1 is given explicitly, set with_$1_requested to given value.
AC_DEFUN([ARIA2_ARG_WITH],
[AC_ARG_WITH([$1],
	AS_HELP_STRING([--with-$1], [Use $1.]),
	[with_$1_requested=$withval with_$1=$withval], [with_$1=no])]
)
dnl ARIA2_ARG_WITHOUT(PACKAGE)
dnl wrapper for AC_ARG_WITH with default value 'yes'.
dnl If --with-$1 is given explicitly, set with_$1_requested to given value.
AC_DEFUN([ARIA2_ARG_WITHOUT],
[AC_ARG_WITH([$1],
	AS_HELP_STRING([--without-$1], [Do not use $1. [default=check]]),
	[with_$1_requested=$withval with_$1=$withval], [with_$1=yes])]
)

dnl ARIA2_ARG_ENABLE(FEATURE)
dnl wrapper for AC_ARG_ENABLE with default value 'no'.
dnl If --enable-$1 is given explicitly, set enable_$1_requested to given value.
AC_DEFUN([ARIA2_ARG_ENABLE],
[AC_ARG_ENABLE([$1],
	AS_HELP_STRING([--enable-$1], [Enable $1 support.]),
	[enable_$1_requested=$enableval enable_$1=$enableval], [enable_$1=no])]
)
dnl ARIA2_ARG_DISABLE(FEATURE)
dnl wrapper for AC_ARG_ENABLE with default value 'yes'.
dnl If --enable-$1 is given explicitly, set enable_$1_requested to given value.
AC_DEFUN([ARIA2_ARG_DISABLE],
[AC_ARG_ENABLE([$1],
  AS_HELP_STRING([--disable-$1], [Disable $1 support. [default=check]]),
	[enable_$1_requested=$enableval enable_$1=$enableval], [enable_$1=yes])]
)

dnl ARIA2_DEP_NOT_MET(PACKAGE)
dnl Show error message PACKAGE is missing and exit.
AC_DEFUN([ARIA2_DEP_NOT_MET],
[AC_MSG_FAILURE([$1 is requested but not found in the system.])])

dnl ARIA2_FET_NOT_SUPPORTED(FEATURE)
dnl Show error message FEATURE can not be enabled and exit.
AC_DEFUN([ARIA2_FET_NOT_SUPPORTED],
[AC_MSG_FAILURE([$1 is requested but cannot be enabled with current\
 configuration.\
 Make sure that dependent libraries are installed and configure script options\
 are correct.])])
