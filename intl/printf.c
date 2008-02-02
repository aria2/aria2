/* Formatted output to strings, using POSIX/XSI format strings with positions.
   Copyright (C) 2003, 2006-2007 Free Software Foundation, Inc.
   Written by Bruno Haible <bruno@clisp.org>, 2003.

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

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#ifdef __GNUC__
# define alloca __builtin_alloca
# define HAVE_ALLOCA 1
#else
# ifdef _MSC_VER
#  include <malloc.h>
#  define alloca _alloca
# else
#  if defined HAVE_ALLOCA_H || defined _LIBC
#   include <alloca.h>
#  else
#   ifdef _AIX
 #pragma alloca
#   else
#    ifndef alloca
char *alloca ();
#    endif
#   endif
#  endif
# endif
#endif

#include <stdio.h>

#if !HAVE_POSIX_PRINTF

#include <errno.h>
#include <limits.h>
#include <stdlib.h>
#include <string.h>

/* Some systems, like OSF/1 4.0 and Woe32, don't have EOVERFLOW.  */
#ifndef EOVERFLOW
# define EOVERFLOW E2BIG
#endif

/* When building a DLL, we must export some functions.  Note that because
   the functions are only defined for binary backward compatibility, we
   don't need to use __declspec(dllimport) in any case.  */
#if defined _MSC_VER && BUILDING_DLL
# define DLL_EXPORTED __declspec(dllexport)
#else
# define DLL_EXPORTED
#endif

#define STATIC static

/* This needs to be consistent with libgnuintl.h.in.  */
#if defined __NetBSD__ || defined __BEOS__ || defined __CYGWIN__ || defined __MINGW32__
/* Don't break __attribute__((format(printf,M,N))).
   This redefinition is only possible because the libc in NetBSD, Cygwin,
   mingw does not have a function __printf__.  */
# define libintl_printf __printf__
#endif

/* Define auxiliary functions declared in "printf-args.h".  */
#include "printf-args.c"

/* Define auxiliary functions declared in "printf-parse.h".  */
#include "printf-parse.c"

/* Define functions declared in "vasnprintf.h".  */
#define vasnprintf libintl_vasnprintf
#include "vasnprintf.c"
#if 0 /* not needed */
#define asnprintf libintl_asnprintf
#include "asnprintf.c"
#endif

DLL_EXPORTED
int
libintl_vfprintf (FILE *stream, const char *format, va_list args)
{
  if (strchr (format, '$') == NULL)
    return vfprintf (stream, format, args);
  else
    {
      size_t length;
      char *result = libintl_vasnprintf (NULL, &length, format, args);
      int retval = -1;
      if (result != NULL)
	{
	  size_t written = fwrite (result, 1, length, stream);
	  free (result);
	  if (written == length)
	    {
	      if (length > INT_MAX)
		errno = EOVERFLOW;
	      else
		retval = length;
	    }
	}
      return retval;
    }
}

DLL_EXPORTED
int
libintl_fprintf (FILE *stream, const char *format, ...)
{
  va_list args;
  int retval;

  va_start (args, format);
  retval = libintl_vfprintf (stream, format, args);
  va_end (args);
  return retval;
}

DLL_EXPORTED
int
libintl_vprintf (const char *format, va_list args)
{
  return libintl_vfprintf (stdout, format, args);
}

DLL_EXPORTED
int
libintl_printf (const char *format, ...)
{
  va_list args;
  int retval;

  va_start (args, format);
  retval = libintl_vprintf (format, args);
  va_end (args);
  return retval;
}

DLL_EXPORTED
int
libintl_vsprintf (char *resultbuf, const char *format, va_list args)
{
  if (strchr (format, '$') == NULL)
    return vsprintf (resultbuf, format, args);
  else
    {
      size_t length = (size_t) ~0 / (4 * sizeof (char));
      char *result = libintl_vasnprintf (resultbuf, &length, format, args);
      if (result != resultbuf)
	{
	  free (result);
	  return -1;
	}
      if (length > INT_MAX)
	{
	  errno = EOVERFLOW;
	  return -1;
	}
      else
	return length;
    }
}

DLL_EXPORTED
int
libintl_sprintf (char *resultbuf, const char *format, ...)
{
  va_list args;
  int retval;

  va_start (args, format);
  retval = libintl_vsprintf (resultbuf, format, args);
  va_end (args);
  return retval;
}

#if HAVE_SNPRINTF

# if HAVE_DECL__SNPRINTF
   /* Windows.  */
#  define system_vsnprintf _vsnprintf
# else
   /* Unix.  */
#  define system_vsnprintf vsnprintf
# endif

DLL_EXPORTED
int
libintl_vsnprintf (char *resultbuf, size_t length, const char *format, va_list args)
{
  if (strchr (format, '$') == NULL)
    return system_vsnprintf (resultbuf, length, format, args);
  else
    {
      size_t maxlength = length;
      char *result = libintl_vasnprintf (resultbuf, &length, format, args);
      if (result != resultbuf)
	{
	  if (maxlength > 0)
	    {
	      size_t pruned_length =
		(length < maxlength ? length : maxlength - 1);
	      memcpy (resultbuf, result, pruned_length);
	      resultbuf[pruned_length] = '\0';
	    }
	  free (result);
	}
      if (length > INT_MAX)
	{
	  errno = EOVERFLOW;
	  return -1;
	}
      else
	return length;
    }
}

