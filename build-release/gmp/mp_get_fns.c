/* mp_get_memory_functions -- Get the allocate, reallocate, and free functions.

Copyright 2002 Free Software Foundation, Inc.

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

#include <stdio.h>  /* for NULL */
#include "gmp.h"
#include "gmp-impl.h"

void
mp_get_memory_functions (void *(**alloc_func) (size_t),
			 void *(**realloc_func) (void *, size_t, size_t),
			 void (**free_func) (void *, size_t)) __GMP_NOTHROW
{
  if (alloc_func != NULL)
    *alloc_func = __gmp_allocate_func;

  if (realloc_func != NULL)
    *realloc_func = __gmp_reallocate_func;

  if (free_func != NULL)
    *free_func = __gmp_free_func;
}
