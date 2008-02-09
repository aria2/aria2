/* Implementation of the internal dcigettext function.
   Copyright (C) 1995-1999, 2000-2007 Free Software Foundation, Inc.

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

/* Tell glibc's <string.h> to provide a prototype for mempcpy().
   This must come before <config.h> because <config.h> may include
   <features.h>, and once <features.h> has been included, it's too late.  */
#ifndef _GNU_SOURCE
# define _GNU_SOURCE	1
#endif

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

/* NL_LOCALE_NAME does not work in glibc-2.4.  Ignore it.  */
#undef HAVE_NL_LOCALE_NAME

#include <sys/types.h>

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

#include <errno.h>
#ifndef errno
extern int errno;
#endif
#ifndef __set_errno
# define __set_errno(val) errno = (val)
#endif

#include <stddef.h>
#include <stdlib.h>
#include <string.h>

#if defined HAVE_UNISTD_H || defined _LIBC
# include <unistd.h>
#endif

#include <locale.h>

#ifdef _LIBC
  /* Guess whether integer division by zero raises signal SIGFPE.
     Set to 1 only if you know for sure.  In case of doubt, set to 0.  */
# if defined __alpha__ || defined __arm__ || defined __i386__ \
     || defined __m68k__ || defined __s390__
#  define INTDIV0_RAISES_SIGFPE 1
# else
#  define INTDIV0_RAISES_SIGFPE 0
# endif
#endif
#if !INTDIV0_RAISES_SIGFPE
# include <signal.h>
#endif

#if defined HAVE_SYS_PARAM_H || defined _LIBC
# include <sys/param.h>
#endif

#if !defined _LIBC
# if HAVE_NL_LOCALE_NAME
#  include <langinfo.h>
# endif
# include "localcharset.h"
#endif

#include "gettextP.h"
#include "plural-exp.h"
#ifdef _LIBC
# include <libintl.h>
#else
# ifdef IN_LIBGLOCALE
#  include <libintl.h>
# endif
# include "libgnuintl.h"
#endif
#include "hash-string.h"

/* Handle multi-threaded applications.  */
#ifdef _LIBC
# include <bits/libc-lock.h>
# define gl_rwlock_define_initialized __libc_rwlock_define_initialized
# define gl_rwlock_rdlock __libc_rwlock_rdlock
# define gl_rwlock_wrlock __libc_rwlock_wrlock
# define gl_rwlock_unlock __libc_rwlock_unlock
#else
# include "lock.h"
#endif

/* Alignment of types.  */
#if defined __GNUC__ && __GNUC__ >= 2
# define alignof(TYPE) __alignof__ (TYPE)
#else
# define alignof(TYPE) \
    ((int) &((struct { char dummy1; TYPE dummy2; } *) 0)->dummy2)
#endif

/* Some compilers, like SunOS4 cc, don't have offsetof in <stddef.h>.  */
#ifndef offsetof
# define offsetof(type,ident) ((size_t)&(((type*)0)->ident))
#endif

/* @@ end of prolog @@ */

#ifdef _LIBC
/* Rename the non ANSI C functions.  This is required by the standard
   because some ANSI C functions will require linking with this object
   file and the name space must not be polluted.  */
# define getcwd __getcwd
# ifndef stpcpy
#  define stpcpy __stpcpy
# endif
# define tfind __tfind
#else
# if !defined HAVE_GETCWD
char *getwd ();
#  define getcwd(buf, max) getwd (buf)
# else
#  if VMS
#   define getcwd(buf, max) (getcwd) (buf, max, 0)
#  else
char *getcwd ();
#  endif
# endif
# ifndef HAVE_STPCPY
static char *stpcpy (char *dest, const char *src);
# endif
# ifndef HAVE_MEMPCPY
static void *mempcpy (void *dest, const void *src, size_t n);
# endif
#endif

/* Use a replacement if the system does not provide the `tsearch' function
   family.  */
#if HAVE_TSEARCH || defined _LIBC
# include <search.h>
#else
# define tsearch libintl_tsearch
# define tfind libintl_tfind
# define tdelete libintl_tdelete
# define twalk libintl_twalk
# include "tsearch.h"
#endif

#ifdef _LIBC
# define tsearch __tsearch
#endif

/* Amount to increase buffer size by in each try.  */
#define PATH_INCR 32

/* The following is from pathmax.h.  */
/* Non-POSIX BSD systems might have gcc's limits.h, which doesn't define
   PATH_MAX but might cause redefinition warnings when sys/param.h is
   later included (as on MORE/BSD 4.3).  */
#if defined _POSIX_VERSION || (defined HAVE_LIMITS_H && !defined __GNUC__)
# include <limits.h>
#endif

#ifndef _POSIX_PATH_MAX
# define _POSIX_PATH_MAX 255
#endif

#if !defined PATH_MAX && defined _PC_PATH_MAX
# define PATH_MAX (pathconf ("/", _PC_PATH_MAX) < 1 ? 1024 : pathconf ("/", _PC_PATH_MAX))
#endif

/* Don't include sys/param.h if it already has been.  */
#if defined HAVE_SYS_PARAM_H && !defined PATH_MAX && !defined MAXPATHLEN
# include <sys/param.h>
#endif

#if !defined PATH_MAX && defined MAXPATHLEN
# define PATH_MAX MAXPATHLEN
#endif

#ifndef PATH_MAX
# define PATH_MAX _POSIX_PATH_MAX
#endif

/* Pathname support.
   ISSLASH(C)           tests whether C is a directory separator character.
   IS_ABSOLUTE_PATH(P)  tests whether P is an absolute path.  If it is not,
                        it may be concatenated to a directory pathname.
   IS_PATH_WITH_DIR(P)  tests whether P contains a directory specification.
 */
#if defined _WIN32 || defined __WIN32__ || defined __CYGWIN__ || defined __EMX__ || defined __DJGPP__
  /* Win32, Cygwin, OS/2, DOS */
# define ISSLASH(C) ((C) == '/' || (C) == '\\')
# define HAS_DEVICE(P) \
    ((((P)[0] >= 'A' && (P)[0] <= 'Z') || ((P)[0] >= 'a' && (P)[0] <= 'z')) \
     && (P)[1] == ':')
# define IS_ABSOLUTE_PATH(P) (ISSLASH ((P)[0]) || HAS_DEVICE (P))
# define IS_PATH_WITH_DIR(P) \
    (strchr (P, '/') != NULL || strchr (P, '\\') != NULL || HAS_DEVICE (P))
