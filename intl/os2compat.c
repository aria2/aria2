/* OS/2 compatibility functions.
   Copyright (C) 2001-2002 Free Software Foundation, Inc.

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

#define OS2_AWARE
#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <stdlib.h>
#include <string.h>
#include <sys/param.h>

/* A version of getenv() that works from DLLs */
extern unsigned long DosScanEnv (const unsigned char *pszName, unsigned char **ppszValue);

char *
_nl_getenv (const char *name)
{
  unsigned char *value;
  if (DosScanEnv (name, &value))
    return NULL;
  else
    return value;
}

/* A fixed size buffer.  */
char libintl_nl_default_dirname[MAXPATHLEN+1];

char *_nlos2_libdir = NULL;
char *_nlos2_localealiaspath = NULL;
char *_nlos2_localedir = NULL;

static __attribute__((constructor)) void
nlos2_initialize ()
{
  char *root = getenv ("UNIXROOT");
  char *gnulocaledir = getenv ("GNULOCALEDIR");

  _nlos2_libdir = gnulocaledir;
  if (!_nlos2_libdir)
    {
      if (root)
        {
          size_t sl = strlen (root);
          _nlos2_libdir = (char *) malloc (sl + strlen (LIBDIR) + 1);
          memcpy (_nlos2_libdir, root, sl);
          memcpy (_nlos2_libdir + sl, LIBDIR, strlen (LIBDIR) + 1);
        }
      else
        _nlos2_libdir = LIBDIR;
    }

  _nlos2_localealiaspath = gnulocaledir;
  if (!_nlos2_localealiaspath)
    {
      if (root)
        {
          size_t sl = strlen (root);
          _nlos2_localealiaspath = (char *) malloc (sl + strlen (LOCALE_ALIAS_PATH) + 1);
          memcpy (_nlos2_localealiaspath, root, sl);
          memcpy (_nlos2_localealiaspath + sl, LOCALE_ALIAS_PATH, strlen (LOCALE_ALIAS_PATH) + 1);
        }
     else
        _nlos2_localealiaspath = LOCALE_ALIAS_PATH;
    }

  _nlos2_localedir = gnulocaledir;
  if (!_nlos2_localedir)
    {
      if (root)
        {
          size_t sl = strlen (root);
          _nlos2_localedir = (char *) malloc (sl + strlen (LOCALEDIR) + 1);
          memcpy (_nlos2_localedir, root, sl);
          memcpy (_nlos2_localedir + sl, LOCALEDIR, strlen (LOCALEDIR) + 1);
        }
      else
        _nlos2_localedir = LOCALEDIR;
    }

  if (strlen (_nlos2_localedir) <= MAXPATHLEN)
    strcpy (libintl_nl_default_dirname, _nlos2_localedir);
}
