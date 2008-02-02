# wchar_t.m4 serial 1 (gettext-0.12)
dnl Copyright (C) 2002-2003 Free Software Foundation, Inc.
dnl This file is free software; the Free Software Foundation
dnl gives unlimited permission to copy and/or distribute it,
dnl with or without modifications, as long as this notice is preserved.

dnl From Bruno Haible.
dnl Test whether <stddef.h> has the 'wchar_t' type.
dnl Prerequisite: AC_PROG_CC

AC_DEFUN([gt_TYPE_WCHAR_T],
[
  AC_CACHE_CHECK([for wchar_t], gt_cv_c_wchar_t,
    [AC_TRY_COMPILE([#include <stddef.h>
       wchar_t foo = (wchar_t)'\0';], ,
       gt_cv_c_wchar_t=yes, gt_cv_c_wchar_t=no)])
  if test $gt_cv_c_wchar_t = yes; then
    AC_DEFINE(HAVE_WCHAR_T, 1, [Define if you have the 'wchar_t' type.])
  fi
])
