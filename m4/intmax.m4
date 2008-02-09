# intmax.m4 serial 3 (gettext-0.16)
dnl Copyright (C) 2002-2005 Free Software Foundation, Inc.
dnl This file is free software; the Free Software Foundation
dnl gives unlimited permission to copy and/or distribute it,
dnl with or without modifications, as long as this notice is preserved.

dnl From Bruno Haible.
dnl Test whether the system has the 'intmax_t' type, but don't attempt to
dnl find a replacement if it is lacking.

AC_DEFUN([gt_TYPE_INTMAX_T],
[
  AC_REQUIRE([gl_AC_HEADER_INTTYPES_H])
  AC_REQUIRE([gl_AC_HEADER_STDINT_H])
  AC_CACHE_CHECK(for intmax_t, gt_cv_c_intmax_t,
    [AC_TRY_COMPILE([
#include <stddef.h>
#include <stdlib.h>
#if HAVE_STDINT_H_WITH_UINTMAX
#include <stdint.h>
#endif
#if HAVE_INTTYPES_H_WITH_UINTMAX
#include <inttypes.h>
#endif
],     [intmax_t x = -1;
        return !x;],
       gt_cv_c_intmax_t=yes,
       gt_cv_c_intmax_t=no)])
  if test $gt_cv_c_intmax_t = yes; then
    AC_DEFINE(HAVE_INTMAX_T, 1,
      [Define if you have the 'intmax_t' type in <stdint.h> or <inttypes.h>.])
  fi
])
