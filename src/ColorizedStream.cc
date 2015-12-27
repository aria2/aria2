/* <!-- copyright */
/*
 * aria2 - The high speed download utility
 *
 * Copyright (C) 2013 Nils Maier
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

#include "ColorizedStream.h"

namespace aria2 {
namespace colors {

const Color black("30");
const Color red("31");
const Color green("32");
const Color yellow("33");
const Color blue("34");
const Color magenta("35");
const Color cyan("36");
const Color white("37");

const Color lightred("1;31");
const Color lightgreen("1;32");
const Color lightyellow("1;33");
const Color lightblue("1;34");
const Color lightmagenta("1;35");
const Color lightcyan("1;36");
const Color lightwhite("1;37");

const Color clear("0");

} // namespace colors

std::string ColorizedStreamBuf::str(bool color) const
{
  std::stringstream rv;
  for (const auto& e : elems) {
    if (color || e.first != eColor) {
      rv << e.second;
    }
  }
  if (color) {
    rv << colors::clear.str();
  }
  return rv.str();
}

std::string ColorizedStreamBuf::str(bool color, size_t max) const
{
  std::stringstream rv;
  for (const auto& e : elems) {
    if (e.first == eColor) {
      if (color) {
        rv << e.second;
      }
      continue;
    }
    auto size = e.second.size();
    if (size > max) {
      rv.write(e.second.c_str(), max);
      break;
    }
    rv << e.second;
    max -= size;
    if (!max) {
      break;
    }
  }
  if (color) {
    rv << colors::clear.str();
  }
  return rv.str();
}

} // namespace aria2