#else
  /* Unix */
# define ISSLASH(C) ((C) == '/')
# define IS_ABSOLUTE_PATH(P) ISSLASH ((P)[0])
# define IS_PATH_WITH_DIR(P) (strchr (P, '/') != NULL)
#endif

/* Whether to support different locales in different threads.  */
#if defined _LIBC || HAVE_NL_LOCALE_NAME || (HAVE_STRUCT___LOCALE_STRUCT___NAMES && defined USE_IN_GETTEXT_TESTS) || defined IN_LIBGLOCALE
# define HAVE_PER_THREAD_LOCALE
#endif

/* This is the type used for the search tree where known translations
   are stored.  */
struct known_translation_t
{
  /* Domain in which to search.  */
  const char *domainname;

  /* The category.  */
  int category;

#ifdef HAVE_PER_THREAD_LOCALE
  /* Name of the relevant locale category, or "" for the global locale.  */
  const char *localename;
#endif

#ifdef IN_LIBGLOCALE
  /* The character encoding.  */
  const char *encoding;
#endif

  /* State of the catalog counter at the point the string was found.  */
  int counter;

  /* Catalog where the string was found.  */
  struct loaded_l10nfile *domain;

  /* And finally the translation.  */
  const char *translation;
  size_t translation_length;

  /* Pointer to the string in question.  */
  char msgid[ZERO];
};

gl_rwlock_define_initialized (static, tree_lock)

/* Root of the search tree with known translations.  */
static void *root;

/* Function to compare two entries in the table of known translations.  */
static int
transcmp (const void *p1, const void *p2)
{
  const struct known_translation_t *s1;
  const struct known_translation_t *s2;
  int result;

  s1 = (const struct known_translation_t *) p1;
  s2 = (const struct known_translation_t *) p2;

  result = strcmp (s1->msgid, s2->msgid);
  if (result == 0)
    {
      result = strcmp (s1->domainname, s2->domainname);
      if (result == 0)
	{
#ifdef HAVE_PER_THREAD_LOCALE
	  result = strcmp (s1->localename, s2->localename);
	  if (result == 0)
#endif
	    {
#ifdef IN_LIBGLOCALE
	      result = strcmp (s1->encoding, s2->encoding);
	      if (result == 0)
#endif
		/* We compare the category last (though this is the cheapest
		   operation) since it is hopefully always the same (namely
		   LC_MESSAGES).  */
		result = s1->category - s2->category;
	    }
	}
    }

  return result;
}

/* Name of the default domain used for gettext(3) prior any call to
   textdomain(3).  The default value for this is "messages".  */
const char _nl_default_default_domain[] attribute_hidden = "messages";

#ifndef IN_LIBGLOCALE
/* Value used as the default domain for gettext(3).  */
const char *_nl_current_default_domain attribute_hidden
     = _nl_default_default_domain;
#endif

/* Contains the default location of the message catalogs.  */
#if defined __EMX__
extern const char _nl_default_dirname[];
#else
# ifdef _LIBC
extern const char _nl_default_dirname[];
libc_hidden_proto (_nl_default_dirname)
# endif
const char _nl_default_dirname[] = LOCALEDIR;
# ifdef _LIBC
libc_hidden_data_def (_nl_default_dirname)
# endif
#endif

#ifndef IN_LIBGLOCALE
/* List with bindings of specific domains created by bindtextdomain()
   calls.  */
struct binding *_nl_domain_bindings;
#endif

/* Prototypes for local functions.  */
static char *plural_lookup (struct loaded_l10nfile *domain,
			    unsigned long int n,
			    const char *translation, size_t translation_len)
     internal_function;

#ifdef IN_LIBGLOCALE
static const char *guess_category_value (int category,
					 const char *categoryname,
					 const char *localename)
     internal_function;
#else
static const char *guess_category_value (int category,
					 const char *categoryname)
     internal_function;
#endif

#ifdef _LIBC
# include "../locale/localeinfo.h"
# define category_to_name(category) \
  _nl_category_names.str + _nl_category_name_idxs[category]
#else
static const char *category_to_name (int category) internal_function;
#endif
#if (defined _LIBC || HAVE_ICONV) && !defined IN_LIBGLOCALE
static const char *get_output_charset (struct binding *domainbinding)
     internal_function;
#endif


/* For those loosing systems which don't have `alloca' we have to add
   some additional code emulating it.  */
#ifdef HAVE_ALLOCA
/* Nothing has to be done.  */
# define freea(p) /* nothing */
# define ADD_BLOCK(list, address) /* nothing */
# define FREE_BLOCKS(list) /* nothing */
#else
struct block_list
{
  void *address;
  struct block_list *next;
};
# define ADD_BLOCK(list, addr)						      \
  do {									      \
    struct block_list *newp = (struct block_list *) malloc (sizeof (*newp));  \
    /* If we cannot get a free block we cannot add the new element to	      \
       the list.  */							      \
    if (newp != NULL) {							      \
      newp->address = (addr);						      \
      newp->next = (list);						      \
      (list) = newp;							      \
    }									      \
  } while (0)
# define FREE_BLOCKS(list)						      \
  do {									      \
    while (list != NULL) {						      \
      struct block_list *old = list;					      \
      list = list->next;						      \
      free (old->address);						      \
      free (old);							      \
    }									      \
  } while (0)
# undef alloca
# define alloca(size) (malloc (size))
# define freea(p) free (p)
#endif	/* have alloca */


#ifdef _LIBC
/* List of blocks allocated for translations.  */
typedef struct transmem_list
{
  struct transmem_list *next;
  char data[ZERO];
} transmem_block_t;
static struct transmem_list *transmem_list;
#else
typedef unsigned char transmem_block_t;
#endif


/* Names for the libintl functions are a problem.  They must not clash
   with existing names and they should follow ANSI C.  But this source
   code is also used in GNU C Library where the names have a __
   prefix.  So we have to make a difference here.  */
#ifdef _LIBC
# define DCIGETTEXT __dcigettext
#else
# define DCIGETTEXT libintl_dcigettext
#endif

/* Lock variable to protect the global data in the gettext implementation.  */
gl_rwlock_define_initialized (, _nl_state_lock attribute_hidden)

/* Checking whether the binaries runs SUID must be done and glibc provides
   easier methods therefore we make a difference here.  */
