/*
 * Copyright (c) 2001, 02  Motoyuki Kasahara
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the project nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE PROJECT AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE PROJECT OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

#include "gai_strerror.h"

#ifdef ENABLE_NLS
#  include <libintl.h>
#endif

#ifdef ENABLE_NLS
#  define _(string) gettext(string)
#  ifdef gettext_noop
#    define N_(string) gettext_noop(string)
#  else
#    define N_(string) (string)
#  endif
#else
#  define gettext(string) (string)
#  define _(string) (string)
#  define N_(string) (string)
#endif

/*
 * Error messages for gai_strerror().
 */
static char* eai_errlist[] = {
    N_("Success"),

    /* EAI_ADDRFAMILY */
    N_("Address family for hostname not supported"),

    /* EAI_AGAIN */
    N_("Temporary failure in name resolution"),

    /* EAI_BADFLAGS */
    N_("Invalid value for ai_flags"),

    /* EAI_FAIL */
    N_("Non-recoverable failure in name resolution"),

    /* EAI_FAMILY */
    N_("ai_family not supported"),

    /* EAI_MEMORY */
    N_("Memory allocation failure"),

    /* EAI_NONAME */
    N_("hostname nor servname provided, or not known"),

    /* EAI_OVERFLOW */
    N_("An argument buffer overflowed"),

    /* EAI_SERVICE */
    N_("servname not supported for ai_socktype"),

    /* EAI_SOCKTYPE */
    N_("ai_socktype not supported"),

    /* EAI_SYSTEM */
    N_("System error returned in errno")};

/*
 * gai_strerror().
 */
const char* gai_strerror(ecode)
int ecode;
{
  if (ecode < 0 || ecode > EAI_SYSTEM)
    return _("Unknown error");

  return gettext(eai_errlist[ecode]);
}
