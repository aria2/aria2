# lock.m4 serial 7 (gettext-0.17)
dnl Copyright (C) 2005-2007 Free Software Foundation, Inc.
dnl This file is free software; the Free Software Foundation
dnl gives unlimited permission to copy and/or distribute it,
dnl with or without modifications, as long as this notice is preserved.

dnl From Bruno Haible.

dnl Tests for a multithreading library to be used.
dnl Defines at most one of the macros USE_POSIX_THREADS, USE_SOLARIS_THREADS,
dnl USE_PTH_THREADS, USE_WIN32_THREADS
dnl Sets the variables LIBTHREAD and LTLIBTHREAD to the linker options for use
dnl in a Makefile (LIBTHREAD for use without libtool, LTLIBTHREAD for use with
dnl libtool).
dnl Sets the variables LIBMULTITHREAD and LTLIBMULTITHREAD similarly, for
dnl programs that really need multithread functionality. The difference
dnl between LIBTHREAD and LIBMULTITHREAD is that on platforms supporting weak
dnl symbols, typically LIBTHREAD="" whereas LIBMULTITHREAD="-lpthread".
dnl Adds to CPPFLAGS the flag -D_REENTRANT or -D_THREAD_SAFE if needed for
dnl multithread-safe programs.

AC_DEFUN([gl_LOCK_EARLY],
[
  AC_REQUIRE([gl_LOCK_EARLY_BODY])
])

dnl The guts of gl_LOCK_EARLY. Needs to be expanded only once.

AC_DEFUN([gl_LOCK_EARLY_BODY],
[
  dnl Ordering constraints: This macro modifies CPPFLAGS in a way that
  dnl influences the result of the autoconf tests that test for *_unlocked
  dnl declarations, on AIX 5 at least. Therefore it must come early.
  AC_BEFORE([$0], [gl_FUNC_GLIBC_UNLOCKED_IO])dnl
  AC_BEFORE([$0], [gl_ARGP])dnl

  AC_REQUIRE([AC_CANONICAL_HOST])
  dnl _GNU_SOURCE is needed for pthread_rwlock_t on glibc systems.
  dnl AC_USE_SYSTEM_EXTENSIONS was introduced in autoconf 2.60 and obsoletes
  dnl AC_GNU_SOURCE.
  m4_ifdef([AC_USE_SYSTEM_EXTENSIONS],
    [AC_REQUIRE([AC_USE_SYSTEM_EXTENSIONS])],
    [AC_REQUIRE([AC_GNU_SOURCE])])
  dnl Check for multithreading.
  AC_ARG_ENABLE(threads,
AC_HELP_STRING([--enable-threads={posix|solaris|pth|win32}], [specify multithreading API])
AC_HELP_STRING([--disable-threads], [build without multithread safety]),
    [gl_use_threads=$enableval],
    [case "$host_os" in
       dnl Disable multithreading by default on OSF/1, because it interferes
       dnl with fork()/exec(): When msgexec is linked with -lpthread, its child
       dnl process gets an endless segmentation fault inside execvp().
       osf*) gl_use_threads=no ;;
       *)    gl_use_threads=yes ;;
     esac
    ])
  if test "$gl_use_threads" = yes || test "$gl_use_threads" = posix; then
    # For using <pthread.h>:
    case "$host_os" in
      osf*)
        # On OSF/1, the compiler needs the flag -D_REENTRANT so that it
        # groks <pthread.h>. cc also understands the flag -pthread, but
        # we don't use it because 1. gcc-2.95 doesn't understand -pthread,
        # 2. putting a flag into CPPFLAGS that has an effect on the linker
        # causes the AC_TRY_LINK test below to succeed unexpectedly,
        # leading to wrong values of LIBTHREAD and LTLIBTHREAD.
        CPPFLAGS="$CPPFLAGS -D_REENTRANT"
        ;;
    esac
    # Some systems optimize for single-threaded programs by default, and
    # need special flags to disable these optimizations. For example, the
    # definition of 'errno' in <errno.h>.
    case "$host_os" in
      aix* | freebsd*) CPPFLAGS="$CPPFLAGS -D_THREAD_SAFE" ;;
      solaris*) CPPFLAGS="$CPPFLAGS -D_REENTRANT" ;;
    esac
  fi
])

dnl The guts of gl_LOCK. Needs to be expanded only once.

