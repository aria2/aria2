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
#include "SplitSlowestSegmentSplitter.h"

bool SplitSlowestSegmentSplitter::splitSegment(Segment& seg, int cuid, Segments& segments) {
  Segments::iterator slowest = segments.end();;
  for(Segments::iterator itr = segments.begin(); itr != segments.end(); itr++) {
    Segment& s = *itr;
    if(s.finish) {
      continue;
    }
    if(s.ep-(s.sp+s.ds) <= minSegmentSize) {
      continue;
    }
    if(slowest == segments.end()) {
      slowest = itr;
    } else {
      Segment sl = *slowest;
      if((sl.ep-(sl.sp+sl.ds))/(sl.speed+1) < (s.ep-(s.sp+s.ds))/(s.speed+1)) {
	slowest = itr;
      }
    }
  }
  if(slowest == segments.end()) {
    return false;
  } else {
    split(seg, cuid, *slowest);
    segments.push_back(seg);
    return true;
  }
}