#ifdef _LIBC
# define ENABLE_SECURE __libc_enable_secure
# define DETERMINE_SECURE
#else
# ifndef HAVE_GETUID
#  define getuid() 0
# endif
# ifndef HAVE_GETGID
#  define getgid() 0
# endif
# ifndef HAVE_GETEUID
#  define geteuid() getuid()
# endif
# ifndef HAVE_GETEGID
#  define getegid() getgid()
# endif
static int enable_secure;
# define ENABLE_SECURE (enable_secure == 1)
# define DETERMINE_SECURE \
  if (enable_secure == 0)						      \
    {									      \
      if (getuid () != geteuid () || getgid () != getegid ())		      \
	enable_secure = 1;						      \
      else								      \
	enable_secure = -1;						      \
    }
#endif

/* Get the function to evaluate the plural expression.  */
#include "eval-plural.h"

/* Look up MSGID in the DOMAINNAME message catalog for the current
   CATEGORY locale and, if PLURAL is nonzero, search over string
   depending on the plural form determined by N.  */
#ifdef IN_LIBGLOCALE
char *
gl_dcigettext (const char *domainname,
	       const char *msgid1, const char *msgid2,
	       int plural, unsigned long int n,
	       int category,
	       const char *localename, const char *encoding)
#else
char *
DCIGETTEXT (const char *domainname, const char *msgid1, const char *msgid2,
	    int plural, unsigned long int n, int category)
