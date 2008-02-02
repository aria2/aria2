/* Header describing internals of libintl library.
   Copyright (C) 1995-1999, 2000-2007 Free Software Foundation, Inc.
   Written by Ulrich Drepper <drepper@cygnus.com>, 1995.

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

#ifndef _GETTEXTP_H
#define _GETTEXTP_H

#include <stddef.h>		/* Get size_t.  */

#ifdef _LIBC
# include "../iconv/gconv_int.h"
#else
# if HAVE_ICONV
#  include <iconv.h>
# endif
#endif

/* Handle multi-threaded applications.  */
#ifdef _LIBC
# include <bits/libc-lock.h>
# define gl_rwlock_define __libc_rwlock_define
#else
# include "lock.h"
#endif

#ifdef _LIBC
extern char *__gettext (const char *__msgid);
extern char *__dgettext (const char *__domainname, const char *__msgid);
extern char *__dcgettext (const char *__domainname, const char *__msgid,
			  int __category);
extern char *__ngettext (const char *__msgid1, const char *__msgid2,
			 unsigned long int __n);
extern char *__dngettext (const char *__domainname,
			  const char *__msgid1, const char *__msgid2,
			  unsigned long int n);
extern char *__dcngettext (const char *__domainname,
			   const char *__msgid1, const char *__msgid2,
			   unsigned long int __n, int __category);
extern char *__dcigettext (const char *__domainname,
			   const char *__msgid1, const char *__msgid2,
			   int __plural, unsigned long int __n,
			   int __category);
extern char *__textdomain (const char *__domainname);
extern char *__bindtextdomain (const char *__domainname,
			       const char *__dirname);
extern char *__bind_textdomain_codeset (const char *__domainname,
					const char *__codeset);
extern void _nl_finddomain_subfreeres (void) attribute_hidden;
extern void _nl_unload_domain (struct loaded_domain *__domain)
     internal_function attribute_hidden;
#else
/* Declare the exported libintl_* functions, in a way that allows us to
   call them under their real name.  */
# undef _INTL_REDIRECT_INLINE
# undef _INTL_REDIRECT_MACROS
# define _INTL_REDIRECT_MACROS
# include "libgnuintl.h"
# ifdef IN_LIBGLOCALE
extern char *gl_dcigettext (const char *__domainname,
			    const char *__msgid1, const char *__msgid2,
			    int __plural, unsigned long int __n,
			    int __category,
			    const char *__localename, const char *__encoding);
# else
extern char *libintl_dcigettext (const char *__domainname,
				 const char *__msgid1, const char *__msgid2,
				 int __plural, unsigned long int __n,
				 int __category);
# endif
#endif

#include "loadinfo.h"

#include "gmo.h"		/* Get nls_uint32.  */

/* @@ end of prolog @@ */

#ifndef internal_function
# define internal_function
#endif

#ifndef attribute_hidden
# define attribute_hidden
#endif

/* Tell the compiler when a conditional or integer expression is
   almost always true or almost always false.  */
#ifndef HAVE_BUILTIN_EXPECT
# define __builtin_expect(expr, val) (expr)
#endif

#ifndef W
# define W(flag, data) ((flag) ? SWAP (data) : (data))
#endif


#ifdef _LIBC
# include <byteswap.h>
# define SWAP(i) bswap_32 (i)
#else
static inline nls_uint32
# ifdef __cplusplus
SWAP (nls_uint32 i)
# else
SWAP (i)
     nls_uint32 i;
# endif
{
  return (i << 24) | ((i & 0xff00) << 8) | ((i >> 8) & 0xff00) | (i >> 24);
}
#endif


/* In-memory representation of system dependent string.  */
struct sysdep_string_desc
{
  /* Length of addressed string, including the trailing NUL.  */
  size_t length;
  /* Pointer to addressed string.  */
  const char *pointer;
};

/* Cache of translated strings after charset conversion.
   Note: The strings are converted to the target encoding only on an as-needed
   basis.  */
struct converted_domain
{
  /* The target encoding name.  */
  const char *encoding;
  /* The descriptor for conversion from the message catalog's encoding to
     this target encoding.  */
#ifdef _LIBC
  __gconv_t conv;
#else
# if HAVE_ICONV
  iconv_t conv;
# endif
#endif
  /* The table of translated strings after charset conversion.  */
  char **conv_tab;
};

/* The representation of an opened message catalog.  */
struct loaded_domain
{
  /* Pointer to memory containing the .mo file.  */
  const char *data;
  /* 1 if the memory is mmap()ed, 0 if the memory is malloc()ed.  */
  int use_mmap;
  /* Size of mmap()ed memory.  */
  size_t mmap_size;
  /* 1 if the .mo file uses a different endianness than this machine.  */
  int must_swap;
  /* Pointer to additional malloc()ed memory.  */
  void *malloced;

