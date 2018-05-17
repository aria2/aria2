/* <!-- copyright */
/*
 * aria2 - The high speed download utility
 *
 * Copyright (C) 2014 Nils Maier
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 *
 * In addition, as a special exception, the copyright holders give
 * permission to link the code of portions of this program with the
 * OpenSSL library under certain conditions as described in each
 * individual source file, and distribute linked combinations
 * including the two.
 * You must obey the GNU General Public License in all respects
 * for all of the code used other than OpenSSL.  If you modify
 * file(s) with this exception, you may extend this exception to your
 * version of the file(s), but you are not obligated to do so.  If you
 * do not wish to do so, delete this exception statement from your
 * version.  If you delete this exception statement from all source
 * files in the program, then also delete it here.
 */
/* copyright --> */

#define _GNUSOURCE
#include <unistd.h>
#include <sys/syscall.h>
#include <linux/random.h>
#include <sys/random.h>
#include <errno.h>
#include <linux/errno.h>
#include <stdint.h>
#include <stdio.h>

#include "config.h"
#include "getrandom_linux.h"

int getrandom_linux(void* buf, size_t buflen)
{
  int rv = 0;
  uint8_t* p = buf;

  /* Loop while we did not fully retrieve what the user asked for.
   * This may happen in particular when a call was EINTRupted.
   */
  while (buflen) {
    int read;
#ifdef HAVE_GETRANDOM
    /* libc already has support */
    read = getrandom(p, buflen, 0);
#else  // HAVE_GETRANDOM
    /* libc has no support, make the syscall ourselves */
    read = syscall(SYS_getrandom, p, buflen, 0);
    /* Some libc impl. might mess -ERESTART up */
    if (read == -EINTR || read == -ERESTART) {
      /* ERESTART, like EINTR, should restart the call, later, so handle both
       * the same way.
       */
      errno = EINTR;
      read = -1;
    }
    /* Some other non-interrupted error happened, put error code into errno and
     * switch read to -1 (return value).
     */
    if (read < -1) {
      errno = -read;
      read = -1;
    }
#endif // HAVE_GETRANDOM
    if (read < 0) {
      if (errno == EINTR) {
        /* Restart call */
        continue;
      }
      /* Call failed, return -1, errno should be set up correctly at this
       * point.
       */
      return -1;
    }
    /* We got some more randomness */
    p += read;
    rv += read;
    buflen -= read;
  }

  return rv;
}
