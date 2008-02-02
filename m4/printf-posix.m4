# printf-posix.m4 serial 3 (gettext-0.17)
dnl Copyright (C) 2003, 2007 Free Software Foundation, Inc.
dnl This file is free software; the Free Software Foundation
dnl gives unlimited permission to copy and/or distribute it,
dnl with or without modifications, as long as this notice is preserved.

dnl From Bruno Haible.
dnl Test whether the printf() function supports POSIX/XSI format strings with
dnl positions.

AC_DEFUN([gt_PRINTF_POSIX],
[
  AC_REQUIRE([AC_PROG_CC])
  AC_CACHE_CHECK([whether printf() supports POSIX/XSI format strings],
    gt_cv_func_printf_posix,
    [
      AC_TRY_RUN([
#include <stdio.h>
#include <string.h>
/* The string "%2$d %1$d", with dollar characters protected from the shell's
   dollar expansion (possibly an autoconf bug).  */
static char format[] = { '%', '2', '$', 'd', ' ', '%', '1', '$', 'd', '\0' };
static char buf[100];
int main ()
{
  sprintf (buf, format, 33, 55);
  return (strcmp (buf, "55 33") != 0);
}], gt_cv_func_printf_posix=yes, gt_cv_func_printf_posix=no,
      [
        AC_EGREP_CPP(notposix, [
#if defined __NetBSD__ || defined __BEOS__ || defined _MSC_VER || defined __MINGW32__ || defined __CYGWIN__
  notposix
#endif
        ], gt_cv_func_printf_posix="guessing no",
           gt_cv_func_printf_posix="guessing yes")
      ])
    ])
  case $gt_cv_func_printf_posix in
    *yes)
      AC_DEFINE(HAVE_POSIX_PRINTF, 1,
        [Define if your printf() function supports format strings with positions.])
      ;;
  esac
])
