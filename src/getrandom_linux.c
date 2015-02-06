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
#include <errno.h>
#include <linux/errno.h>
#include <stdint.h>
#include <stdio.h>

#include "config.h"
#include "getrandom_linux.h"

int getrandom_linux(void *buf, size_t buflen) {
  int rv = 0;
  uint8_t* p = buf;
  while (buflen) {
    int read;
#ifdef HAVE_GETRANDOM
    read = getrandom(p, buflen, 0);
#else // HAVE_GETRANDOM
    read = syscall(SYS_getrandom, p, buflen, 0);
    /* Some libc impl. might mess this up */
    if (read == -EINTR || read == -ERESTART) {
      errno = EINTR;
      read = -1;
    }
    if (read < -1) {
      errno = -read;
      read = -1;
    }
#endif // HAVE_GETRANDOM
    if (read < 0) {
      if (errno == EINTR) {
        continue;
      }
      return -1;
    }
    p += read;
    rv += read;
    buflen -= read;
  }
  return rv;
}