  /* Number of static strings pairs.  */
  nls_uint32 nstrings;
  /* Pointer to descriptors of original strings in the file.  */
  const struct string_desc *orig_tab;
  /* Pointer to descriptors of translated strings in the file.  */
  const struct string_desc *trans_tab;

  /* Number of system dependent strings pairs.  */
  nls_uint32 n_sysdep_strings;
  /* Pointer to descriptors of original sysdep strings.  */
  const struct sysdep_string_desc *orig_sysdep_tab;
  /* Pointer to descriptors of translated sysdep strings.  */
  const struct sysdep_string_desc *trans_sysdep_tab;

  /* Size of hash table.  */
  nls_uint32 hash_size;
  /* Pointer to hash table.  */
  const nls_uint32 *hash_tab;
  /* 1 if the hash table uses a different endianness than this machine.  */
  int must_swap_hash_tab;

  /* Cache of charset conversions of the translated strings.  */
  struct converted_domain *conversions;
  size_t nconversions;
  gl_rwlock_define (, conversions_lock)

  const struct expression *plural;
  unsigned long int nplurals;
};

/* We want to allocate a string at the end of the struct.  But ISO C
   doesn't allow zero sized arrays.  */
#ifdef __GNUC__
# define ZERO 0
#else
# define ZERO 1
#endif

/* A set of settings bound to a message domain.  Used to store settings
   from bindtextdomain() and bind_textdomain_codeset().  */
struct binding
{
  struct binding *next;
  char *dirname;
  char *codeset;
  char domainname[ZERO];
};

/* A counter which is incremented each time some previous translations
   become invalid.
   This variable is part of the external ABI of the GNU libintl.  */
#ifdef IN_LIBGLOCALE
# include <glocale/config.h>
extern LIBGLOCALE_DLL_EXPORTED int _nl_msg_cat_cntr;
#else
extern LIBINTL_DLL_EXPORTED int _nl_msg_cat_cntr;
#endif

#ifndef _LIBC
extern const char *_nl_language_preferences_default (void);
# define gl_locale_name_canonicalize _nl_locale_name_canonicalize
extern void _nl_locale_name_canonicalize (char *name);
# define gl_locale_name_posix _nl_locale_name_posix
extern const char *_nl_locale_name_posix (int category,
					  const char *categoryname);
# define gl_locale_name_default _nl_locale_name_default
extern const char *_nl_locale_name_default (void);
# define gl_locale_name _nl_locale_name
extern const char *_nl_locale_name (int category, const char *categoryname);
#endif

struct loaded_l10nfile *_nl_find_domain (const char *__dirname, char *__locale,
					 const char *__domainname,
					 struct binding *__domainbinding)
     internal_function;
void _nl_load_domain (struct loaded_l10nfile *__domain,
		      struct binding *__domainbinding)
     internal_function;

#ifdef IN_LIBGLOCALE
char *_nl_find_msg (struct loaded_l10nfile *domain_file,
		    struct binding *domainbinding, const char *encoding,
		    const char *msgid,
		    size_t *lengthp)
     internal_function;
#else
char *_nl_find_msg (struct loaded_l10nfile *domain_file,
		    struct binding *domainbinding, const char *msgid,
		    int convert, size_t *lengthp)
     internal_function;
#endif

/* The internal variables in the standalone libintl.a must have different
   names than the internal variables in GNU libc, otherwise programs
   using libintl.a cannot be linked statically.  */
#if !defined _LIBC
# define _nl_default_dirname libintl_nl_default_dirname
# define _nl_domain_bindings libintl_nl_domain_bindings
#endif

/* Contains the default location of the message catalogs.  */
extern const char _nl_default_dirname[];
#ifdef _LIBC
libc_hidden_proto (_nl_default_dirname)
#endif

/* List with bindings of specific domains.  */
extern struct binding *_nl_domain_bindings;

/* The internal variables in the standalone libintl.a must have different
   names than the internal variables in GNU libc, otherwise programs
   using libintl.a cannot be linked statically.  */
#if !defined _LIBC
# define _nl_default_default_domain libintl_nl_default_default_domain
# define _nl_current_default_domain libintl_nl_current_default_domain
#endif

/* Name of the default text domain.  */
extern const char _nl_default_default_domain[] attribute_hidden;

/* Default text domain in which entries for gettext(3) are to be found.  */
extern const char *_nl_current_default_domain attribute_hidden;

/* @@ begin of epilog @@ */

#endif /* gettextP.h  */
