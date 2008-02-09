/* Determine a canonical name for the current locale's character encoding.

   Copyright (C) 2000-2006 Free Software Foundation, Inc.

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

/* Written by Bruno Haible <bruno@clisp.org>.  */

#include <config.h>

/* Specification.  */
#include "localcharset.h"

#include <stddef.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#if defined _WIN32 || defined __WIN32__
# define WIN32_NATIVE
#endif

#if defined __EMX__
/* Assume EMX program runs on OS/2, even if compiled under DOS.  */
# define OS2
#endif

#if !defined WIN32_NATIVE
# if HAVE_LANGINFO_CODESET
#  include <langinfo.h>
# else
#  if 0 /* see comment below */
#   include <locale.h>
#  endif
# endif
# ifdef __CYGWIN__
#  define WIN32_LEAN_AND_MEAN
#  include <windows.h>
# endif
#elif defined WIN32_NATIVE
# define WIN32_LEAN_AND_MEAN
# include <windows.h>
#endif
#if defined OS2
# define INCL_DOS
# include <os2.h>
#endif

#if ENABLE_RELOCATABLE
# include "relocatable.h"
#else
# define relocate(pathname) (pathname)
#endif

/* Get LIBDIR.  */
#ifndef LIBDIR
# include "configmake.h"
#endif

#if defined _WIN32 || defined __WIN32__ || defined __CYGWIN__ || defined __EMX__ || defined __DJGPP__
  /* Win32, Cygwin, OS/2, DOS */
# define ISSLASH(C) ((C) == '/' || (C) == '\\')
#endif

#ifndef DIRECTORY_SEPARATOR
# define DIRECTORY_SEPARATOR '/'
#endif

#ifndef ISSLASH
# define ISSLASH(C) ((C) == DIRECTORY_SEPARATOR)
#endif

#if HAVE_DECL_GETC_UNLOCKED
# undef getc
# define getc getc_unlocked
#endif

/* The following static variable is declared 'volatile' to avoid a
   possible multithread problem in the function get_charset_aliases. If we
   are running in a threaded environment, and if two threads initialize
   'charset_aliases' simultaneously, both will produce the same value,
   and everything will be ok if the two assignments to 'charset_aliases'
   are atomic. But I don't know what will happen if the two assignments mix.  */
#if __STDC__ != 1
# define volatile /* empty */
#endif
/* Pointer to the contents of the charset.alias file, if it has already been
   read, else NULL.  Its format is:
   ALIAS_1 '\0' CANONICAL_1 '\0' ... ALIAS_n '\0' CANONICAL_n '\0' '\0'  */
static const char * volatile charset_aliases;

