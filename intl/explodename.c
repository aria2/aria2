/* Copyright (C) 1995-1998, 2000-2001, 2003, 2005, 2007 Free Software Foundation, Inc.
   Contributed by Ulrich Drepper <drepper@gnu.ai.mit.edu>, 1995.

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

#include <stdlib.h>
#include <string.h>
#include <sys/types.h>

#include "loadinfo.h"

/* On some strange systems still no definition of NULL is found.  Sigh!  */
#ifndef NULL
# if defined __STDC__ && __STDC__
#  define NULL ((void *) 0)
# else
#  define NULL 0
# endif
#endif

/* @@ end of prolog @@ */

/* Split a locale name NAME into a leading language part and all the
   rest.  Return a pointer to the first character after the language,
   i.e. to the first byte of the rest.  */
static char *_nl_find_language (const char *name);

static char *
_nl_find_language (const char *name)
{
  while (name[0] != '\0' && name[0] != '_' && name[0] != '@' && name[0] != '.')
    ++name;

  return (char *) name;
}


int
_nl_explode_name (char *name,
		  const char **language, const char **modifier,
		  const char **territory, const char **codeset,
		  const char **normalized_codeset)
{
  char *cp;
  int mask;

  *modifier = NULL;
  *territory = NULL;
  *codeset = NULL;
  *normalized_codeset = NULL;

  /* Now we determine the single parts of the locale name.  First
     look for the language.  Termination symbols are `_', '.', and `@'.  */
  mask = 0;
  *language = cp = name;
  cp = _nl_find_language (*language);

  if (*language == cp)
    /* This does not make sense: language has to be specified.  Use
       this entry as it is without exploding.  Perhaps it is an alias.  */
    cp = strchr (*language, '\0');
  else
    {
      if (cp[0] == '_')
	{
	  /* Next is the territory.  */
	  cp[0] = '\0';
	  *territory = ++cp;

	  while (cp[0] != '\0' && cp[0] != '.' && cp[0] != '@')
	    ++cp;

	  mask |= XPG_TERRITORY;
	}

      if (cp[0] == '.')
	{
	  /* Next is the codeset.  */
	  cp[0] = '\0';
	  *codeset = ++cp;

	  while (cp[0] != '\0' && cp[0] != '@')
	    ++cp;

	  mask |= XPG_CODESET;

	  if (*codeset != cp && (*codeset)[0] != '\0')
	    {
	      *normalized_codeset = _nl_normalize_codeset (*codeset,
							   cp - *codeset);
	      if (*normalized_codeset == NULL)
		return -1;
	      else if (strcmp (*codeset, *normalized_codeset) == 0)
		free ((char *) *normalized_codeset);
	      else
		mask |= XPG_NORM_CODESET;
	    }
	}
    }

  if (cp[0] == '@')
    {
      /* Next is the modifier.  */
      cp[0] = '\0';
      *modifier = ++cp;

      if (cp[0] != '\0')
	mask |= XPG_MODIFIER;
    }

  if (*territory != NULL && (*territory)[0] == '\0')
    mask &= ~XPG_TERRITORY;

  if (*codeset != NULL && (*codeset)[0] == '\0')
    mask &= ~XPG_CODESET;

  return mask;
}
