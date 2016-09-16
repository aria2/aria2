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
#include "Range.h"

namespace aria2 {

Range::Range() : startByte(0), endByte(0), entityLength(0) {}

Range::Range(int64_t startByte, int64_t endByte, int64_t entityLength)
    : startByte(startByte), endByte(endByte), entityLength(entityLength)
{
}

Range::Range(const Range& c) = default;

Range::~Range() = default;

Range& Range::operator=(const Range& c)
{
  if (this != &c) {
    startByte = c.startByte;
    endByte = c.endByte;
    entityLength = c.entityLength;
  }
  return *this;
}

bool Range::operator==(const Range& range) const
{
  return startByte == range.startByte && endByte == range.endByte &&
         entityLength == range.entityLength;
}

bool Range::operator!=(const Range& range) const { return !(*this == range); }

int64_t Range::getContentLength() const
{
  if (endByte >= startByte) {
    return endByte - startByte + 1;
  }
  else {
    return 0;
  }
}

} // namespace aria2