/* Return a pointer to the contents of the charset.alias file.  */
static const char *
get_charset_aliases (void)
{
  const char *cp;

  cp = charset_aliases;
  if (cp == NULL)
    {
#if !(defined VMS || defined WIN32_NATIVE || defined __CYGWIN__)
      FILE *fp;
      const char *dir;
      const char *base = "charset.alias";
      char *file_name;

      /* Make it possible to override the charset.alias location.  This is
	 necessary for running the testsuite before "make install".  */
      dir = getenv ("CHARSETALIASDIR");
      if (dir == NULL || dir[0] == '\0')
	dir = relocate (LIBDIR);

      /* Concatenate dir and base into freshly allocated file_name.  */
      {
	size_t dir_len = strlen (dir);
	size_t base_len = strlen (base);
	int add_slash = (dir_len > 0 && !ISSLASH (dir[dir_len - 1]));
	file_name = (char *) malloc (dir_len + add_slash + base_len + 1);
	if (file_name != NULL)
	  {
	    memcpy (file_name, dir, dir_len);
	    if (add_slash)
	      file_name[dir_len] = DIRECTORY_SEPARATOR;
	    memcpy (file_name + dir_len + add_slash, base, base_len + 1);
	  }
      }

      if (file_name == NULL || (fp = fopen (file_name, "r")) == NULL)
	/* Out of memory or file not found, treat it as empty.  */
	cp = "";
      else
	{
	  /* Parse the file's contents.  */
	  char *res_ptr = NULL;
	  size_t res_size = 0;

	  for (;;)
	    {
	      int c;
	      char buf1[50+1];
	      char buf2[50+1];
	      size_t l1, l2;
	      char *old_res_ptr;

	      c = getc (fp);
	      if (c == EOF)
		break;
	      if (c == '\n' || c == ' ' || c == '\t')
		continue;
	      if (c == '#')
		{
		  /* Skip comment, to end of line.  */
		  do
		    c = getc (fp);
		  while (!(c == EOF || c == '\n'));
		  if (c == EOF)
		    break;
		  continue;
		}
	      ungetc (c, fp);
	      if (fscanf (fp, "%50s %50s", buf1, buf2) < 2)
		break;
	      l1 = strlen (buf1);
	      l2 = strlen (buf2);
	      old_res_ptr = res_ptr;
	      if (res_size == 0)
		{
		  res_size = l1 + 1 + l2 + 1;
		  res_ptr = (char *) malloc (res_size + 1);
		}
	      else
		{
		  res_size += l1 + 1 + l2 + 1;
		  res_ptr = (char *) realloc (res_ptr, res_size + 1);
		}
	      if (res_ptr == NULL)
		{
		  /* Out of memory. */
		  res_size = 0;
		  if (old_res_ptr != NULL)
		    free (old_res_ptr);
		  break;
		}
	      strcpy (res_ptr + res_size - (l2 + 1) - (l1 + 1), buf1);
	      strcpy (res_ptr + res_size - (l2 + 1), buf2);
	    }
	  fclose (fp);
	  if (res_size == 0)
	    cp = "";
	  else
	    {
	      *(res_ptr + res_size) = '\0';
	      cp = res_ptr;
	    }
	}

      if (file_name != NULL)
	free (file_name);

#else

# if defined VMS
      /* To avoid the troubles of an extra file charset.alias_vms in the
	 sources of many GNU packages, simply inline the aliases here.  */
      /* The list of encodings is taken from the OpenVMS 7.3-1 documentation
	 "Compaq C Run-Time Library Reference Manual for OpenVMS systems"
	 section 10.7 "Handling Different Character Sets".  */
      cp = "ISO8859-1" "\0" "ISO-8859-1" "\0"
	   "ISO8859-2" "\0" "ISO-8859-2" "\0"
	   "ISO8859-5" "\0" "ISO-8859-5" "\0"
	   "ISO8859-7" "\0" "ISO-8859-7" "\0"
	   "ISO8859-8" "\0" "ISO-8859-8" "\0"
	   "ISO8859-9" "\0" "ISO-8859-9" "\0"
	   /* Japanese */
	   "eucJP" "\0" "EUC-JP" "\0"
	   "SJIS" "\0" "SHIFT_JIS" "\0"
	   "DECKANJI" "\0" "DEC-KANJI" "\0"
	   "SDECKANJI" "\0" "EUC-JP" "\0"
	   /* Chinese */
	   "eucTW" "\0" "EUC-TW" "\0"
	   "DECHANYU" "\0" "DEC-HANYU" "\0"
	   "DECHANZI" "\0" "GB2312" "\0"
	   /* Korean */
	   "DECKOREAN" "\0" "EUC-KR" "\0";
# endif

# if defined WIN32_NATIVE || defined __CYGWIN__
      /* To avoid the troubles of installing a separate file in the same
	 directory as the DLL and of retrieving the DLL's directory at
	 runtime, simply inline the aliases here.  */

      cp = "CP936" "\0" "GBK" "\0"
	   "CP1361" "\0" "JOHAB" "\0"
	   "CP20127" "\0" "ASCII" "\0"
	   "CP20866" "\0" "KOI8-R" "\0"
	   "CP20936" "\0" "GB2312" "\0"
	   "CP21866" "\0" "KOI8-RU" "\0"
	   "CP28591" "\0" "ISO-8859-1" "\0"
	   "CP28592" "\0" "ISO-8859-2" "\0"
	   "CP28593" "\0" "ISO-8859-3" "\0"
	   "CP28594" "\0" "ISO-8859-4" "\0"
	   "CP28595" "\0" "ISO-8859-5" "\0"
	   "CP28596" "\0" "ISO-8859-6" "\0"
	   "CP28597" "\0" "ISO-8859-7" "\0"
	   "CP28598" "\0" "ISO-8859-8" "\0"
	   "CP28599" "\0" "ISO-8859-9" "\0"
	   "CP28605" "\0" "ISO-8859-15" "\0"
	   "CP38598" "\0" "ISO-8859-8" "\0"
	   "CP51932" "\0" "EUC-JP" "\0"
	   "CP51936" "\0" "GB2312" "\0"
	   "CP51949" "\0" "EUC-KR" "\0"
	   "CP51950" "\0" "EUC-TW" "\0"
	   "CP54936" "\0" "GB18030" "\0"
	   "CP65001" "\0" "UTF-8" "\0";
# endif
#endif

      charset_aliases = cp;
    }

  return cp;
}

/* Determine the current locale's character encoding, and canonicalize it
   into one of the canonical names listed in config.charset.
   The result must not be freed; it is statically allocated.
   If the canonical name cannot be determined, the result is a non-canonical
   name.  */

