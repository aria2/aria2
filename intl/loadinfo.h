/* Copyright (C) 1996-1999, 2000-2003, 2005-2006 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Contributed by Ulrich Drepper <drepper@cygnus.com>, 1996.

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

#ifndef _LOADINFO_H
#define _LOADINFO_H	1

/* Declarations of locale dependent catalog lookup functions.
   Implemented in

     localealias.c    Possibly replace a locale name by another.
     explodename.c    Split a locale name into its various fields.
     l10nflist.c      Generate a list of filenames of possible message catalogs.
     finddomain.c     Find and open the relevant message catalogs.

   The main function _nl_find_domain() in finddomain.c is declared
   in gettextP.h.
 */

#ifndef internal_function
# define internal_function
#endif

#ifndef LIBINTL_DLL_EXPORTED
# define LIBINTL_DLL_EXPORTED
#endif

/* Tell the compiler when a conditional or integer expression is
   almost always true or almost always false.  */
#ifndef HAVE_BUILTIN_EXPECT
# define __builtin_expect(expr, val) (expr)
#endif

/* Separator in PATH like lists of pathnames.  */
#if ((defined _WIN32 || defined __WIN32__) && !defined __CYGWIN__) || defined __EMX__ || defined __DJGPP__
  /* Win32, OS/2, DOS */
# define PATH_SEPARATOR ';'
#else
  /* Unix */
# define PATH_SEPARATOR ':'
#endif

/* Encoding of locale name parts.  */
#define XPG_NORM_CODESET	1
#define XPG_CODESET		2
#define XPG_TERRITORY		4
#define XPG_MODIFIER		8


struct loaded_l10nfile
{
  const char *filename;
  int decided;

  const void *data;

  struct loaded_l10nfile *next;
  struct loaded_l10nfile *successor[1];
};


/* Normalize codeset name.  There is no standard for the codeset
   names.  Normalization allows the user to use any of the common
   names.  The return value is dynamically allocated and has to be
   freed by the caller.  */
extern const char *_nl_normalize_codeset (const char *codeset,
					  size_t name_len);

/* Lookup a locale dependent file.
   *L10NFILE_LIST denotes a pool of lookup results of locale dependent
   files of the same kind, sorted in decreasing order of ->filename.
   DIRLIST and DIRLIST_LEN are an argz list of directories in which to
   look, containing at least one directory (i.e. DIRLIST_LEN > 0).
   MASK, LANGUAGE, TERRITORY, CODESET, NORMALIZED_CODESET, MODIFIER
   are the pieces of the locale name, as produced by _nl_explode_name().
   FILENAME is the filename suffix.
   The return value is the lookup result, either found in *L10NFILE_LIST,
   or - if DO_ALLOCATE is nonzero - freshly allocated, or possibly NULL.
   If the return value is non-NULL, it is added to *L10NFILE_LIST, and
   its ->next field denotes the chaining inside *L10NFILE_LIST, and
   furthermore its ->successor[] field contains a list of other lookup
   results from which this lookup result inherits.  */
extern struct loaded_l10nfile *
_nl_make_l10nflist (struct loaded_l10nfile **l10nfile_list,
		    const char *dirlist, size_t dirlist_len, int mask,
		    const char *language, const char *territory,
		    const char *codeset, const char *normalized_codeset,
		    const char *modifier,
		    const char *filename, int do_allocate);

/* Lookup the real locale name for a locale alias NAME, or NULL if
   NAME is not a locale alias (but possibly a real locale name).
   The return value is statically allocated and must not be freed.  */
/* Part of the libintl ABI only for the sake of the gettext.m4 macro.  */
extern LIBINTL_DLL_EXPORTED const char *_nl_expand_alias (const char *name);

/* Split a locale name NAME into its pieces: language, modifier,
   territory, codeset.
   NAME gets destructively modified: NUL bytes are inserted here and
   there.  *LANGUAGE gets assigned NAME.  Each of *MODIFIER, *TERRITORY,
   *CODESET gets assigned either a pointer into the old NAME string, or
   NULL.  *NORMALIZED_CODESET gets assigned the expanded *CODESET, if it
   is different from *CODESET; this one is dynamically allocated and has
   to be freed by the caller.
   The return value is a bitmask, where each bit corresponds to one
   filled-in value:
     XPG_MODIFIER                for *MODIFIER,
     XPG_TERRITORY               for *TERRITORY,
     XPG_CODESET                 for *CODESET,
     XPG_NORM_CODESET            for *NORMALIZED_CODESET.
 */
extern int _nl_explode_name (char *name, const char **language,
			     const char **modifier, const char **territory,
			     const char **codeset,
			     const char **normalized_codeset);

#endif	/* loadinfo.h */
