/* vswprintf with automatic memory allocation.
   Copyright (C) 2002-2003 Free Software Foundation, Inc.

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

#ifndef _VASNWPRINTF_H
#define _VASNWPRINTF_H

/* Get va_list.  */
#include <stdarg.h>

/* Get wchar_t, size_t.  */
#include <stddef.h>

#ifdef	__cplusplus
extern "C" {
#endif

/* Write formatted output to a string dynamically allocated with malloc().
   You can pass a preallocated buffer for the result in RESULTBUF and its
   size in *LENGTHP; otherwise you pass RESULTBUF = NULL.
   If successful, return the address of the string (this may be = RESULTBUF
   if no dynamic memory allocation was necessary) and set *LENGTHP to the
   number of resulting bytes, excluding the trailing NUL.  Upon error, set
   errno and return NULL.  */
extern wchar_t * asnwprintf (wchar_t *resultbuf, size_t *lengthp, const wchar_t *format, ...);
extern wchar_t * vasnwprintf (wchar_t *resultbuf, size_t *lengthp, const wchar_t *format, va_list args);

#ifdef	__cplusplus
}
#endif

#endif /* _VASNWPRINTF_H */
