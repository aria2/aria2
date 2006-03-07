/* OS/2 compatibility defines.
   This file is intended to be included from config.h
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
   Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307,
   USA.  */

/* When included from os2compat.h we need all the original definitions */
#ifndef OS2_AWARE

#undef LIBDIR
#define LIBDIR			_nlos2_libdir
extern char *_nlos2_libdir;

#undef LOCALEDIR
#define LOCALEDIR		_nlos2_localedir
extern char *_nlos2_localedir;

#undef LOCALE_ALIAS_PATH
#define LOCALE_ALIAS_PATH	_nlos2_localealiaspath
extern char *_nlos2_localealiaspath;

#endif

#undef HAVE_STRCASECMP
#define HAVE_STRCASECMP 1
#define strcasecmp stricmp
#define strncasecmp strnicmp

/* We have our own getenv() which works even if library is compiled as DLL */
#define getenv _nl_getenv

/* Older versions of gettext used -1 as the value of LC_MESSAGES */
#define LC_MESSAGES_COMPAT (-1)
