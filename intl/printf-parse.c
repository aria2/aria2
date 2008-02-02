/* Formatted output to strings.
   Copyright (C) 1999-2000, 2002-2003, 2006-2007 Free Software Foundation, Inc.

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
     CHAR_T             The element type of the format string.
     CHAR_T_ONLY_ASCII  Set to 1 to enable verification that all characters
                        in the format string are ASCII.
     DIRECTIVE          Structure denoting a format directive.
                        Depends on CHAR_T.
     DIRECTIVES         Structure denoting the set of format directives of a
                        format string.  Depends on CHAR_T.
     PRINTF_PARSE       Function that parses a format string.
                        Depends on CHAR_T.
     STATIC             Set to 'static' to declare the function static.
     ENABLE_UNISTDIO    Set to 1 to enable the unistdio extensions.  */

#ifndef PRINTF_PARSE
# include <config.h>
#endif

/* Specification.  */
#ifndef PRINTF_PARSE
# include "printf-parse.h"
#endif

/* Default parameters.  */
#ifndef PRINTF_PARSE
# define PRINTF_PARSE printf_parse
# define CHAR_T char
# define DIRECTIVE char_directive
# define DIRECTIVES char_directives
#endif

/* Get size_t, NULL.  */
#include <stddef.h>

/* Get intmax_t.  */
#if defined IN_LIBINTL || defined IN_LIBASPRINTF
# if HAVE_STDINT_H_WITH_UINTMAX
#  include <stdint.h>
# endif
# if HAVE_INTTYPES_H_WITH_UINTMAX
#  include <inttypes.h>
# endif
#else
# include <stdint.h>
#endif

/* malloc(), realloc(), free().  */
#include <stdlib.h>

/* errno.  */
#include <errno.h>

/* Checked size_t computations.  */
#include "xsize.h"

#if CHAR_T_ONLY_ASCII
/* c_isascii().  */
# include "c-ctype.h"
#endif

