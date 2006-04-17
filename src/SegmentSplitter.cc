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
#include "SegmentSplitter.h"
#include "Util.h"
#include "LogFactory.h"

SegmentSplitter::SegmentSplitter() {
  logger = LogFactory::getInstance();
}

void SegmentSplitter::split(Segment& seg, int cuid, Segment& s) const {
  long long int nep = (s.ep-(s.sp+s.ds))/2+(s.sp+s.ds);
  seg.cuid = cuid;
  seg.sp = nep+1;
  seg.ep = s.ep;
  seg.ds = 0;
  seg.speed = s.speed;
  seg.finish = false;
  s.ep = nep;
  logger->debug(string("return new segment { "
		"sp = "+Util::llitos(seg.sp)+", "+
		"ep = "+Util::llitos(seg.ep)+", "+
		"ds = "+Util::llitos(seg.ds)+", "+
		"speed = "+Util::itos(seg.speed)+" } to "+
		"cuid "+Util::llitos(cuid)).c_str());
  logger->debug(string("update segment { "
		"sp = "+Util::llitos(s.sp)+", "+
		"ep = "+Util::llitos(s.ep)+", "+
		"ds = "+Util::llitos(s.ds)+", "+
		"speed = "+Util::itos(s.speed)+" } of "+
		"cuid "+Util::llitos(s.cuid)).c_str());
}