#endif
{
#ifndef HAVE_ALLOCA
  struct block_list *block_list = NULL;
#endif
  struct loaded_l10nfile *domain;
  struct binding *binding;
  const char *categoryname;
  const char *categoryvalue;
  const char *dirname;
  char *xdomainname;
  char *single_locale;
  char *retval;
  size_t retlen;
  int saved_errno;
  struct known_translation_t *search;
  struct known_translation_t **foundp = NULL;
  size_t msgid_len;
#if defined HAVE_PER_THREAD_LOCALE && !defined IN_LIBGLOCALE
  const char *localename;
#endif
  size_t domainname_len;

  /* If no real MSGID is given return NULL.  */
  if (msgid1 == NULL)
    return NULL;

#ifdef _LIBC
  if (category < 0 || category >= __LC_LAST || category == LC_ALL)
    /* Bogus.  */
    return (plural == 0
	    ? (char *) msgid1
	    /* Use the Germanic plural rule.  */
	    : n == 1 ? (char *) msgid1 : (char *) msgid2);
#endif

  /* Preserve the `errno' value.  */
  saved_errno = errno;

  gl_rwlock_rdlock (_nl_state_lock);

  /* If DOMAINNAME is NULL, we are interested in the default domain.  If
     CATEGORY is not LC_MESSAGES this might not make much sense but the
     definition left this undefined.  */
  if (domainname == NULL)
    domainname = _nl_current_default_domain;

  /* OS/2 specific: backward compatibility with older libintl versions  */
#ifdef LC_MESSAGES_COMPAT
  if (category == LC_MESSAGES_COMPAT)
    category = LC_MESSAGES;
#endif

  msgid_len = strlen (msgid1) + 1;

  /* Try to find the translation among those which we found at
     some time.  */
  search = (struct known_translation_t *)
	   alloca (offsetof (struct known_translation_t, msgid) + msgid_len);
  memcpy (search->msgid, msgid1, msgid_len);
  search->domainname = domainname;
  search->category = category;
#ifdef HAVE_PER_THREAD_LOCALE
# ifndef IN_LIBGLOCALE
#  ifdef _LIBC
  localename = __current_locale_name (category);
#  else
#   if HAVE_NL_LOCALE_NAME
  /* NL_LOCALE_NAME is public glibc API introduced in glibc-2.4.  */
  localename = nl_langinfo (NL_LOCALE_NAME (category));
#   else
#    if HAVE_STRUCT___LOCALE_STRUCT___NAMES && defined USE_IN_GETTEXT_TESTS
  /* The __names field is not public glibc API and must therefore not be used
     in code that is installed in public locations.  */
  {
    locale_t thread_locale = uselocale (NULL);
    if (thread_locale != LC_GLOBAL_LOCALE)
      localename = thread_locale->__names[category];
    else
      localename = "";
  }
#    endif
#   endif
#  endif
# endif
  search->localename = localename;
# ifdef IN_LIBGLOCALE
  search->encoding = encoding;
# endif

  /* Since tfind/tsearch manage a balanced tree, concurrent tfind and
     tsearch calls can be fatal.  */
  gl_rwlock_rdlock (tree_lock);

  foundp = (struct known_translation_t **) tfind (search, &root, transcmp);

  gl_rwlock_unlock (tree_lock);

  freea (search);
  if (foundp != NULL && (*foundp)->counter == _nl_msg_cat_cntr)
    {
      /* Now deal with plural.  */
      if (plural)
	retval = plural_lookup ((*foundp)->domain, n, (*foundp)->translation,
				(*foundp)->translation_length);
      else
	retval = (char *) (*foundp)->translation;

      gl_rwlock_unlock (_nl_state_lock);
      __set_errno (saved_errno);
      return retval;
    }
#endif

  /* See whether this is a SUID binary or not.  */
  DETERMINE_SECURE;

  /* First find matching binding.  */
#ifdef IN_LIBGLOCALE
  /* We can use a trivial binding, since _nl_find_msg will ignore it anyway,
     and _nl_load_domain and _nl_find_domain just pass it through.  */
  binding = NULL;
  dirname = bindtextdomain (domainname, NULL);
#else
  for (binding = _nl_domain_bindings; binding != NULL; binding = binding->next)
    {
      int compare = strcmp (domainname, binding->domainname);
      if (compare == 0)
	/* We found it!  */
	break;
      if (compare < 0)
	{
	  /* It is not in the list.  */
	  binding = NULL;
	  break;
	}
    }

  if (binding == NULL)
    dirname = _nl_default_dirname;
  else
    {
      dirname = binding->dirname;
#endif
      if (!IS_ABSOLUTE_PATH (dirname))
	{
	  /* We have a relative path.  Make it absolute now.  */
	  size_t dirname_len = strlen (dirname) + 1;
	  size_t path_max;
	  char *resolved_dirname;
	  char *ret;

	  path_max = (unsigned int) PATH_MAX;
	  path_max += 2;		/* The getcwd docs say to do this.  */

	  for (;;)
	    {
	      resolved_dirname = (char *) alloca (path_max + dirname_len);
	      ADD_BLOCK (block_list, tmp_dirname);

	      __set_errno (0);
	      ret = getcwd (resolved_dirname, path_max);
	      if (ret != NULL || errno != ERANGE)
		break;

	      path_max += path_max / 2;
	      path_max += PATH_INCR;
	    }

	  if (ret == NULL)
	    /* We cannot get the current working directory.  Don't signal an
	       error but simply return the default string.  */
	    goto return_untranslated;

	  stpcpy (stpcpy (strchr (resolved_dirname, '\0'), "/"), dirname);
	  dirname = resolved_dirname;
	}
#ifndef IN_LIBGLOCALE
    }
#endif

  /* Now determine the symbolic name of CATEGORY and its value.  */
  categoryname = category_to_name (category);
#ifdef IN_LIBGLOCALE
  categoryvalue = guess_category_value (category, categoryname, localename);
#else
  categoryvalue = guess_category_value (category, categoryname);
#endif

  domainname_len = strlen (domainname);
  xdomainname = (char *) alloca (strlen (categoryname)
				 + domainname_len + 5);
  ADD_BLOCK (block_list, xdomainname);

  stpcpy ((char *) mempcpy (stpcpy (stpcpy (xdomainname, categoryname), "/"),
			    domainname, domainname_len),
	  ".mo");

  /* Creating working area.  */
  single_locale = (char *) alloca (strlen (categoryvalue) + 1);
  ADD_BLOCK (block_list, single_locale);


  /* Search for the given string.  This is a loop because we perhaps
     got an ordered list of languages to consider for the translation.  */
  while (1)
    {
      /* Make CATEGORYVALUE point to the next element of the list.  */
      while (categoryvalue[0] != '\0' && categoryvalue[0] == ':')
	++categoryvalue;
      if (categoryvalue[0] == '\0')
	{
	  /* The whole contents of CATEGORYVALUE has been searched but
	     no valid entry has been found.  We solve this situation
	     by implicitly appending a "C" entry, i.e. no translation
	     will take place.  */
	  single_locale[0] = 'C';
	  single_locale[1] = '\0';
	}
      else
	{
	  char *cp = single_locale;
	  while (categoryvalue[0] != '\0' && categoryvalue[0] != ':')
	    *cp++ = *categoryvalue++;
	  *cp = '\0';

	  /* When this is a SUID binary we must not allow accessing files
	     outside the dedicated directories.  */
	  if (ENABLE_SECURE && IS_PATH_WITH_DIR (single_locale))
	    /* Ingore this entry.  */
	    continue;
	}

      /* If the current locale value is C (or POSIX) we don't load a
	 domain.  Return the MSGID.  */
      if (strcmp (single_locale, "C") == 0
	  || strcmp (single_locale, "POSIX") == 0)
	break;

      /* Find structure describing the message catalog matching the
	 DOMAINNAME and CATEGORY.  */
      domain = _nl_find_domain (dirname, single_locale, xdomainname, binding);

      if (domain != NULL)
	{
#if defined IN_LIBGLOCALE
	  retval = _nl_find_msg (domain, binding, encoding, msgid1, &retlen);
#else
	  retval = _nl_find_msg (domain, binding, msgid1, 1, &retlen);
#endif

	  if (retval == NULL)
	    {
	      int cnt;

	      for (cnt = 0; domain->successor[cnt] != NULL; ++cnt)
		{
#if defined IN_LIBGLOCALE
		  retval = _nl_find_msg (domain->successor[cnt], binding,
					 encoding, msgid1, &retlen);
#else
		  retval = _nl_find_msg (domain->successor[cnt], binding,
					 msgid1, 1, &retlen);
#endif

		  if (retval != NULL)
		    {
		      domain = domain->successor[cnt];
		      break;
		    }
		}
	    }

	  /* Returning -1 means that some resource problem exists
	     (likely memory) and that the strings could not be
	     converted.  Return the original strings.  */
	  if (__builtin_expect (retval == (char *) -1, 0))
	    break;

	  if (retval != NULL)
	    {
	      /* Found the translation of MSGID1 in domain DOMAIN:
		 starting at RETVAL, RETLEN bytes.  */
	      FREE_BLOCKS (block_list);
	      if (foundp == NULL)
		{
		  /* Create a new entry and add it to the search tree.  */
		  size_t size;
		  struct known_translation_t *newp;

		  size = offsetof (struct known_translation_t, msgid)
			 + msgid_len + domainname_len + 1;
#ifdef HAVE_PER_THREAD_LOCALE
		  size += strlen (localename) + 1;
#endif
		  newp = (struct known_translation_t *) malloc (size);
		  if (newp != NULL)
		    {
		      char *new_domainname;
#ifdef HAVE_PER_THREAD_LOCALE
		      char *new_localename;
#endif

		      new_domainname =
			(char *) mempcpy (newp->msgid, msgid1, msgid_len);
		      memcpy (new_domainname, domainname, domainname_len + 1);
#ifdef HAVE_PER_THREAD_LOCALE
		      new_localename = new_domainname + domainname_len + 1;
		      strcpy (new_localename, localename);
#endif
		      newp->domainname = new_domainname;
		      newp->category = category;
#ifdef HAVE_PER_THREAD_LOCALE
		      newp->localename = new_localename;
#endif
#ifdef IN_LIBGLOCALE
		      newp->encoding = encoding;
#endif
		      newp->counter = _nl_msg_cat_cntr;
		      newp->domain = domain;
		      newp->translation = retval;
		      newp->translation_length = retlen;

		      gl_rwlock_wrlock (tree_lock);

		      /* Insert the entry in the search tree.  */
		      foundp = (struct known_translation_t **)
			tsearch (newp, &root, transcmp);

		      gl_rwlock_unlock (tree_lock);

		      if (foundp == NULL
			  || __builtin_expect (*foundp != newp, 0))
			/* The insert failed.  */
			free (newp);
		    }
		}
	      else
		{
		  /* We can update the existing entry.  */
		  (*foundp)->counter = _nl_msg_cat_cntr;
		  (*foundp)->domain = domain;
		  (*foundp)->translation = retval;
		  (*foundp)->translation_length = retlen;
		}

	      __set_errno (saved_errno);

	      /* Now deal with plural.  */
	      if (plural)
		retval = plural_lookup (domain, n, retval, retlen);

	      gl_rwlock_unlock (_nl_state_lock);
	      return retval;
	    }
	}
    }

 return_untranslated:
  /* Return the untranslated MSGID.  */
  FREE_BLOCKS (block_list);
  gl_rwlock_unlock (_nl_state_lock);
#ifndef _LIBC
  if (!ENABLE_SECURE)
    {
      extern void _nl_log_untranslated (const char *logfilename,
					const char *domainname,
					const char *msgid1, const char *msgid2,
					int plural);
      const char *logfilename = getenv ("GETTEXT_LOG_UNTRANSLATED");

      if (logfilename != NULL && logfilename[0] != '\0')
	_nl_log_untranslated (logfilename, domainname, msgid1, msgid2, plural);
    }
#endif
  __set_errno (saved_errno);
  return (plural == 0
	  ? (char *) msgid1
	  /* Use the Germanic plural rule.  */
	  : n == 1 ? (char *) msgid1 : (char *) msgid2);
}


