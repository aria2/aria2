/* <!-- copyright */
/*
 * aria2 - a simple utility for downloading files faster
 *
 * Copyright (C) 2006 Tatsuhiro Tsujikawa
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
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */
/* copyright --> */
#ifndef _D_SEGMENT_H_
#define _D_SEGMENT_H_

#include "common.h"
#include <deque>

using namespace std;

class Segment {
public:
  int index;
  int length;
  int segmentLength;
  int writtenLength;

  Segment(int index, int length, int segmentLength, int writtenLength = 0)
    :index(index), length(length), segmentLength(segmentLength),
     writtenLength(writtenLength) {}

  Segment():index(-1), length(0), segmentLength(0), writtenLength(0) {}

  bool complete() const {
    return length <= writtenLength;
  }

  bool isNull() const {
    return index == -1;
  }

  long long int getPosition() const {
    return ((long long int)index)*segmentLength;
  }

  bool operator==(const Segment& segment) const {
    return index == segment.index &&
      length == segment.length &&
      segmentLength == segment.segmentLength &&
      writtenLength == segment.writtenLength;
  }

  bool operator!=(const Segment& segment) const {
    return !(*this == segment);
  }

  friend ostream& operator<<(ostream& o, const Segment& segment);
};

ostream& operator<<(ostream& o, const Segment& segment);

#endif // _D_SEGMENT_H_

