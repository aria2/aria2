/* xsize.h -- Checked size_t computations.

   Copyright (C) 2003 Free Software Foundation, Inc.

   This program is free software; you can redistribute it and/or modify it
   under the terms of the GNU Library General Public License as published
   by the Free Software Foundation; either version 2, or (at your option)
   any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public
   License along with this program; if not, write to the Free Software
   Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301,
   USA.  */

#ifndef _XSIZE_H
#define _XSIZE_H

/* Get size_t.  */
#include <stddef.h>

/* Get SIZE_MAX.  */
#include <limits.h>
#if HAVE_STDINT_H
# include <stdint.h>
#endif

/* The size of memory objects is often computed through expressions of
   type size_t. Example:
      void* p = malloc (header_size + n * element_size).
   These computations can lead to overflow.  When this happens, malloc()
   returns a piece of memory that is way too small, and the program then
   crashes while attempting to fill the memory.
   To avoid this, the functions and macros in this file check for overflow.
   The convention is that SIZE_MAX represents overflow.
   malloc (SIZE_MAX) is not guaranteed to fail -- think of a malloc
   implementation that uses mmap --, it's recommended to use size_overflow_p()
   or size_in_bounds_p() before invoking malloc().
   The example thus becomes:
      size_t size = xsum (header_size, xtimes (n, element_size));
      void *p = (size_in_bounds_p (size) ? malloc (size) : NULL);
*/

/* Convert an arbitrary value >= 0 to type size_t.  */
#define xcast_size_t(N) \
  ((N) <= SIZE_MAX ? (size_t) (N) : SIZE_MAX)

/* Sum of two sizes, with overflow check.  */
static inline size_t
#if __GNUC__ >= 3
__attribute__ ((__pure__))
#endif
xsum (size_t size1, size_t size2)
{
  size_t sum = size1 + size2;
  return (sum >= size1 ? sum : SIZE_MAX);
}

/* Sum of three sizes, with overflow check.  */
static inline size_t
#if __GNUC__ >= 3
__attribute__ ((__pure__))
#endif
xsum3 (size_t size1, size_t size2, size_t size3)
{
  return xsum (xsum (size1, size2), size3);
}

/* Sum of four sizes, with overflow check.  */
static inline size_t
#if __GNUC__ >= 3
__attribute__ ((__pure__))
#endif
xsum4 (size_t size1, size_t size2, size_t size3, size_t size4)
{
  return xsum (xsum (xsum (size1, size2), size3), size4);
}

/* Maximum of two sizes, with overflow check.  */
static inline size_t
#if __GNUC__ >= 3
__attribute__ ((__pure__))
#endif
xmax (size_t size1, size_t size2)
{
  /* No explicit check is needed here, because for any n:
     max (SIZE_MAX, n) == SIZE_MAX and max (n, SIZE_MAX) == SIZE_MAX.  */
  return (size1 >= size2 ? size1 : size2);
}

/* Multiplication of a count with an element size, with overflow check.
   The count must be >= 0 and the element size must be > 0.
   This is a macro, not an inline function, so that it works correctly even
   when N is of a wider tupe and N > SIZE_MAX.  */
#define xtimes(N, ELSIZE) \
  ((N) <= SIZE_MAX / (ELSIZE) ? (size_t) (N) * (ELSIZE) : SIZE_MAX)

/* Check for overflow.  */
#define size_overflow_p(SIZE) \
  ((SIZE) == SIZE_MAX)
/* Check against overflow.  */
#define size_in_bounds_p(SIZE) \
  ((SIZE) != SIZE_MAX)

#endif /* _XSIZE_H */
