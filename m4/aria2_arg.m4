dnl ARIA2_ARG_WITH(PACKAGE)
dnl wrapper for AC_ARG_WITH with default value 'yes'.
AC_DEFUN([ARIA2_ARG_WITH],
[AC_ARG_WITH([$1],
	AS_HELP_STRING([--with-$1], [Use $1 if it is installed.]),
	[with_$1=$withval], [with_$1=yes])]
)

dnl ARIA2_ARG_ENABLE(FEATURE)
dnl wrapper for AC_ARG_ENABLE with default value 'yes'.
AC_DEFUN([ARIA2_ARG_ENABLE],
[AC_ARG_ENABLE([$1],
	AS_HELP_STRING([--enable-$1], [Enable $1 support.]),
	[enable_$1=$enableval], [enable_$1=yes])]
)
