/* GMP assertion failure handler.

   THE FUNCTIONS IN THIS FILE ARE FOR INTERNAL USE ONLY.  THEY'RE ALMOST
   CERTAIN TO BE SUBJECT TO INCOMPATIBLE CHANGES OR DISAPPEAR COMPLETELY IN
   FUTURE GNU MP RELEASES.

Copyright 2000, 2001 Free Software Foundation, Inc.

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

#include <stdio.h>
#include <stdlib.h>
#include "gmp.h"
#include "gmp-impl.h"


void
__gmp_assert_header (const char *filename, int linenum)
{
  if (filename != NULL && filename[0] != '\0')
    {
      fprintf (stderr, "%s:", filename);
      if (linenum != -1)
        fprintf (stderr, "%d: ", linenum);
    }
}

void
__gmp_assert_fail (const char *filename, int linenum,
                   const char *expr)
{
  __gmp_assert_header (filename, linenum);
  fprintf (stderr, "GNU MP assertion failed: %s\n", expr);
  abort();
}
