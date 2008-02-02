# longlong.m4 serial 13
dnl Copyright (C) 1999-2007 Free Software Foundation, Inc.
dnl This file is free software; the Free Software Foundation
dnl gives unlimited permission to copy and/or distribute it,
dnl with or without modifications, as long as this notice is preserved.

dnl From Paul Eggert.

# Define HAVE_LONG_LONG_INT if 'long long int' works.
# This fixes a bug in Autoconf 2.61, but can be removed once we
# assume 2.62 everywhere.

# Note: If the type 'long long int' exists but is only 32 bits large
# (as on some very old compilers), HAVE_LONG_LONG_INT will not be
# defined. In this case you can treat 'long long int' like 'long int'.

AC_DEFUN([AC_TYPE_LONG_LONG_INT],
[
  AC_CACHE_CHECK([for long long int], [ac_cv_type_long_long_int],
    [AC_LINK_IFELSE(
       [_AC_TYPE_LONG_LONG_SNIPPET],
       [dnl This catches a bug in Tandem NonStop Kernel (OSS) cc -O circa 2004.
	dnl If cross compiling, assume the bug isn't important, since
	dnl nobody cross compiles for this platform as far as we know.
	AC_RUN_IFELSE(
	  [AC_LANG_PROGRAM(
	     [[@%:@include <limits.h>
	       @%:@ifndef LLONG_MAX
	       @%:@ define HALF \
			(1LL << (sizeof (long long int) * CHAR_BIT - 2))
	       @%:@ define LLONG_MAX (HALF - 1 + HALF)
	       @%:@endif]],
	     [[long long int n = 1;
	       int i;
	       for (i = 0; ; i++)
		 {
		   long long int m = n << i;
		   if (m >> i != n)
		     return 1;
		   if (LLONG_MAX / 2 < m)
		     break;
		 }
	       return 0;]])],
	  [ac_cv_type_long_long_int=yes],
	  [ac_cv_type_long_long_int=no],
	  [ac_cv_type_long_long_int=yes])],
       [ac_cv_type_long_long_int=no])])
  if test $ac_cv_type_long_long_int = yes; then
    AC_DEFINE([HAVE_LONG_LONG_INT], 1,
      [Define to 1 if the system has the type `long long int'.])
  fi
])

# Define HAVE_UNSIGNED_LONG_LONG_INT if 'unsigned long long int' works.
# This fixes a bug in Autoconf 2.61, but can be removed once we
# assume 2.62 everywhere.

# Note: If the type 'unsigned long long int' exists but is only 32 bits
# large (as on some very old compilers), AC_TYPE_UNSIGNED_LONG_LONG_INT
# will not be defined. In this case you can treat 'unsigned long long int'
# like 'unsigned long int'.

AC_DEFUN([AC_TYPE_UNSIGNED_LONG_LONG_INT],
[
  AC_CACHE_CHECK([for unsigned long long int],
    [ac_cv_type_unsigned_long_long_int],
    [AC_LINK_IFELSE(
       [_AC_TYPE_LONG_LONG_SNIPPET],
       [ac_cv_type_unsigned_long_long_int=yes],
       [ac_cv_type_unsigned_long_long_int=no])])
  if test $ac_cv_type_unsigned_long_long_int = yes; then
    AC_DEFINE([HAVE_UNSIGNED_LONG_LONG_INT], 1,
      [Define to 1 if the system has the type `unsigned long long int'.])
  fi
])

# Expands to a C program that can be used to test for simultaneous support
# of 'long long' and 'unsigned long long'. We don't want to say that
# 'long long' is available if 'unsigned long long' is not, or vice versa,
# because too many programs rely on the symmetry between signed and unsigned
# integer types (excluding 'bool').
AC_DEFUN([_AC_TYPE_LONG_LONG_SNIPPET],
[
  AC_LANG_PROGRAM(
    [[/* Test preprocessor.  */
      #if ! (-9223372036854775807LL < 0 && 0 < 9223372036854775807ll)
        error in preprocessor;
      #endif
      #if ! (18446744073709551615ULL <= -1ull)
        error in preprocessor;
      #endif
      /* Test literals.  */
      long long int ll = 9223372036854775807ll;
      long long int nll = -9223372036854775807LL;
      unsigned long long int ull = 18446744073709551615ULL;
      /* Test constant expressions.   */
      typedef int a[((-9223372036854775807LL < 0 && 0 < 9223372036854775807ll)
		     ? 1 : -1)];
      typedef int b[(18446744073709551615ULL <= (unsigned long long int) -1
		     ? 1 : -1)];
      int i = 63;]],
    [[/* Test availability of runtime routines for shift and division.  */
      long long int llmax = 9223372036854775807ll;
      unsigned long long int ullmax = 18446744073709551615ull;
      return ((ll << 63) | (ll >> 63) | (ll < i) | (ll > i)
	      | (llmax / ll) | (llmax % ll)
	      | (ull << 63) | (ull >> 63) | (ull << i) | (ull >> i)
	      | (ullmax / ull) | (ullmax % ull));]])
])