#ifdef STATIC
STATIC
#endif
int
PRINTF_PARSE (const CHAR_T *format, DIRECTIVES *d, arguments *a)
{
  const CHAR_T *cp = format;		/* pointer into format */
  size_t arg_posn = 0;		/* number of regular arguments consumed */
  size_t d_allocated;			/* allocated elements of d->dir */
  size_t a_allocated;			/* allocated elements of a->arg */
  size_t max_width_length = 0;
  size_t max_precision_length = 0;

  d->count = 0;
  d_allocated = 1;
  d->dir = (DIRECTIVE *) malloc (d_allocated * sizeof (DIRECTIVE));
  if (d->dir == NULL)
    /* Out of memory.  */
    goto out_of_memory_1;

  a->count = 0;
  a_allocated = 0;
  a->arg = NULL;

#define REGISTER_ARG(_index_,_type_) \
  {									\
    size_t n = (_index_);						\
    if (n >= a_allocated)						\
      {									\
	size_t memory_size;						\
	argument *memory;						\
									\
	a_allocated = xtimes (a_allocated, 2);				\
	if (a_allocated <= n)						\
	  a_allocated = xsum (n, 1);					\
	memory_size = xtimes (a_allocated, sizeof (argument));		\
	if (size_overflow_p (memory_size))				\
	  /* Overflow, would lead to out of memory.  */			\
	  goto out_of_memory;						\
	memory = (argument *) (a->arg					\
			       ? realloc (a->arg, memory_size)		\
			       : malloc (memory_size));			\
	if (memory == NULL)						\
	  /* Out of memory.  */						\
	  goto out_of_memory;						\
	a->arg = memory;						\
      }									\
    while (a->count <= n)						\
      a->arg[a->count++].type = TYPE_NONE;				\
    if (a->arg[n].type == TYPE_NONE)					\
      a->arg[n].type = (_type_);					\
    else if (a->arg[n].type != (_type_))				\
      /* Ambiguous type for positional argument.  */			\
      goto error;							\
  }

  while (*cp != '\0')
    {
      CHAR_T c = *cp++;
      if (c == '%')
	{
	  size_t arg_index = ARG_NONE;
	  DIRECTIVE *dp = &d->dir[d->count]; /* pointer to next directive */

	  /* Initialize the next directive.  */
	  dp->dir_start = cp - 1;
	  dp->flags = 0;
	  dp->width_start = NULL;
	  dp->width_end = NULL;
	  dp->width_arg_index = ARG_NONE;
	  dp->precision_start = NULL;
	  dp->precision_end = NULL;
	  dp->precision_arg_index = ARG_NONE;
	  dp->arg_index = ARG_NONE;

	  /* Test for positional argument.  */
	  if (*cp >= '0' && *cp <= '9')
	    {
	      const CHAR_T *np;

	      for (np = cp; *np >= '0' && *np <= '9'; np++)
		;
	      if (*np == '$')
		{
		  size_t n = 0;

		  for (np = cp; *np >= '0' && *np <= '9'; np++)
		    n = xsum (xtimes (n, 10), *np - '0');
		  if (n == 0)
		    /* Positional argument 0.  */
		    goto error;
		  if (size_overflow_p (n))
		    /* n too large, would lead to out of memory later.  */
		    goto error;
		  arg_index = n - 1;
		  cp = np + 1;
		}
	    }

	  /* Read the flags.  */
	  for (;;)
	    {
	      if (*cp == '\'')
		{
		  dp->flags |= FLAG_GROUP;
		  cp++;
		}
	      else if (*cp == '-')
		{
		  dp->flags |= FLAG_LEFT;
		  cp++;
		}
	      else if (*cp == '+')
		{
		  dp->flags |= FLAG_SHOWSIGN;
		  cp++;
		}
	      else if (*cp == ' ')
		{
		  dp->flags |= FLAG_SPACE;
		  cp++;
		}
	      else if (*cp == '#')
		{
		  dp->flags |= FLAG_ALT;
		  cp++;
		}
	      else if (*cp == '0')
		{
		  dp->flags |= FLAG_ZERO;
		  cp++;
		}
	      else
		break;
	    }

	  /* Parse the field width.  */
	  if (*cp == '*')
	    {
	      dp->width_start = cp;
	      cp++;
	      dp->width_end = cp;
	      if (max_width_length < 1)
		max_width_length = 1;

	      /* Test for positional argument.  */
	      if (*cp >= '0' && *cp <= '9')
		{
		  const CHAR_T *np;

		  for (np = cp; *np >= '0' && *np <= '9'; np++)
		    ;
		  if (*np == '$')
		    {
		      size_t n = 0;

		      for (np = cp; *np >= '0' && *np <= '9'; np++)
			n = xsum (xtimes (n, 10), *np - '0');
		      if (n == 0)
			/* Positional argument 0.  */
			goto error;
		      if (size_overflow_p (n))
			/* n too large, would lead to out of memory later.  */
			goto error;
		      dp->width_arg_index = n - 1;
		      cp = np + 1;
		    }
		}
	      if (dp->width_arg_index == ARG_NONE)
		{
		  dp->width_arg_index = arg_posn++;
		  if (dp->width_arg_index == ARG_NONE)
		    /* arg_posn wrapped around.  */
		    goto error;
		}
	      REGISTER_ARG (dp->width_arg_index, TYPE_INT);
	    }
	  else if (*cp >= '0' && *cp <= '9')
	    {
	      size_t width_length;

	      dp->width_start = cp;
	      for (; *cp >= '0' && *cp <= '9'; cp++)
		;
	      dp->width_end = cp;
	      width_length = dp->width_end - dp->width_start;
	      if (max_width_length < width_length)
		max_width_length = width_length;
	    }

	  /* Parse the precision.  */
	  if (*cp == '.')
	    {
	      cp++;
	      if (*cp == '*')
		{
		  dp->precision_start = cp - 1;
		  cp++;
		  dp->precision_end = cp;
		  if (max_precision_length < 2)
		    max_precision_length = 2;

		  /* Test for positional argument.  */
		  if (*cp >= '0' && *cp <= '9')
		    {
		      const CHAR_T *np;

		      for (np = cp; *np >= '0' && *np <= '9'; np++)
			;
		      if (*np == '$')
			{
			  size_t n = 0;

			  for (np = cp; *np >= '0' && *np <= '9'; np++)
			    n = xsum (xtimes (n, 10), *np - '0');
			  if (n == 0)
			    /* Positional argument 0.  */
			    goto error;
			  if (size_overflow_p (n))
			    /* n too large, would lead to out of memory
			       later.  */
			    goto error;
			  dp->precision_arg_index = n - 1;
			  cp = np + 1;
			}
		    }
		  if (dp->precision_arg_index == ARG_NONE)
		    {
		      dp->precision_arg_index = arg_posn++;
		      if (dp->precision_arg_index == ARG_NONE)
			/* arg_posn wrapped around.  */
			goto error;
		    }
		  REGISTER_ARG (dp->precision_arg_index, TYPE_INT);
		}
	      else
		{
		  size_t precision_length;

		  dp->precision_start = cp - 1;
		  for (; *cp >= '0' && *cp <= '9'; cp++)
		    ;
		  dp->precision_end = cp;
		  precision_length = dp->precision_end - dp->precision_start;
		  if (max_precision_length < precision_length)
		    max_precision_length = precision_length;
		}
	    }

	  {
	    arg_type type;

	    /* Parse argument type/size specifiers.  */
	    {
	      int flags = 0;

	      for (;;)
		{
		  if (*cp == 'h')
		    {
		      flags |= (1 << (flags & 1));
		      cp++;
		    }
		  else if (*cp == 'L')
		    {
		      flags |= 4;
		      cp++;
		    }
		  else if (*cp == 'l')
		    {
		      flags += 8;
		      cp++;
		    }
		  else if (*cp == 'j')
		    {
		      if (sizeof (intmax_t) > sizeof (long))
			{
			  /* intmax_t = long long */
			  flags += 16;
			}
		      else if (sizeof (intmax_t) > sizeof (int))
			{
			  /* intmax_t = long */
			  flags += 8;
			}
		      cp++;
		    }
		  else if (*cp == 'z' || *cp == 'Z')
		    {
		      /* 'z' is standardized in ISO C 99, but glibc uses 'Z'
			 because the warning facility in gcc-2.95.2 understands
			 only 'Z' (see gcc-2.95.2/gcc/c-common.c:1784).  */
		      if (sizeof (size_t) > sizeof (long))
			{
			  /* size_t = long long */
			  flags += 16;
			}
		      else if (sizeof (size_t) > sizeof (int))
			{
			  /* size_t = long */
			  flags += 8;
			}
		      cp++;
		    }
		  else if (*cp == 't')
		    {
		      if (sizeof (ptrdiff_t) > sizeof (long))
			{
			  /* ptrdiff_t = long long */
			  flags += 16;
			}
		      else if (sizeof (ptrdiff_t) > sizeof (int))
			{
			  /* ptrdiff_t = long */
			  flags += 8;
			}
		      cp++;
		    }
		  else
		    break;
		}

	      /* Read the conversion character.  */
	      c = *cp++;
	      switch (c)
		{
		case 'd': case 'i':
#if HAVE_LONG_LONG_INT
		  /* If 'long long' exists and is larger than 'long':  */
		  if (flags >= 16 || (flags & 4))
		    type = TYPE_LONGLONGINT;
		  else
#endif
		  /* If 'long long' exists and is the same as 'long', we parse
		     "lld" into TYPE_LONGINT.  */
		  if (flags >= 8)
		    type = TYPE_LONGINT;
		  else if (flags & 2)
		    type = TYPE_SCHAR;
		  else if (flags & 1)
		    type = TYPE_SHORT;
		  else
		    type = TYPE_INT;
		  break;
		case 'o': case 'u': case 'x': case 'X':
#if HAVE_LONG_LONG_INT
		  /* If 'long long' exists and is larger than 'long':  */
		  if (flags >= 16 || (flags & 4))
		    type = TYPE_ULONGLONGINT;
		  else
#endif
		  /* If 'unsigned long long' exists and is the same as
		     'unsigned long', we parse "llu" into TYPE_ULONGINT.  */
		  if (flags >= 8)
		    type = TYPE_ULONGINT;
		  else if (flags & 2)
		    type = TYPE_UCHAR;
		  else if (flags & 1)
		    type = TYPE_USHORT;
		  else
		    type = TYPE_UINT;
		  break;
		case 'f': case 'F': case 'e': case 'E': case 'g': case 'G':
		case 'a': case 'A':
		  if (flags >= 16 || (flags & 4))
		    type = TYPE_LONGDOUBLE;
		  else
		    type = TYPE_DOUBLE;
		  break;
		case 'c':
		  if (flags >= 8)
#if HAVE_WINT_T
		    type = TYPE_WIDE_CHAR;
#else
		    goto error;
#endif
		  else
		    type = TYPE_CHAR;
		  break;
#if HAVE_WINT_T
		case 'C':
		  type = TYPE_WIDE_CHAR;
		  c = 'c';
		  break;
#endif
		case 's':
		  if (flags >= 8)
#if HAVE_WCHAR_T
		    type = TYPE_WIDE_STRING;
#else
		    goto error;
#endif
		  else
		    type = TYPE_STRING;
		  break;
#if HAVE_WCHAR_T
		case 'S':
		  type = TYPE_WIDE_STRING;
		  c = 's';
		  break;
#endif
		case 'p':
		  type = TYPE_POINTER;
		  break;
		case 'n':
#if HAVE_LONG_LONG_INT
		  /* If 'long long' exists and is larger than 'long':  */
		  if (flags >= 16 || (flags & 4))
		    type = TYPE_COUNT_LONGLONGINT_POINTER;
		  else
#endif
		  /* If 'long long' exists and is the same as 'long', we parse
		     "lln" into TYPE_COUNT_LONGINT_POINTER.  */
		  if (flags >= 8)
		    type = TYPE_COUNT_LONGINT_POINTER;
		  else if (flags & 2)
		    type = TYPE_COUNT_SCHAR_POINTER;
		  else if (flags & 1)
		    type = TYPE_COUNT_SHORT_POINTER;
		  else
		    type = TYPE_COUNT_INT_POINTER;
		  break;
#if ENABLE_UNISTDIO
		/* The unistdio extensions.  */
		case 'U':
		  if (flags >= 16)
		    type = TYPE_U32_STRING;
		  else if (flags >= 8)
		    type = TYPE_U16_STRING;
		  else
		    type = TYPE_U8_STRING;
		  break;
#endif
		case '%':
		  type = TYPE_NONE;
		  break;
		default:
		  /* Unknown conversion character.  */
		  goto error;
		}
	    }

	    if (type != TYPE_NONE)
	      {
		dp->arg_index = arg_index;
		if (dp->arg_index == ARG_NONE)
		  {
		    dp->arg_index = arg_posn++;
		    if (dp->arg_index == ARG_NONE)
		      /* arg_posn wrapped around.  */
		      goto error;
		  }
		REGISTER_ARG (dp->arg_index, type);
	      }
	    dp->conversion = c;
	    dp->dir_end = cp;
	  }

	  d->count++;
	  if (d->count >= d_allocated)
	    {
	      size_t memory_size;
	      DIRECTIVE *memory;

	      d_allocated = xtimes (d_allocated, 2);
	      memory_size = xtimes (d_allocated, sizeof (DIRECTIVE));
	      if (size_overflow_p (memory_size))
		/* Overflow, would lead to out of memory.  */
		goto out_of_memory;
	      memory = (DIRECTIVE *) realloc (d->dir, memory_size);
	      if (memory == NULL)
		/* Out of memory.  */
		goto out_of_memory;
	      d->dir = memory;
	    }
	}
#if CHAR_T_ONLY_ASCII
      else if (!c_isascii (c))
	{
	  /* Non-ASCII character.  Not supported.  */
	  goto error;
	}
#endif
    }
  d->dir[d->count].dir_start = cp;

  d->max_width_length = max_width_length;
  d->max_precision_length = max_precision_length;
  return 0;

error:
  if (a->arg)
    free (a->arg);
  if (d->dir)
    free (d->dir);
  errno = EINVAL;
  return -1;

out_of_memory:
  if (a->arg)
    free (a->arg);
  if (d->dir)
    free (d->dir);
out_of_memory_1:
  errno = ENOMEM;
  return -1;
}

#undef PRINTF_PARSE
#undef DIRECTIVES
#undef DIRECTIVE
#undef CHAR_T_ONLY_ASCII
#undef CHAR_T