DLL_EXPORTED
int
libintl_snprintf (char *resultbuf, size_t length, const char *format, ...)
{
  va_list args;
  int retval;

  va_start (args, format);
  retval = libintl_vsnprintf (resultbuf, length, format, args);
  va_end (args);
  return retval;
}

#endif

#if HAVE_ASPRINTF

DLL_EXPORTED
int
libintl_vasprintf (char **resultp, const char *format, va_list args)
{
  size_t length;
  char *result = libintl_vasnprintf (NULL, &length, format, args);
  if (result == NULL)
    return -1;
  if (length > INT_MAX)
    {
      free (result);
      errno = EOVERFLOW;
      return -1;
    }
  *resultp = result;
  return length;
}

DLL_EXPORTED
int
libintl_asprintf (char **resultp, const char *format, ...)
{
  va_list args;
  int retval;

  va_start (args, format);
  retval = libintl_vasprintf (resultp, format, args);
  va_end (args);
  return retval;
}

#endif

#if HAVE_FWPRINTF

#include <wchar.h>

#define WIDE_CHAR_VERSION 1

#include "wprintf-parse.h"
/* Define auxiliary functions declared in "wprintf-parse.h".  */
#define CHAR_T wchar_t
#define DIRECTIVE wchar_t_directive
#define DIRECTIVES wchar_t_directives
#define PRINTF_PARSE wprintf_parse
#include "printf-parse.c"

/* Define functions declared in "vasnprintf.h".  */
#define vasnwprintf libintl_vasnwprintf
#include "vasnprintf.c"
#if 0 /* not needed */
#define asnwprintf libintl_asnwprintf
#include "asnprintf.c"
#endif

# if HAVE_DECL__SNWPRINTF
   /* Windows.  */
#  define system_vswprintf _vsnwprintf
# else
   /* Unix.  */
#  define system_vswprintf vswprintf
# endif

DLL_EXPORTED
int
libintl_vfwprintf (FILE *stream, const wchar_t *format, va_list args)
{
  if (wcschr (format, '$') == NULL)
    return vfwprintf (stream, format, args);
  else
    {
      size_t length;
      wchar_t *result = libintl_vasnwprintf (NULL, &length, format, args);
      int retval = -1;
      if (result != NULL)
	{
	  size_t i;
	  for (i = 0; i < length; i++)
	    if (fputwc (result[i], stream) == WEOF)
	      break;
	  free (result);
	  if (i == length)
	    {
	      if (length > INT_MAX)
		errno = EOVERFLOW;
	      else
		retval = length;
	    }
	}
      return retval;
    }
}

DLL_EXPORTED
int
libintl_fwprintf (FILE *stream, const wchar_t *format, ...)
{
  va_list args;
  int retval;

  va_start (args, format);
  retval = libintl_vfwprintf (stream, format, args);
  va_end (args);
  return retval;
}

DLL_EXPORTED
int
libintl_vwprintf (const wchar_t *format, va_list args)
{
  return libintl_vfwprintf (stdout, format, args);
}

DLL_EXPORTED
int
libintl_wprintf (const wchar_t *format, ...)
{
  va_list args;
  int retval;

  va_start (args, format);
  retval = libintl_vwprintf (format, args);
  va_end (args);
  return retval;
}

DLL_EXPORTED
int
libintl_vswprintf (wchar_t *resultbuf, size_t length, const wchar_t *format, va_list args)
{
  if (wcschr (format, '$') == NULL)
    return system_vswprintf (resultbuf, length, format, args);
  else
    {
      size_t maxlength = length;
      wchar_t *result = libintl_vasnwprintf (resultbuf, &length, format, args);
      if (result != resultbuf)
	{
	  if (maxlength > 0)
	    {
	      size_t pruned_length =
		(length < maxlength ? length : maxlength - 1);
	      memcpy (resultbuf, result, pruned_length * sizeof (wchar_t));
	      resultbuf[pruned_length] = 0;
	    }
	  free (result);
	  /* Unlike vsnprintf, which has to return the number of character that
	     would have been produced if the resultbuf had been sufficiently
	     large, the vswprintf function has to return a negative value if
	     the resultbuf was not sufficiently large.  */
	  if (length >= maxlength)
	    return -1;
	}
      if (length > INT_MAX)
	{
	  errno = EOVERFLOW;
	  return -1;
	}
      else
	return length;
    }
}

DLL_EXPORTED
int
libintl_swprintf (wchar_t *resultbuf, size_t length, const wchar_t *format, ...)
{
  va_list args;
  int retval;

  va_start (args, format);
  retval = libintl_vswprintf (resultbuf, length, format, args);
  va_end (args);
  return retval;
}

#endif

#endif
