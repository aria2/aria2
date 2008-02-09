/* Log file output.
   Copyright (C) 2003, 2005 Free Software Foundation, Inc.

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

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Handle multi-threaded applications.  */
#ifdef _LIBC
# include <bits/libc-lock.h>
#else
# include "lock.h"
#endif

/* Print an ASCII string with quotes and escape sequences where needed.  */
static void
print_escaped (FILE *stream, const char *str)
{
  putc ('"', stream);
  for (; *str != '\0'; str++)
    if (*str == '\n')
      {
	fputs ("\\n\"", stream);
	if (str[1] == '\0')
	  return;
	fputs ("\n\"", stream);
      }
    else
      {
	if (*str == '"' || *str == '\\')
	  putc ('\\', stream);
	putc (*str, stream);
      }
  putc ('"', stream);
}

static char *last_logfilename = NULL;
static FILE *last_logfile = NULL;
__libc_lock_define_initialized (static, lock)

static inline void
_nl_log_untranslated_locked (const char *logfilename, const char *domainname,
			     const char *msgid1, const char *msgid2, int plural)
{
  FILE *logfile;

  /* Can we reuse the last opened logfile?  */
  if (last_logfilename == NULL || strcmp (logfilename, last_logfilename) != 0)
    {
      /* Close the last used logfile.  */
      if (last_logfilename != NULL)
	{
	  if (last_logfile != NULL)
	    {
	      fclose (last_logfile);
	      last_logfile = NULL;
	    }
	  free (last_logfilename);
	  last_logfilename = NULL;
	}
      /* Open the logfile.  */
      last_logfilename = (char *) malloc (strlen (logfilename) + 1);
      if (last_logfilename == NULL)
	return;
      strcpy (last_logfilename, logfilename);
      last_logfile = fopen (logfilename, "a");
      if (last_logfile == NULL)
	return;
    }
  logfile = last_logfile;

  fprintf (logfile, "domain ");
  print_escaped (logfile, domainname);
  fprintf (logfile, "\nmsgid ");
  print_escaped (logfile, msgid1);
  if (plural)
    {
      fprintf (logfile, "\nmsgid_plural ");
      print_escaped (logfile, msgid2);
      fprintf (logfile, "\nmsgstr[0] \"\"\n");
    }
  else
    fprintf (logfile, "\nmsgstr \"\"\n");
  putc ('\n', logfile);
}

/* Add to the log file an entry denoting a failed translation.  */
void
_nl_log_untranslated (const char *logfilename, const char *domainname,
		      const char *msgid1, const char *msgid2, int plural)
{
  __libc_lock_lock (lock);
  _nl_log_untranslated_locked (logfilename, domainname, msgid1, msgid2, plural);
  __libc_lock_unlock (lock);
}
