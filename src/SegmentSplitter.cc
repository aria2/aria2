/* <!-- copyright */
/*
 * aria2 - The high speed download utility
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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
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
