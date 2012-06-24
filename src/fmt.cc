/* <!-- copyright */
/*
 * aria2 - The high speed download utility
 *
 * Copyright (C) 2010 Tatsuhiro Tsujikawa
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
#include "fmt.h"

#include <cstring>
#include <cstdio>
#include <cstdlib>

namespace aria2 {

std::string fmt(const char* fmtTemplate, ...)
{
  va_list ap;
  va_start(ap, fmtTemplate);
  char buf[2048];
  int rv;
  rv = vsnprintf(buf, sizeof(buf), fmtTemplate, ap);
#ifdef __MINGW32__
  // MINGW32 vsnprintf returns -1 if output is truncated.
  if(rv < 0 && rv != -1) {
    // Reachable?
    buf[0] = '\0';
  }
#else // !__MINGW32__
  if(rv < 0) {
    buf[0] = '\0';
  }
#endif // !__MINGW32__
  va_end(ap);
  return buf;
}

} // namespace aria2