/* Look up the translation of msgid within DOMAIN_FILE and DOMAINBINDING.
   Return it if found.  Return NULL if not found or in case of a conversion
   failure (problem in the particular message catalog).  Return (char *) -1
   in case of a memory allocation failure during conversion (only if
   ENCODING != NULL resp. CONVERT == true).  */
char *
internal_function
#ifdef IN_LIBGLOCALE
_nl_find_msg (struct loaded_l10nfile *domain_file,
	      struct binding *domainbinding, const char *encoding,
	      const char *msgid,
	      size_t *lengthp)
#else
_nl_find_msg (struct loaded_l10nfile *domain_file,
	      struct binding *domainbinding,
	      const char *msgid, int convert,
	      size_t *lengthp)
#endif
{
  struct loaded_domain *domain;
  nls_uint32 nstrings;
  size_t act;
  char *result;
  size_t resultlen;

  if (domain_file->decided <= 0)
    _nl_load_domain (domain_file, domainbinding);

  if (domain_file->data == NULL)
    return NULL;

  domain = (struct loaded_domain *) domain_file->data;

  nstrings = domain->nstrings;

  /* Locate the MSGID and its translation.  */
  if (domain->hash_tab != NULL)
    {
      /* Use the hashing table.  */
      nls_uint32 len = strlen (msgid);
      nls_uint32 hash_val = __hash_string (msgid);
      nls_uint32 idx = hash_val % domain->hash_size;
      nls_uint32 incr = 1 + (hash_val % (domain->hash_size - 2));

      while (1)
	{
	  nls_uint32 nstr =
	    W (domain->must_swap_hash_tab, domain->hash_tab[idx]);

	  if (nstr == 0)
	    /* Hash table entry is empty.  */
	    return NULL;

	  nstr--;

	  /* Compare msgid with the original string at index nstr.
	     We compare the lengths with >=, not ==, because plural entries
	     are represented by strings with an embedded NUL.  */
	  if (nstr < nstrings
	      ? W (domain->must_swap, domain->orig_tab[nstr].length) >= len
		&& (strcmp (msgid,
			    domain->data + W (domain->must_swap,
					      domain->orig_tab[nstr].offset))
		    == 0)
	      : domain->orig_sysdep_tab[nstr - nstrings].length > len
		&& (strcmp (msgid,
			    domain->orig_sysdep_tab[nstr - nstrings].pointer)
		    == 0))
	    {
	      act = nstr;
	      goto found;
	    }

	  if (idx >= domain->hash_size - incr)
	    idx -= domain->hash_size - incr;
	  else
	    idx += incr;
	}
      /* NOTREACHED */
    }
  else
    {
      /* Try the default method:  binary search in the sorted array of
	 messages.  */
      size_t top, bottom;

      bottom = 0;
      top = nstrings;
      while (bottom < top)
	{
	  int cmp_val;

	  act = (bottom + top) / 2;
	  cmp_val = strcmp (msgid, (domain->data
				    + W (domain->must_swap,
					 domain->orig_tab[act].offset)));
	  if (cmp_val < 0)
	    top = act;
	  else if (cmp_val > 0)
	    bottom = act + 1;
	  else
	    goto found;
	}
      /* No translation was found.  */
      return NULL;
    }

 found:
  /* The translation was found at index ACT.  If we have to convert the
     string to use a different character set, this is the time.  */
  if (act < nstrings)
    {
      result = (char *)
	(domain->data + W (domain->must_swap, domain->trans_tab[act].offset));
      resultlen = W (domain->must_swap, domain->trans_tab[act].length) + 1;
    }
  else
    {
      result = (char *) domain->trans_sysdep_tab[act - nstrings].pointer;
      resultlen = domain->trans_sysdep_tab[act - nstrings].length;
    }

#if defined _LIBC || HAVE_ICONV
# ifdef IN_LIBGLOCALE
  if (encoding != NULL)
# else
  if (convert)
# endif
    {
      /* We are supposed to do a conversion.  */
# ifndef IN_LIBGLOCALE
      const char *encoding = get_output_charset (domainbinding);
# endif
      size_t nconversions;
      struct converted_domain *convd;
      size_t i;

      /* Protect against reallocation of the table.  */
      gl_rwlock_rdlock (domain->conversions_lock);

      /* Search whether a table with converted translations for this
	 encoding has already been allocated.  */
      nconversions = domain->nconversions;
      convd = NULL;

      for (i = nconversions; i > 0; )
	{
	  i--;
	  if (strcmp (domain->conversions[i].encoding, encoding) == 0)
	    {
	      convd = &domain->conversions[i];
	      break;
	    }
	}

      gl_rwlock_unlock (domain->conversions_lock);

      if (convd == NULL)
	{
	  /* We have to allocate a new conversions table.  */
	  gl_rwlock_wrlock (domain->conversions_lock);

	  /* Maybe in the meantime somebody added the translation.
	     Recheck.  */
	  for (i = nconversions; i > 0; )
	    {
	      i--;
	      if (strcmp (domain->conversions[i].encoding, encoding) == 0)
		{
		  convd = &domain->conversions[i];
		  goto found_convd;
		}
	    }

	  {
	    /* Allocate a table for the converted translations for this
	       encoding.  */
	    struct converted_domain *new_conversions =
	      (struct converted_domain *)
	      (domain->conversions != NULL
	       ? realloc (domain->conversions,
			  (nconversions + 1) * sizeof (struct converted_domain))
	       : malloc ((nconversions + 1) * sizeof (struct converted_domain)));

	    if (__builtin_expect (new_conversions == NULL, 0))
	      {
		/* Nothing we can do, no more memory.  We cannot use the
		   translation because it might be encoded incorrectly.  */
	      unlock_fail:
		gl_rwlock_unlock (domain->conversions_lock);
		return (char *) -1;
	      }

	    domain->conversions = new_conversions;

	    /* Copy the 'encoding' string to permanent storage.  */
	    encoding = strdup (encoding);
	    if (__builtin_expect (encoding == NULL, 0))
	      /* Nothing we can do, no more memory.  We cannot use the
		 translation because it might be encoded incorrectly.  */
	      goto unlock_fail;

	    convd = &new_conversions[nconversions];
	    convd->encoding = encoding;

	    /* Find out about the character set the file is encoded with.
	       This can be found (in textual form) in the entry "".  If this
	       entry does not exist or if this does not contain the 'charset='
	       information, we will assume the charset matches the one the
	       current locale and we don't have to perform any conversion.  */
# ifdef _LIBC
	    convd->conv = (__gconv_t) -1;
# else
#  if HAVE_ICONV
	    convd->conv = (iconv_t) -1;
#  endif
# endif
	    {
	      char *nullentry;
	      size_t nullentrylen;

	      /* Get the header entry.  This is a recursion, but it doesn't
		 reallocate domain->conversions because we pass
		 encoding = NULL or convert = 0, respectively.  */
	      nullentry =
# ifdef IN_LIBGLOCALE
		_nl_find_msg (domain_file, domainbinding, NULL, "",
			      &nullentrylen);
# else
		_nl_find_msg (domain_file, domainbinding, "", 0, &nullentrylen);
# endif

	      if (nullentry != NULL)
		{
		  const char *charsetstr;

		  charsetstr = strstr (nullentry, "charset=");
		  if (charsetstr != NULL)
		    {
		      size_t len;
		      char *charset;
		      const char *outcharset;

		      charsetstr += strlen ("charset=");
		      len = strcspn (charsetstr, " \t\n");

		      charset = (char *) alloca (len + 1);
# if defined _LIBC || HAVE_MEMPCPY
		      *((char *) mempcpy (charset, charsetstr, len)) = '\0';
# else
		      memcpy (charset, charsetstr, len);
		      charset[len] = '\0';
# endif

		      outcharset = encoding;

# ifdef _LIBC
		      /* We always want to use transliteration.  */
		      outcharset = norm_add_slashes (outcharset, "TRANSLIT");
		      charset = norm_add_slashes (charset, "");
		      int r = __gconv_open (outcharset, charset, &convd->conv,
					    GCONV_AVOID_NOCONV);
		      if (__builtin_expect (r != __GCONV_OK, 0))
			{
			  /* If the output encoding is the same there is
			     nothing to do.  Otherwise do not use the
			     translation at all.  */
			  if (__builtin_expect (r != __GCONV_NULCONV, 1))
			    {
			      gl_rwlock_unlock (domain->conversions_lock);
			      free ((char *) encoding);
			      return NULL;
			    }

			  convd->conv = (__gconv_t) -1;
			}
# else
#  if HAVE_ICONV
		      /* When using GNU libc >= 2.2 or GNU libiconv >= 1.5,
			 we want to use transliteration.  */
#   if (__GLIBC__ == 2 && __GLIBC_MINOR__ >= 2) || __GLIBC__ > 2 \
       || _LIBICONV_VERSION >= 0x0105
		      if (strchr (outcharset, '/') == NULL)
			{
			  char *tmp;

			  len = strlen (outcharset);
			  tmp = (char *) alloca (len + 10 + 1);
			  memcpy (tmp, outcharset, len);
			  memcpy (tmp + len, "//TRANSLIT", 10 + 1);
			  outcharset = tmp;

			  convd->conv = iconv_open (outcharset, charset);

			  freea (outcharset);
			}
		      else
#   endif
			convd->conv = iconv_open (outcharset, charset);
#  endif
# endif

		      freea (charset);
		    }
		}
	    }
	    convd->conv_tab = NULL;
	    /* Here domain->conversions is still == new_conversions.  */
	    domain->nconversions++;
	  }

	found_convd:
	  gl_rwlock_unlock (domain->conversions_lock);
	}

      if (
# ifdef _LIBC
	  convd->conv != (__gconv_t) -1
# else
#  if HAVE_ICONV
	  convd->conv != (iconv_t) -1
#  endif
# endif
	  )
	{
	  /* We are supposed to do a conversion.  First allocate an
	     appropriate table with the same structure as the table
	     of translations in the file, where we can put the pointers
	     to the converted strings in.
	     There is a slight complication with plural entries.  They
	     are represented by consecutive NUL terminated strings.  We
	     handle this case by converting RESULTLEN bytes, including
	     NULs.  */

	  if (convd->conv_tab == NULL
	      && ((convd->conv_tab =
		    (char **) calloc (nstrings + domain->n_sysdep_strings,
				      sizeof (char *)))
		  == NULL))
	    /* Mark that we didn't succeed allocating a table.  */
	    convd->conv_tab = (char **) -1;

	  if (__builtin_expect (convd->conv_tab == (char **) -1, 0))
	    /* Nothing we can do, no more memory.  We cannot use the
	       translation because it might be encoded incorrectly.  */
	    return (char *) -1;

	  if (convd->conv_tab[act] == NULL)
	    {
	      /* We haven't used this string so far, so it is not
		 translated yet.  Do this now.  */
	      /* We use a bit more efficient memory handling.
		 We allocate always larger blocks which get used over
		 time.  This is faster than many small allocations.   */
	      __libc_lock_define_initialized (static, lock)
# define INITIAL_BLOCK_SIZE	4080
	      static unsigned char *freemem;
	      static size_t freemem_size;

	      const unsigned char *inbuf;
	      unsigned char *outbuf;
	      int malloc_count;
# ifndef _LIBC
	      transmem_block_t *transmem_list = NULL;
# endif

	      __libc_lock_lock (lock);

	      inbuf = (const unsigned char *) result;
	      outbuf = freemem + sizeof (size_t);

	      malloc_count = 0;
	      while (1)
		{
		  transmem_block_t *newmem;
# ifdef _LIBC
		  size_t non_reversible;
		  int res;

		  if (freemem_size < sizeof (size_t))
		    goto resize_freemem;

		  res = __gconv (convd->conv,
				 &inbuf, inbuf + resultlen,
				 &outbuf,
				 outbuf + freemem_size - sizeof (size_t),
				 &non_reversible);

		  if (res == __GCONV_OK || res == __GCONV_EMPTY_INPUT)
		    break;

		  if (res != __GCONV_FULL_OUTPUT)
		    {
		      /* We should not use the translation at all, it
			 is incorrectly encoded.  */
		      __libc_lock_unlock (lock);
		      return NULL;
		    }

		  inbuf = (const unsigned char *) result;
# else
#  if HAVE_ICONV
		  const char *inptr = (const char *) inbuf;
		  size_t inleft = resultlen;
		  char *outptr = (char *) outbuf;
		  size_t outleft;

		  if (freemem_size < sizeof (size_t))
		    goto resize_freemem;

		  outleft = freemem_size - sizeof (size_t);
		  if (iconv (convd->conv,
			     (ICONV_CONST char **) &inptr, &inleft,
			     &outptr, &outleft)
		      != (size_t) (-1))
		    {
		      outbuf = (unsigned char *) outptr;
		      break;
		    }
		  if (errno != E2BIG)
		    {
		      __libc_lock_unlock (lock);
		      return NULL;
		    }
#  endif
# endif

		resize_freemem:
		  /* We must allocate a new buffer or resize the old one.  */
		  if (malloc_count > 0)
		    {
		      ++malloc_count;
		      freemem_size = malloc_count * INITIAL_BLOCK_SIZE;
		      newmem = (transmem_block_t *) realloc (transmem_list,
							     freemem_size);
# ifdef _LIBC
		      if (newmem != NULL)
			transmem_list = transmem_list->next;
		      else
			{
			  struct transmem_list *old = transmem_list;

			  transmem_list = transmem_list->next;
			  free (old);
			}
# endif
		    }
		  else
		    {
		      malloc_count = 1;
		      freemem_size = INITIAL_BLOCK_SIZE;
		      newmem = (transmem_block_t *) malloc (freemem_size);
		    }
		  if (__builtin_expect (newmem == NULL, 0))
		    {
		      freemem = NULL;
		      freemem_size = 0;
		      __libc_lock_unlock (lock);
		      return (char *) -1;
		    }

# ifdef _LIBC
		  /* Add the block to the list of blocks we have to free
		     at some point.  */
		  newmem->next = transmem_list;
		  transmem_list = newmem;

		  freemem = (unsigned char *) newmem->data;
		  freemem_size -= offsetof (struct transmem_list, data);
# else
		  transmem_list = newmem;
		  freemem = newmem;
# endif

		  outbuf = freemem + sizeof (size_t);
		}

	      /* We have now in our buffer a converted string.  Put this
		 into the table of conversions.  */
	      *(size_t *) freemem = outbuf - freemem - sizeof (size_t);
	      convd->conv_tab[act] = (char *) freemem;
	      /* Shrink freemem, but keep it aligned.  */
	      freemem_size -= outbuf - freemem;
	      freemem = outbuf;
	      freemem += freemem_size & (alignof (size_t) - 1);
	      freemem_size = freemem_size & ~ (alignof (size_t) - 1);

	      __libc_lock_unlock (lock);
	    }

	  /* Now convd->conv_tab[act] contains the translation of all
	     the plural variants.  */
	  result = convd->conv_tab[act] + sizeof (size_t);
	  resultlen = *(size_t *) convd->conv_tab[act];
	}
    }

  /* The result string is converted.  */

#endif /* _LIBC || HAVE_ICONV */

  *lengthp = resultlen;
  return result;
}