#ifdef STATIC
STATIC
#endif
const char *
locale_charset (void)
{
  const char *codeset;
  const char *aliases;

#if !(defined WIN32_NATIVE || defined OS2)

# if HAVE_LANGINFO_CODESET

  /* Most systems support nl_langinfo (CODESET) nowadays.  */
  codeset = nl_langinfo (CODESET);

#  ifdef __CYGWIN__
  /* Cygwin 2006 does not have locales.  nl_langinfo (CODESET) always
     returns "US-ASCII".  As long as this is not fixed, return the suffix
     of the locale name from the environment variables (if present) or
     the codepage as a number.  */
  if (codeset != NULL && strcmp (codeset, "US-ASCII") == 0)
    {
      const char *locale;
      static char buf[2 + 10 + 1];

      locale = getenv ("LC_ALL");
      if (locale == NULL || locale[0] == '\0')
	{
	  locale = getenv ("LC_CTYPE");
	  if (locale == NULL || locale[0] == '\0')
	    locale = getenv ("LANG");
	}
      if (locale != NULL && locale[0] != '\0')
	{
	  /* If the locale name contains an encoding after the dot, return
	     it.  */
	  const char *dot = strchr (locale, '.');

	  if (dot != NULL)
	    {
	      const char *modifier;

	      dot++;
	      /* Look for the possible @... trailer and remove it, if any.  */
	      modifier = strchr (dot, '@');
	      if (modifier == NULL)
		return dot;
	      if (modifier - dot < sizeof (buf))
		{
		  memcpy (buf, dot, modifier - dot);
		  buf [modifier - dot] = '\0';
		  return buf;
		}
	    }
	}

      /* Woe32 has a function returning the locale's codepage as a number.  */
      sprintf (buf, "CP%u", GetACP ());
      codeset = buf;
    }
#  endif

# else

  /* On old systems which lack it, use setlocale or getenv.  */
  const char *locale = NULL;

  /* But most old systems don't have a complete set of locales.  Some
     (like SunOS 4 or DJGPP) have only the C locale.  Therefore we don't
     use setlocale here; it would return "C" when it doesn't support the
     locale name the user has set.  */
#  if 0
  locale = setlocale (LC_CTYPE, NULL);
#  endif
  if (locale == NULL || locale[0] == '\0')
    {
      locale = getenv ("LC_ALL");
      if (locale == NULL || locale[0] == '\0')
	{
	  locale = getenv ("LC_CTYPE");
	  if (locale == NULL || locale[0] == '\0')
	    locale = getenv ("LANG");
	}
    }

  /* On some old systems, one used to set locale = "iso8859_1". On others,
     you set it to "language_COUNTRY.charset". In any case, we resolve it
     through the charset.alias file.  */
  codeset = locale;

# endif

#elif defined WIN32_NATIVE

  static char buf[2 + 10 + 1];

  /* Woe32 has a function returning the locale's codepage as a number.  */
  sprintf (buf, "CP%u", GetACP ());
  codeset = buf;

#elif defined OS2

  const char *locale;
  static char buf[2 + 10 + 1];
  ULONG cp[3];
  ULONG cplen;

  /* Allow user to override the codeset, as set in the operating system,
     with standard language environment variables.  */
  locale = getenv ("LC_ALL");
  if (locale == NULL || locale[0] == '\0')
    {
      locale = getenv ("LC_CTYPE");
      if (locale == NULL || locale[0] == '\0')
	locale = getenv ("LANG");
    }
  if (locale != NULL && locale[0] != '\0')
    {
      /* If the locale name contains an encoding after the dot, return it.  */
      const char *dot = strchr (locale, '.');

      if (dot != NULL)
	{
	  const char *modifier;

	  dot++;
	  /* Look for the possible @... trailer and remove it, if any.  */
	  modifier = strchr (dot, '@');
	  if (modifier == NULL)
	    return dot;
	  if (modifier - dot < sizeof (buf))
	    {
	      memcpy (buf, dot, modifier - dot);
	      buf [modifier - dot] = '\0';
	      return buf;
	    }
	}

      /* Resolve through the charset.alias file.  */
      codeset = locale;
    }
  else
    {
      /* OS/2 has a function returning the locale's codepage as a number.  */
      if (DosQueryCp (sizeof (cp), cp, &cplen))
	codeset = "";
      else
	{
	  sprintf (buf, "CP%u", cp[0]);
	  codeset = buf;
	}
    }

#endif

  if (codeset == NULL)
    /* The canonical name cannot be determined.  */
    codeset = "";

  /* Resolve alias. */
  for (aliases = get_charset_aliases ();
       *aliases != '\0';
       aliases += strlen (aliases) + 1, aliases += strlen (aliases) + 1)
    if (strcmp (codeset, aliases) == 0
	|| (aliases[0] == '*' && aliases[1] == '\0'))
      {
	codeset = aliases + strlen (aliases) + 1;
	break;
      }

  /* Don't return an empty string.  GNU libc and GNU libiconv interpret
     the empty string as denoting "the locale's character encoding",
     thus GNU libiconv would call this function a second time.  */
  if (codeset[0] == '\0')
    codeset = "ASCII";

  return codeset;
}
