# intdiv0.m4 serial 2 (gettext-0.17)
dnl Copyright (C) 2002, 2007 Free Software Foundation, Inc.
dnl This file is free software; the Free Software Foundation
dnl gives unlimited permission to copy and/or distribute it,
dnl with or without modifications, as long as this notice is preserved.

dnl From Bruno Haible.

AC_DEFUN([gt_INTDIV0],
[
  AC_REQUIRE([AC_PROG_CC])dnl
  AC_REQUIRE([AC_CANONICAL_HOST])dnl

  AC_CACHE_CHECK([whether integer division by zero raises SIGFPE],
    gt_cv_int_divbyzero_sigfpe,
    [
      gt_cv_int_divbyzero_sigfpe=
changequote(,)dnl
      case "$host_os" in
        macos* | darwin[6-9]* | darwin[1-9][0-9]*)
          # On MacOS X 10.2 or newer, just assume the same as when cross-
          # compiling. If we were to perform the real test, 1 Crash Report
          # dialog window would pop up.
          case "$host_cpu" in
            i[34567]86 | x86_64)
              gt_cv_int_divbyzero_sigfpe="guessing yes" ;;
          esac
          ;;
      esac
changequote([,])dnl
      if test -z "$gt_cv_int_divbyzero_sigfpe"; then
        AC_TRY_RUN([
#include <stdlib.h>
#include <signal.h>

static void
sigfpe_handler (int sig)
{
  /* Exit with code 0 if SIGFPE, with code 1 if any other signal.  */
  exit (sig != SIGFPE);
}

int x = 1;
int y = 0;
int z;
int nan;

int main ()
{
  signal (SIGFPE, sigfpe_handler);
/* IRIX and AIX (when "xlc -qcheck" is used) yield signal SIGTRAP.  */
#if (defined (__sgi) || defined (_AIX)) && defined (SIGTRAP)
  signal (SIGTRAP, sigfpe_handler);
#endif
/* Linux/SPARC yields signal SIGILL.  */
#if defined (__sparc__) && defined (__linux__)
  signal (SIGILL, sigfpe_handler);
#endif

  z = x / y;
  nan = y / y;
  exit (1);
}
], gt_cv_int_divbyzero_sigfpe=yes, gt_cv_int_divbyzero_sigfpe=no,
          [
            # Guess based on the CPU.
changequote(,)dnl
            case "$host_cpu" in
              alpha* | i[34567]86 | x86_64 | m68k | s390*)
                gt_cv_int_divbyzero_sigfpe="guessing yes";;
              *)
                gt_cv_int_divbyzero_sigfpe="guessing no";;
            esac
changequote([,])dnl
          ])
      fi
    ])
  case "$gt_cv_int_divbyzero_sigfpe" in
    *yes) value=1;;
    *) value=0;;
  esac
  AC_DEFINE_UNQUOTED(INTDIV0_RAISES_SIGFPE, $value,
    [Define if integer division by zero raises signal SIGFPE.])
])