/* Look up a plural variant.  */
static char *
internal_function
plural_lookup (struct loaded_l10nfile *domain, unsigned long int n,
	       const char *translation, size_t translation_len)
{
  struct loaded_domain *domaindata = (struct loaded_domain *) domain->data;
  unsigned long int index;
  const char *p;

  index = plural_eval (domaindata->plural, n);
  if (index >= domaindata->nplurals)
    /* This should never happen.  It means the plural expression and the
       given maximum value do not match.  */
    index = 0;

  /* Skip INDEX strings at TRANSLATION.  */
  p = translation;
  while (index-- > 0)
    {
#ifdef _LIBC
      p = __rawmemchr (p, '\0');
#else
      p = strchr (p, '\0');
#endif
      /* And skip over the NUL byte.  */
      p++;

      if (p >= translation + translation_len)
	/* This should never happen.  It means the plural expression
	   evaluated to a value larger than the number of variants
	   available for MSGID1.  */
	return (char *) translation;
    }
  return (char *) p;
}

#ifndef _LIBC
/* Return string representation of locale CATEGORY.  */
static const char *
internal_function
category_to_name (int category)
{
  const char *retval;

  switch (category)
  {
#ifdef LC_COLLATE
  case LC_COLLATE:
    retval = "LC_COLLATE";
    break;
#endif
#ifdef LC_CTYPE
  case LC_CTYPE:
    retval = "LC_CTYPE";
    break;
#endif
#ifdef LC_MONETARY
  case LC_MONETARY:
    retval = "LC_MONETARY";
    break;
#endif
#ifdef LC_NUMERIC
  case LC_NUMERIC:
    retval = "LC_NUMERIC";
    break;
#endif
#ifdef LC_TIME
  case LC_TIME:
    retval = "LC_TIME";
    break;
#endif
#ifdef LC_MESSAGES
  case LC_MESSAGES:
    retval = "LC_MESSAGES";
    break;
#endif
#ifdef LC_RESPONSE
  case LC_RESPONSE:
    retval = "LC_RESPONSE";
    break;
#endif
#ifdef LC_ALL
  case LC_ALL:
    /* This might not make sense but is perhaps better than any other
       value.  */
    retval = "LC_ALL";
    break;
#endif
  default:
    /* If you have a better idea for a default value let me know.  */
    retval = "LC_XXX";
  }

  return retval;
}
#endif

