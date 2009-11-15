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
#include "SingleFileAllocationIterator.h"
#include "BinaryStream.h"
#include "util.h"
#include "a2io.h"
#include <cstring>
#include <cstdlib>

namespace aria2 {

#define BUFSIZE (256*1024)
#define ALIGNMENT 512

SingleFileAllocationIterator::SingleFileAllocationIterator(BinaryStream* stream, off_t offset, uint64_t totalLength):_stream(stream), _offset(offset), _totalLength(totalLength), _buffer(0)
{
  if(_offset%ALIGNMENT != 0) {
    _stream->disableDirectIO();
  }
}

SingleFileAllocationIterator::~SingleFileAllocationIterator()
{
#ifdef HAVE_POSIX_MEMALIGN
  free(_buffer);
#else
  delete [] _buffer;
#endif // HAVE_POSIX_MEMALIGN
}

void SingleFileAllocationIterator::init()
{
#ifdef HAVE_POSIX_MEMALIGN
  _buffer = reinterpret_cast<unsigned char*>
    (util::allocateAlignedMemory(ALIGNMENT, BUFSIZE));
#else
  _buffer = new unsigned char[BUFSIZE];
#endif // HAVE_POSIX_MEMALIGN
  memset(_buffer, 0, BUFSIZE);
}

void SingleFileAllocationIterator::allocateChunk()
{
  _stream->writeData(_buffer, BUFSIZE, _offset);
  _offset += BUFSIZE;

  if(_totalLength < (uint64_t)_offset) {
    _stream->truncate(_totalLength);
    _offset = _totalLength;
  }
}

bool SingleFileAllocationIterator::finished()
{
  return (uint64_t)_offset >= _totalLength;
}

} // namespace aria2
