/* <!-- copyright */
/*
 * aria2 - The high speed download utility
 *
 * Copyright (C) 2009 Tatsuhiro Tsujikawa
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
#include "daemon.h"

#include <unistd.h>
#include <cstdio>
#include <cstdlib>

namespace aria2 {

int daemon(int nochdir, int noclose)
{
#ifdef HAVE_WORKING_FORK
  pid_t pid;
  pid = fork();
  if (pid == -1) {
    return -1;
  }
  else if (pid > 0) {
    _exit(EXIT_SUCCESS);
  }
  if (setsid() == -1) {
    return -1;
  }
  pid = fork();
  if (pid == -1) {
    return -1;
  }
  else if (pid > 0) {
    _exit(EXIT_SUCCESS);
  }
  if (nochdir == 0) {
    if (chdir("/") == -1) {
      return -1;
    }
  }
  if (noclose == 0) {
    if (freopen("/dev/null", "r", stdin) == nullptr) {
      return -1;
    }
    if (freopen("/dev/null", "w", stdout) == nullptr) {
      return -1;
    }
    if (freopen("/dev/null", "w", stderr) == nullptr) {
      return -1;
    }
  }
#endif // HAVE_WORKING_FORK
  return 0;
}

} // namespace aria2