/* Guess value of current locale from value of the environment variables
   or system-dependent defaults.  */
static const char *
internal_function
#ifdef IN_LIBGLOCALE
guess_category_value (int category, const char *categoryname,
		      const char *locale)

#else
guess_category_value (int category, const char *categoryname)
#endif
{
  const char *language;
#ifndef IN_LIBGLOCALE
  const char *locale;
# ifndef _LIBC
  const char *language_default;
  int locale_defaulted;
# endif
#endif

  /* We use the settings in the following order:
     1. The value of the environment variable 'LANGUAGE'.  This is a GNU
        extension.  Its value can be a colon-separated list of locale names.
     2. The value of the environment variable 'LC_ALL', 'LC_xxx', or 'LANG'.
        More precisely, the first among these that is set to a non-empty value.
        This is how POSIX specifies it.  The value is a single locale name.
     3. A system-dependent preference list of languages.  Its value can be a
        colon-separated list of locale names.
     4. A system-dependent default locale name.
     This way:
       - System-dependent settings can be overridden by environment variables.
       - If the system provides both a list of languages and a default locale,
         the former is used.  */

#ifndef IN_LIBGLOCALE
  /* Fetch the locale name, through the POSIX method of looking to `LC_ALL',
     `LC_xxx', and `LANG'.  On some systems this can be done by the
     `setlocale' function itself.  */
# ifdef _LIBC
  locale = __current_locale_name (category);
# else
#  if HAVE_STRUCT___LOCALE_STRUCT___NAMES && defined USE_IN_GETTEXT_TESTS
  /* The __names field is not public glibc API and must therefore not be used
     in code that is installed in public locations.  */
  locale_t thread_locale = uselocale (NULL);
  if (thread_locale != LC_GLOBAL_LOCALE)
    {
      locale = thread_locale->__names[category];
      locale_defaulted = 0;
    }
  else
#  endif
    {
      locale = _nl_locale_name_posix (category, categoryname);
      locale_defaulted = 0;
      if (locale == NULL)
	{
	  locale = _nl_locale_name_default ();
	  locale_defaulted = 1;
	}
    }
# endif
#endif

  /* Ignore LANGUAGE and its system-dependent analogon if the locale is set
     to "C" because
     1. "C" locale usually uses the ASCII encoding, and most international
	messages use non-ASCII characters. These characters get displayed
	as question marks (if using glibc's iconv()) or as invalid 8-bit
	characters (because other iconv()s refuse to convert most non-ASCII
	characters to ASCII). In any case, the output is ugly.
     2. The precise output of some programs in the "C" locale is specified
	by POSIX and should not depend on environment variables like
	"LANGUAGE" or system-dependent information.  We allow such programs
        to use gettext().  */
  if (strcmp (locale, "C") == 0)
    return locale;

  /* The highest priority value is the value of the 'LANGUAGE' environment
     variable.  */
  language = getenv ("LANGUAGE");
  if (language != NULL && language[0] != '\0')
    return language;
#if !defined IN_LIBGLOCALE && !defined _LIBC
  /* The next priority value is the locale name, if not defaulted.  */
  if (locale_defaulted)
    {
      /* The next priority value is the default language preferences list. */
      language_default = _nl_language_preferences_default ();
      if (language_default != NULL)
        return language_default;
    }
  /* The least priority value is the locale name, if defaulted.  */
#endif
  return locale;
}

