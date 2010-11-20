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
#include "ChunkedDecodingStreamFilter.h"

#include <cassert>

#include "util.h"
#include "message.h"
#include "DlAbortEx.h"
#include "A2STR.h"

namespace aria2 {

const std::string ChunkedDecodingStreamFilter::NAME
("ChunkedDecodingStreamFilter");

size_t ChunkedDecodingStreamFilter::MAX_BUF_SIZE = 1024*1024;

ChunkedDecodingStreamFilter::ReadChunkSizeStateHandler*
ChunkedDecodingStreamFilter::readChunkSizeStateHandler_ =
  new ChunkedDecodingStreamFilter::ReadChunkSizeStateHandler();

ChunkedDecodingStreamFilter::ReadTrailerStateHandler*
ChunkedDecodingStreamFilter::readTrailerStateHandler_ =
  new ChunkedDecodingStreamFilter::ReadTrailerStateHandler();

ChunkedDecodingStreamFilter::ReadDataStateHandler*
ChunkedDecodingStreamFilter::readDataStateHandler_ =
  new ChunkedDecodingStreamFilter::ReadDataStateHandler();

ChunkedDecodingStreamFilter::ReadDataEndStateHandler*
ChunkedDecodingStreamFilter::readDataEndStateHandler_ =
  new ChunkedDecodingStreamFilter::ReadDataEndStateHandler();

ChunkedDecodingStreamFilter::StreamEndStatehandler*
ChunkedDecodingStreamFilter::streamEndStateHandler_ =
  new ChunkedDecodingStreamFilter::StreamEndStatehandler();

ChunkedDecodingStreamFilter::ChunkedDecodingStreamFilter
(const SharedHandle<StreamFilter>& delegate):
  StreamFilter(delegate),
  state_(readChunkSizeStateHandler_),
  chunkSize_(0),
  bytesProcessed_(0) {}

ChunkedDecodingStreamFilter::~ChunkedDecodingStreamFilter() {}

void ChunkedDecodingStreamFilter::init() {}

bool ChunkedDecodingStreamFilter::readChunkSize
(size_t& inbufOffset, const unsigned char* inbuf, size_t inlen)
{
  size_t pbufSize = buf_.size();
  buf_.append(&inbuf[inbufOffset], &inbuf[inlen]);
  std::string::size_type crlfPos = buf_.find(A2STR::CRLF);
  if(crlfPos == std::string::npos) {
    if(buf_.size() > MAX_BUF_SIZE) {
      throw DL_ABORT_EX("Could not find chunk size before buffer got full.");
    }
    inbufOffset = inlen;
    return false;
  }
  std::string::size_type extPos = buf_.find(A2STR::SEMICOLON_C);
  if(extPos == std::string::npos || crlfPos < extPos) {
    extPos = crlfPos;
  }
  chunkSize_ = util::parseULLInt(buf_.substr(0, extPos), 16);
  assert(crlfPos+2 > pbufSize);
  inbufOffset += crlfPos+2-pbufSize;
  buf_.clear();
  if(chunkSize_ == 0) {
    state_ = readTrailerStateHandler_;
  } else {
    state_ = readDataStateHandler_;
  }
  return true;
}

bool ChunkedDecodingStreamFilter::readTrailer
(size_t& inbufOffset, const unsigned char* inbuf, size_t inlen)
{
  size_t pbufSize = buf_.size();
  buf_.append(&inbuf[inbufOffset], &inbuf[inlen]);
  std::string::size_type crlfcrlfPos = buf_.find("\r\n\r\n");
  if(crlfcrlfPos != std::string::npos) {
    // TODO crlfcrlfPos == 0 case?
    inbufOffset += crlfcrlfPos+4-pbufSize;
    inbufOffset = inlen;
    buf_.clear();
    state_ = streamEndStateHandler_;
    return true;
  } else {
    std::string::size_type crlfPos = buf_.find(A2STR::CRLF);
    if(crlfPos == std::string::npos) {
      if(buf_.size() > MAX_BUF_SIZE) {
        throw DL_ABORT_EX
          ("Could not find end of stream before buffer got full.");
      }
      inbufOffset = inlen;
      return false;
    } else if(crlfPos == 0) {
      inbufOffset += crlfPos+2-pbufSize;
      buf_.clear();
      state_ = streamEndStateHandler_;
      return true;
    } else {
      if(buf_.size() > MAX_BUF_SIZE) {
        throw DL_ABORT_EX
          ("Could not find end of stream before buffer got full.");
      }
      inbufOffset = inlen;
      return false;
    }
  }
}

bool ChunkedDecodingStreamFilter::readData
(ssize_t& outlen,
 size_t& inbufOffset,
 const unsigned char* inbuf,
 size_t inlen,
 const SharedHandle<BinaryStream>& out,
 const SharedHandle<Segment>& segment)
{
  uint64_t readlen =
    std::min(chunkSize_, static_cast<uint64_t>(inlen-inbufOffset));
  outlen += getDelegate()->transform(out, segment, inbuf+inbufOffset, readlen);
  chunkSize_ -= readlen;
  inbufOffset += readlen;
  if(chunkSize_ == 0) {
    state_ = readDataEndStateHandler_;    
    return true;
  } else {
    return false;
  }
}

bool ChunkedDecodingStreamFilter::readDataEnd
(size_t& inbufOffset, const unsigned char* inbuf, size_t inlen)
{
  size_t pbufSize = buf_.size();
  buf_.append(&inbuf[inbufOffset], &inbuf[inlen]);
  if(buf_.size() >= 2) {
    if(util::startsWith(buf_, A2STR::CRLF)) {
      inbufOffset += 2-pbufSize;
      buf_.clear();
      state_ = readChunkSizeStateHandler_;
      return true;
    } else {
      throw DL_ABORT_EX("No CRLF at the end of chunk.");
    }
  } else {
    inbufOffset = inlen;
    return false;
  }
}

ssize_t ChunkedDecodingStreamFilter::transform
(const SharedHandle<BinaryStream>& out,
 const SharedHandle<Segment>& segment,
 const unsigned char* inbuf, size_t inlen)
{
  size_t inbufOffset = 0;
  ssize_t outlen = 0;
  while(inbufOffset < inlen) {
    ssize_t olen = 0;
    bool r = state_->execute
      (this, olen, inbufOffset, inbuf, inlen, out, segment);
    outlen += olen;
    if(!r) {
      break;
    }
  }
  bytesProcessed_ = inbufOffset;
  return outlen;
}

bool ChunkedDecodingStreamFilter::finished()
{
  return state_ == streamEndStateHandler_ && getDelegate()->finished();
}

void ChunkedDecodingStreamFilter::release() {}

const std::string& ChunkedDecodingStreamFilter::getName() const
{
  return NAME;
}

} // namespace aria2
