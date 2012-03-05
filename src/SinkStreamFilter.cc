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
#include "SinkStreamFilter.h"
#include "BinaryStream.h"
#include "Segment.h"

namespace aria2 {

const std::string SinkStreamFilter::NAME("SinkStreamFilter");

SinkStreamFilter::SinkStreamFilter(bool hashUpdate):
  hashUpdate_(hashUpdate),
  bytesProcessed_(0) {}

ssize_t SinkStreamFilter::transform
(const SharedHandle<BinaryStream>& out,
 const SharedHandle<Segment>& segment,
 const unsigned char* inbuf, size_t inlen)
{
  size_t wlen;
  if(inlen > 0) {
    if(segment->getLength() > 0) {
      // We must not write data larger than available space in
      // segment.
      assert(segment->getLength() >= segment->getWrittenLength());
      size_t lenAvail = segment->getLength()-segment->getWrittenLength();
      wlen = std::min(inlen, lenAvail);
    } else {
      wlen = inlen;
    }
    out->writeData(inbuf, wlen, segment->getPositionToWrite());
#ifdef ENABLE_MESSAGE_DIGEST
    if(hashUpdate_) {
      segment->updateHash(segment->getWrittenLength(), inbuf, wlen);
    }
#endif // ENABLE_MESSAGE_DIGEST
    segment->updateWrittenLength(wlen);
  } else {
    wlen = 0;
  }
  bytesProcessed_ = wlen;
  return bytesProcessed_;
}

} // namespace aria2