#if (defined _LIBC || HAVE_ICONV) && !defined IN_LIBGLOCALE
/* Returns the output charset.  */
static const char *
internal_function
get_output_charset (struct binding *domainbinding)
{
  /* The output charset should normally be determined by the locale.  But
     sometimes the locale is not used or not correctly set up, so we provide
     a possibility for the user to override this: the OUTPUT_CHARSET
     environment variable.  Moreover, the value specified through
     bind_textdomain_codeset overrides both.  */
  if (domainbinding != NULL && domainbinding->codeset != NULL)
    return domainbinding->codeset;
  else
    {
      /* For speed reasons, we look at the value of OUTPUT_CHARSET only
	 once.  This is a user variable that is not supposed to change
	 during a program run.  */
      static char *output_charset_cache;
      static int output_charset_cached;

      if (!output_charset_cached)
	{
	  const char *value = getenv ("OUTPUT_CHARSET");

	  if (value != NULL && value[0] != '\0')
	    {
	      size_t len = strlen (value) + 1;
	      char *value_copy = (char *) malloc (len);

	      if (value_copy != NULL)
		memcpy (value_copy, value, len);
	      output_charset_cache = value_copy;
	    }
	  output_charset_cached = 1;
	}

      if (output_charset_cache != NULL)
	return output_charset_cache;
      else
	{
# ifdef _LIBC
	  return _NL_CURRENT (LC_CTYPE, CODESET);
# else
#  if HAVE_ICONV
	  return locale_charset ();
#  endif
# endif
	}
    }
}
#endif

/* @@ begin of epilog @@ */

/* We don't want libintl.a to depend on any other library.  So we
   avoid the non-standard function stpcpy.  In GNU C Library this
   function is available, though.  Also allow the symbol HAVE_STPCPY
   to be defined.  */
#if !_LIBC && !HAVE_STPCPY
static char *
stpcpy (char *dest, const char *src)
{
  while ((*dest++ = *src++) != '\0')
    /* Do nothing. */ ;
  return dest - 1;
}
#endif

#if !_LIBC && !HAVE_MEMPCPY
static void *
mempcpy (void *dest, const void *src, size_t n)
{
  return (void *) ((char *) memcpy (dest, src, n) + n);
}
#endif

#if !_LIBC && !HAVE_TSEARCH
# include "tsearch.c"
#endif


#ifdef _LIBC
/* If we want to free all resources we have to do some work at
   program's end.  */
libc_freeres_fn (free_mem)
{
  void *old;

  while (_nl_domain_bindings != NULL)
    {
      struct binding *oldp = _nl_domain_bindings;
      _nl_domain_bindings = _nl_domain_bindings->next;
      if (oldp->dirname != _nl_default_dirname)
	/* Yes, this is a pointer comparison.  */
	free (oldp->dirname);
      free (oldp->codeset);
      free (oldp);
    }

  if (_nl_current_default_domain != _nl_default_default_domain)
    /* Yes, again a pointer comparison.  */
    free ((char *) _nl_current_default_domain);

  /* Remove the search tree with the known translations.  */
  __tdestroy (root, free);
  root = NULL;

  while (transmem_list != NULL)
    {
      old = transmem_list;
      transmem_list = transmem_list->next;
      free (old);
    }
}
#endif
