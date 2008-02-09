/* Decomposed printf argument list.
   Copyright (C) 1999, 2002-2003, 2005-2007 Free Software Foundation, Inc.

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

/* This file can be parametrized with the following macros:
     ENABLE_UNISTDIO    Set to 1 to enable the unistdio extensions.
     PRINTF_FETCHARGS   Name of the function to be defined.
     STATIC             Set to 'static' to declare the function static.  */

#ifndef PRINTF_FETCHARGS
# include <config.h>
#endif

/* Specification.  */
#ifndef PRINTF_FETCHARGS
# include "printf-args.h"
#endif

#ifdef STATIC
STATIC
#endif
int
PRINTF_FETCHARGS (va_list args, arguments *a)
{
  size_t i;
  argument *ap;

  for (i = 0, ap = &a->arg[0]; i < a->count; i++, ap++)
    switch (ap->type)
      {
      case TYPE_SCHAR:
	ap->a.a_schar = va_arg (args, /*signed char*/ int);
	break;
      case TYPE_UCHAR:
	ap->a.a_uchar = va_arg (args, /*unsigned char*/ int);
	break;
      case TYPE_SHORT:
	ap->a.a_short = va_arg (args, /*short*/ int);
	break;
      case TYPE_USHORT:
	ap->a.a_ushort = va_arg (args, /*unsigned short*/ int);
	break;
      case TYPE_INT:
	ap->a.a_int = va_arg (args, int);
	break;
      case TYPE_UINT:
	ap->a.a_uint = va_arg (args, unsigned int);
	break;
      case TYPE_LONGINT:
	ap->a.a_longint = va_arg (args, long int);
	break;
      case TYPE_ULONGINT:
	ap->a.a_ulongint = va_arg (args, unsigned long int);
	break;
#if HAVE_LONG_LONG_INT
      case TYPE_LONGLONGINT:
	ap->a.a_longlongint = va_arg (args, long long int);
	break;
      case TYPE_ULONGLONGINT:
	ap->a.a_ulonglongint = va_arg (args, unsigned long long int);
	break;
#endif
      case TYPE_DOUBLE:
	ap->a.a_double = va_arg (args, double);
	break;
      case TYPE_LONGDOUBLE:
	ap->a.a_longdouble = va_arg (args, long double);
	break;
      case TYPE_CHAR:
	ap->a.a_char = va_arg (args, int);
	break;
#if HAVE_WINT_T
      case TYPE_WIDE_CHAR:
	/* Although ISO C 99 7.24.1.(2) says that wint_t is "unchanged by
	   default argument promotions", this is not the case in mingw32,
	   where wint_t is 'unsigned short'.  */
	ap->a.a_wide_char =
	  (sizeof (wint_t) < sizeof (int)
	   ? va_arg (args, int)
	   : va_arg (args, wint_t));
	break;
#endif
      case TYPE_STRING:
	ap->a.a_string = va_arg (args, const char *);
	/* A null pointer is an invalid argument for "%s", but in practice
	   it occurs quite frequently in printf statements that produce
	   debug output.  Use a fallback in this case.  */
	if (ap->a.a_string == NULL)
	  ap->a.a_string = "(NULL)";
	break;
#if HAVE_WCHAR_T
      case TYPE_WIDE_STRING:
	ap->a.a_wide_string = va_arg (args, const wchar_t *);
	/* A null pointer is an invalid argument for "%ls", but in practice
	   it occurs quite frequently in printf statements that produce
	   debug output.  Use a fallback in this case.  */
	if (ap->a.a_wide_string == NULL)
	  {
	    static const wchar_t wide_null_string[] =
	      {
		(wchar_t)'(',
		(wchar_t)'N', (wchar_t)'U', (wchar_t)'L', (wchar_t)'L',
		(wchar_t)')',
		(wchar_t)0
	      };
	    ap->a.a_wide_string = wide_null_string;
	  }
	break;
#endif
      case TYPE_POINTER:
	ap->a.a_pointer = va_arg (args, void *);
	break;
      case TYPE_COUNT_SCHAR_POINTER:
	ap->a.a_count_schar_pointer = va_arg (args, signed char *);
	break;
      case TYPE_COUNT_SHORT_POINTER:
	ap->a.a_count_short_pointer = va_arg (args, short *);
	break;
      case TYPE_COUNT_INT_POINTER:
	ap->a.a_count_int_pointer = va_arg (args, int *);
	break;
      case TYPE_COUNT_LONGINT_POINTER:
	ap->a.a_count_longint_pointer = va_arg (args, long int *);
	break;
#if HAVE_LONG_LONG_INT
      case TYPE_COUNT_LONGLONGINT_POINTER:
	ap->a.a_count_longlongint_pointer = va_arg (args, long long int *);
	break;
#endif
#if ENABLE_UNISTDIO
      /* The unistdio extensions.  */
      case TYPE_U8_STRING:
	ap->a.a_u8_string = va_arg (args, const uint8_t *);
	/* A null pointer is an invalid argument for "%U", but in practice
	   it occurs quite frequently in printf statements that produce
	   debug output.  Use a fallback in this case.  */
	if (ap->a.a_u8_string == NULL)
	  {
	    static const uint8_t u8_null_string[] =
	      { '(', 'N', 'U', 'L', 'L', ')', 0 };
	    ap->a.a_u8_string = u8_null_string;
	  }
	break;
      case TYPE_U16_STRING:
	ap->a.a_u16_string = va_arg (args, const uint16_t *);
	/* A null pointer is an invalid argument for "%lU", but in practice
	   it occurs quite frequently in printf statements that produce
	   debug output.  Use a fallback in this case.  */
	if (ap->a.a_u16_string == NULL)
	  {
	    static const uint16_t u16_null_string[] =
	      { '(', 'N', 'U', 'L', 'L', ')', 0 };
	    ap->a.a_u16_string = u16_null_string;
	  }
	break;
      case TYPE_U32_STRING:
	ap->a.a_u32_string = va_arg (args, const uint32_t *);
	/* A null pointer is an invalid argument for "%llU", but in practice
	   it occurs quite frequently in printf statements that produce
	   debug output.  Use a fallback in this case.  */
	if (ap->a.a_u32_string == NULL)
	  {
	    static const uint32_t u32_null_string[] =
	      { '(', 'N', 'U', 'L', 'L', ')', 0 };
	    ap->a.a_u32_string = u32_null_string;
	  }
	break;
#endif
      default:
	/* Unknown type.  */
	return -1;
      }
  return 0;
}
