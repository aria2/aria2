
#-----------------------------------------------------------------------
# Supports the following non-standard switches.
#
#   --enable-threadsafe
#   --enable-readline
#   --enable-dynamic-extensions
#

AC_PREREQ(2.61)
AC_INIT(sqlite, 3.8.10.2, http://www.sqlite.org)
AC_CONFIG_SRCDIR([sqlite3.c])

# Use automake.
AM_INIT_AUTOMAKE([foreign])

AC_SYS_LARGEFILE

# Check for required programs.
AC_PROG_CC
AC_PROG_RANLIB
AC_PROG_LIBTOOL
AC_PROG_MKDIR_P

# Check for library functions that SQLite can optionally use.
AC_CHECK_FUNCS([fdatasync usleep fullfsync localtime_r gmtime_r])
AC_FUNC_STRERROR_R

AC_CONFIG_FILES([Makefile sqlite3.pc])
AC_SUBST(BUILD_CFLAGS)

#-----------------------------------------------------------------------
#   --enable-readline
#
AC_ARG_ENABLE(readline, [AS_HELP_STRING(
  [--enable-readline], 
  [use readline in shell tool (yes, no) [default=yes]])], 
  [], [enable_readline=yes])
if test x"$enable_readline" != xno ; then
  sLIBS=$LIBS
  LIBS=""
  AC_SEARCH_LIBS(tgetent, curses ncurses ncursesw, [], [])
  AC_SEARCH_LIBS(readline, readline, [], [enable_readline=no])
  AC_CHECK_FUNCS(readline, [], [])
  READLINE_LIBS=$LIBS
  LIBS=$sLIBS
fi
AC_SUBST(READLINE_LIBS)
#-----------------------------------------------------------------------

#-----------------------------------------------------------------------
#   --enable-threadsafe
#
AC_ARG_ENABLE(threadsafe, [AS_HELP_STRING(
  [--enable-threadsafe], [build a thread-safe library [default=yes]])], 
  [], [enable_threadsafe=yes])
THREADSAFE_FLAGS=-DSQLITE_THREADSAFE=0
if test x"$enable_threadsafe" != "xno"; then
  THREADSAFE_FLAGS="-D_REENTRANT=1 -DSQLITE_THREADSAFE=1"
  AC_SEARCH_LIBS(pthread_create, pthread)
fi
AC_SUBST(THREADSAFE_FLAGS)
#-----------------------------------------------------------------------

#-----------------------------------------------------------------------
#   --enable-dynamic-extensions
#
AC_ARG_ENABLE(dynamic-extensions, [AS_HELP_STRING(
  [--enable-dynamic-extensions], [support loadable extensions [default=yes]])], 
  [], [enable_dynamic_extensions=yes])
if test x"$enable_dynamic_extensions" != "xno"; then
  AC_SEARCH_LIBS(dlopen, dl)
else
  DYNAMIC_EXTENSION_FLAGS=-DSQLITE_OMIT_LOAD_EXTENSION=1
fi
AC_MSG_CHECKING([for whether to support dynamic extensions])
AC_MSG_RESULT($enable_dynamic_extensions)
AC_SUBST(DYNAMIC_EXTENSION_FLAGS)
#-----------------------------------------------------------------------

AC_CHECK_FUNCS(posix_fallocate)

#-----------------------------------------------------------------------
# UPDATE: Maybe it's better if users just set CFLAGS before invoking
# configure. This option doesn't really add much...
#
#   --enable-tempstore
#
# AC_ARG_ENABLE(tempstore, [AS_HELP_STRING(
#   [--enable-tempstore], 
#   [in-memory temporary tables (never, no, yes, always) [default=no]])], 
#   [], [enable_tempstore=no])
# AC_MSG_CHECKING([for whether or not to store temp tables in-memory])
# case "$enable_tempstore" in
#   never )  TEMP_STORE=0 ;;
#   no )     TEMP_STORE=1 ;;
#   always ) TEMP_STORE=3 ;;
#   yes )    TEMP_STORE=3 ;;
#   * )
#     TEMP_STORE=1
#     enable_tempstore=yes
#   ;;
# esac
# AC_MSG_RESULT($enable_tempstore)
# AC_SUBST(TEMP_STORE)
#-----------------------------------------------------------------------

AC_OUTPUT
