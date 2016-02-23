/* __gmp_obstack_printf_funs -- support for gmp_obstack_printf and
   gmp_obstack_vprintf.

   THE FUNCTIONS IN THIS FILE ARE FOR INTERNAL USE ONLY.  THEY'RE ALMOST
   CERTAIN TO BE SUBJECT TO INCOMPATIBLE CHANGES OR DISAPPEAR COMPLETELY IN
   FUTURE GNU MP RELEASES.

Copyright 2001 Free Software Foundation, Inc.

This file is part of the GNU MP Library.

The GNU MP Library is free software; you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation; either version 3 of the License, or (at your
option) any later version.

The GNU MP Library is distributed in the hope that it will be useful, but
WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public
License for more details.

You should have received a copy of the GNU Lesser General Public License
along with the GNU MP Library.  If not, see http://www.gnu.org/licenses/.  */

#include "config.h"

#if HAVE_OBSTACK_VPRINTF

#define _GNU_SOURCE   /* ask glibc <stdio.h> for obstack_vprintf */

#if HAVE_STDARG
#include <stdarg.h>
#else
#include <varargs.h>
#endif

#include <stdio.h>    /* for obstack_vprintf */
#include <string.h>
#include <obstack.h>

#include "gmp.h"
#include "gmp-impl.h"


static int
gmp_obstack_memory (struct obstack *ob, const char *ptr, size_t len)
{
  obstack_grow (ob, ptr, len);
  return len;
}

static int
gmp_obstack_reps (struct obstack *ob, int c, int reps)
{
  obstack_blank (ob, reps);
  memset ((char *) obstack_next_free(ob) - reps, c, reps);
  return reps;
}

const struct doprnt_funs_t  __gmp_obstack_printf_funs = {
  (doprnt_format_t) obstack_vprintf,
  (doprnt_memory_t) gmp_obstack_memory,
  (doprnt_reps_t)   gmp_obstack_reps
};

#endif /* HAVE_OBSTACK_VPRINTF */
