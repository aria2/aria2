AC_DEFUN([ARIA2_CHECK_FALLOCATE],
[
AC_MSG_CHECKING([for fallocate])
AC_COMPILE_IFELSE([AC_LANG_PROGRAM([[
#include <fcntl.h>
]],
[[
fallocate(2, 0, 0, 4096);
]])],
[have_fallocate=yes],
[have_fallocate=no])
AC_MSG_RESULT([$have_fallocate])
if test "x$have_fallocate" = "xyes"; then
  AC_DEFINE([HAVE_FALLOCATE], [1],
            [Define to 1 if you have the `fallocate' function.])
fi
])