AC_DEFUN([gl_LOCK_BODY],
[
  AC_REQUIRE([gl_LOCK_EARLY_BODY])
  gl_threads_api=none
  LIBTHREAD=
  LTLIBTHREAD=
  LIBMULTITHREAD=
  LTLIBMULTITHREAD=
  if test "$gl_use_threads" != no; then
    dnl Check whether the compiler and linker support weak declarations.
    AC_MSG_CHECKING([whether imported symbols can be declared weak])
    gl_have_weak=no
    AC_TRY_LINK([extern void xyzzy ();
#pragma weak xyzzy], [xyzzy();], [gl_have_weak=yes])
    AC_MSG_RESULT([$gl_have_weak])
    if test "$gl_use_threads" = yes || test "$gl_use_threads" = posix; then
      # On OSF/1, the compiler needs the flag -pthread or -D_REENTRANT so that
      # it groks <pthread.h>. It's added above, in gl_LOCK_EARLY_BODY.
      AC_CHECK_HEADER(pthread.h, gl_have_pthread_h=yes, gl_have_pthread_h=no)
      if test "$gl_have_pthread_h" = yes; then
        # Other possible tests:
        #   -lpthreads (FSU threads, PCthreads)
        #   -lgthreads
        gl_have_pthread=
        # Test whether both pthread_mutex_lock and pthread_mutexattr_init exist
        # in libc. IRIX 6.5 has the first one in both libc and libpthread, but
        # the second one only in libpthread, and lock.c needs it.
        AC_TRY_LINK([#include <pthread.h>],
          [pthread_mutex_lock((pthread_mutex_t*)0);
           pthread_mutexattr_init((pthread_mutexattr_t*)0);],
          [gl_have_pthread=yes])
        # Test for libpthread by looking for pthread_kill. (Not pthread_self,
        # since it is defined as a macro on OSF/1.)
        if test -n "$gl_have_pthread"; then
          # The program links fine without libpthread. But it may actually
          # need to link with libpthread in order to create multiple threads.
          AC_CHECK_LIB(pthread, pthread_kill,
            [LIBMULTITHREAD=-lpthread LTLIBMULTITHREAD=-lpthread
             # On Solaris and HP-UX, most pthread functions exist also in libc.
             # Therefore pthread_in_use() needs to actually try to create a
             # thread: pthread_create from libc will fail, whereas
             # pthread_create will actually create a thread.
             case "$host_os" in
               solaris* | hpux*)
                 AC_DEFINE([PTHREAD_IN_USE_DETECTION_HARD], 1,
                   [Define if the pthread_in_use() detection is hard.])
             esac
            ])
        else
          # Some library is needed. Try libpthread and libc_r.
          AC_CHECK_LIB(pthread, pthread_kill,
            [gl_have_pthread=yes
             LIBTHREAD=-lpthread LTLIBTHREAD=-lpthread
             LIBMULTITHREAD=-lpthread LTLIBMULTITHREAD=-lpthread])
          if test -z "$gl_have_pthread"; then
            # For FreeBSD 4.
            AC_CHECK_LIB(c_r, pthread_kill,
              [gl_have_pthread=yes
               LIBTHREAD=-lc_r LTLIBTHREAD=-lc_r
               LIBMULTITHREAD=-lc_r LTLIBMULTITHREAD=-lc_r])
          fi
        fi
        if test -n "$gl_have_pthread"; then
          gl_threads_api=posix
          AC_DEFINE([USE_POSIX_THREADS], 1,
            [Define if the POSIX multithreading library can be used.])
          if test -n "$LIBMULTITHREAD" || test -n "$LTLIBMULTITHREAD"; then
            if test $gl_have_weak = yes; then
              AC_DEFINE([USE_POSIX_THREADS_WEAK], 1,
                [Define if references to the POSIX multithreading library should be made weak.])
              LIBTHREAD=
              LTLIBTHREAD=
            fi
          fi
          # OSF/1 4.0 and MacOS X 10.1 lack the pthread_rwlock_t type and the
          # pthread_rwlock_* functions.
          AC_CHECK_TYPE([pthread_rwlock_t],
            [AC_DEFINE([HAVE_PTHREAD_RWLOCK], 1,
               [Define if the POSIX multithreading library has read/write locks.])],
            [],
            [#include <pthread.h>])
          # glibc defines PTHREAD_MUTEX_RECURSIVE as enum, not as a macro.
          AC_TRY_COMPILE([#include <pthread.h>],
            [#if __FreeBSD__ == 4
error "No, in FreeBSD 4.0 recursive mutexes actually don't work."
#else
int x = (int)PTHREAD_MUTEX_RECURSIVE;
return !x;
#endif],
            [AC_DEFINE([HAVE_PTHREAD_MUTEX_RECURSIVE], 1,
               [Define if the <pthread.h> defines PTHREAD_MUTEX_RECURSIVE.])])
        fi
      fi
    fi
    if test -z "$gl_have_pthread"; then
      if test "$gl_use_threads" = yes || test "$gl_use_threads" = solaris; then
        gl_have_solaristhread=
        gl_save_LIBS="$LIBS"
        LIBS="$LIBS -lthread"
        AC_TRY_LINK([#include <thread.h>
#include <synch.h>],
          [thr_self();],
          [gl_have_solaristhread=yes])
        LIBS="$gl_save_LIBS"
        if test -n "$gl_have_solaristhread"; then
          gl_threads_api=solaris
          LIBTHREAD=-lthread
          LTLIBTHREAD=-lthread
          LIBMULTITHREAD="$LIBTHREAD"
          LTLIBMULTITHREAD="$LTLIBTHREAD"
          AC_DEFINE([USE_SOLARIS_THREADS], 1,
            [Define if the old Solaris multithreading library can be used.])
          if test $gl_have_weak = yes; then
            AC_DEFINE([USE_SOLARIS_THREADS_WEAK], 1,
              [Define if references to the old Solaris multithreading library should be made weak.])
            LIBTHREAD=
            LTLIBTHREAD=
          fi
        fi
      fi
    fi
    if test "$gl_use_threads" = pth; then
      gl_save_CPPFLAGS="$CPPFLAGS"
      AC_LIB_LINKFLAGS(pth)
      gl_have_pth=
      gl_save_LIBS="$LIBS"
      LIBS="$LIBS -lpth"
      AC_TRY_LINK([#include <pth.h>], [pth_self();], gl_have_pth=yes)
      LIBS="$gl_save_LIBS"
      if test -n "$gl_have_pth"; then
        gl_threads_api=pth
        LIBTHREAD="$LIBPTH"
        LTLIBTHREAD="$LTLIBPTH"
        LIBMULTITHREAD="$LIBTHREAD"
        LTLIBMULTITHREAD="$LTLIBTHREAD"
        AC_DEFINE([USE_PTH_THREADS], 1,
          [Define if the GNU Pth multithreading library can be used.])
        if test -n "$LIBMULTITHREAD" || test -n "$LTLIBMULTITHREAD"; then
          if test $gl_have_weak = yes; then
            AC_DEFINE([USE_PTH_THREADS_WEAK], 1,
              [Define if references to the GNU Pth multithreading library should be made weak.])
            LIBTHREAD=
            LTLIBTHREAD=
          fi
        fi
      else
        CPPFLAGS="$gl_save_CPPFLAGS"
      fi
    fi
    if test -z "$gl_have_pthread"; then
      if test "$gl_use_threads" = yes || test "$gl_use_threads" = win32; then
        if { case "$host_os" in
               mingw*) true;;
               *) false;;
             esac
           }; then
          gl_threads_api=win32
          AC_DEFINE([USE_WIN32_THREADS], 1,
            [Define if the Win32 multithreading API can be used.])
        fi
      fi
    fi
  fi
  AC_MSG_CHECKING([for multithread API to use])
  AC_MSG_RESULT([$gl_threads_api])
  AC_SUBST(LIBTHREAD)
  AC_SUBST(LTLIBTHREAD)
  AC_SUBST(LIBMULTITHREAD)
  AC_SUBST(LTLIBMULTITHREAD)
])

AC_DEFUN([gl_LOCK],
[
  AC_REQUIRE([gl_LOCK_EARLY])
  AC_REQUIRE([gl_LOCK_BODY])
  gl_PREREQ_LOCK
])

# Prerequisites of lib/lock.c.
AC_DEFUN([gl_PREREQ_LOCK], [
  AC_REQUIRE([AC_C_INLINE])
])

dnl Survey of platforms:
dnl
dnl Platform          Available   Compiler    Supports   test-lock
dnl                   flavours    option      weak       result
dnl ---------------   ---------   ---------   --------   ---------
dnl Linux 2.4/glibc   posix       -lpthread       Y      OK
dnl
dnl GNU Hurd/glibc    posix
dnl
dnl FreeBSD 5.3       posix       -lc_r           Y
dnl                   posix       -lkse ?         Y
dnl                   posix       -lpthread ?     Y
dnl                   posix       -lthr           Y
dnl
dnl FreeBSD 5.2       posix       -lc_r           Y
dnl                   posix       -lkse           Y
dnl                   posix       -lthr           Y
dnl
dnl FreeBSD 4.0,4.10  posix       -lc_r           Y      OK
dnl
dnl NetBSD 1.6        --
dnl
dnl OpenBSD 3.4       posix       -lpthread       Y      OK
dnl
dnl MacOS X 10.[123]  posix       -lpthread       Y      OK
dnl
dnl Solaris 7,8,9     posix       -lpthread       Y      Sol 7,8: 0.0; Sol 9: OK
dnl                   solaris     -lthread        Y      Sol 7,8: 0.0; Sol 9: OK
dnl
dnl HP-UX 11          posix       -lpthread       N (cc) OK
dnl                                               Y (gcc)
dnl
dnl IRIX 6.5          posix       -lpthread       Y      0.5
dnl
dnl AIX 4.3,5.1       posix       -lpthread       N      AIX 4: 0.5; AIX 5: OK
dnl
dnl OSF/1 4.0,5.1     posix       -pthread (cc)   N      OK
dnl                               -lpthread (gcc) Y
dnl
dnl Cygwin            posix       -lpthread       Y      OK
dnl
dnl Any of the above  pth         -lpth                  0.0
dnl
dnl Mingw             win32                       N      OK
dnl
dnl BeOS 5            --
dnl
dnl The test-lock result shows what happens if in test-lock.c EXPLICIT_YIELD is
dnl turned off:
dnl   OK if all three tests terminate OK,
dnl   0.5 if the first test terminates OK but the second one loops endlessly,
dnl   0.0 if the first test already loops endlessly.
