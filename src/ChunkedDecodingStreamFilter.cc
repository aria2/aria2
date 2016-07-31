/* <!-- copyright */
/*
 * aria2 - The high speed download utility
 *
 * Copyright (C) 2012 Tatsuhiro Tsujikawa
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

const std::string
    ChunkedDecodingStreamFilter::NAME("ChunkedDecodingStreamFilter");

namespace {
enum {
  PREV_CHUNK_SIZE,
  CHUNK_SIZE,
  CHUNK_EXTENSION,
  PREV_CHUNK_SIZE_LF,
  CHUNK,
  PREV_CHUNK_CR,
  PREV_CHUNK_LF,
  PREV_TRAILER,
  TRAILER,
  PREV_TRAILER_LF,
  PREV_END_CR,
  PREV_END_LF,
  CHUNKS_COMPLETE
};
} // namespace

ChunkedDecodingStreamFilter::ChunkedDecodingStreamFilter(
    std::unique_ptr<StreamFilter> delegate)
    : StreamFilter{std::move(delegate)},
      state_{PREV_CHUNK_SIZE},
      chunkSize_{0},
      chunkRemaining_{0},
      bytesProcessed_{0}
{
}

ChunkedDecodingStreamFilter::~ChunkedDecodingStreamFilter() = default;

void ChunkedDecodingStreamFilter::init() {}

ssize_t
ChunkedDecodingStreamFilter::transform(const std::shared_ptr<BinaryStream>& out,
                                       const std::shared_ptr<Segment>& segment,
                                       const unsigned char* inbuf, size_t inlen)
{
  ssize_t outlen = 0;
  size_t i;
  bytesProcessed_ = 0;
  for (i = 0; i < inlen; ++i) {
    unsigned char c = inbuf[i];
    switch (state_) {
    case PREV_CHUNK_SIZE:
      if (util::isHexDigit(c)) {
        chunkSize_ = util::hexCharToUInt(c);
        state_ = CHUNK_SIZE;
      }
      else {
        throw DL_ABORT_EX("Bad chunk size: not hex string");
      }
      break;
    case CHUNK_SIZE:
      if (util::isHexDigit(c)) {
        if (chunkSize_ & 0x7800000000000000LL) {
          throw DL_ABORT_EX("Too big chunk size");
        }
        chunkSize_ <<= 4;
        chunkSize_ += util::hexCharToUInt(c);
      }
      else if (c == ';') {
        state_ = CHUNK_EXTENSION;
      }
      else if (c == '\r') {
        state_ = PREV_CHUNK_SIZE_LF;
      }
      else {
        throw DL_ABORT_EX("Bad chunk size: not hex string");
      }
      break;
    case CHUNK_EXTENSION:
      if (c == '\r') {
        state_ = PREV_CHUNK_SIZE_LF;
      }
      break;
    case PREV_CHUNK_SIZE_LF:
      if (c == '\n') {
        chunkRemaining_ = chunkSize_;
        if (chunkSize_ == 0) {
          state_ = PREV_TRAILER;
        }
        else {
          state_ = CHUNK;
        }
      }
      else {
        throw DL_ABORT_EX("Bad chunk encoding: "
                          "missing LF at the end of chunk size");
      }
      break;
    case CHUNK: {
      int64_t readlen =
          std::min(chunkRemaining_, static_cast<int64_t>(inlen - i));
      outlen += getDelegate()->transform(out, segment, inbuf + i, readlen);
      chunkRemaining_ -= readlen;
      i += readlen - 1;
      if (chunkRemaining_ == 0) {
        state_ = PREV_CHUNK_CR;
      }
      break;
    }
    case PREV_CHUNK_CR:
      if (c == '\r') {
        state_ = PREV_CHUNK_LF;
      }
      else {
        throw DL_ABORT_EX("Bad chunk encoding: "
                          "missing CR at the end of chunk");
      }
      break;
    case PREV_CHUNK_LF:
      if (c == '\n') {
        if (chunkSize_ == 0) {
          state_ = PREV_TRAILER;
        }
        else {
          chunkSize_ = chunkRemaining_ = 0;
          state_ = PREV_CHUNK_SIZE;
        }
      }
      else {
        throw DL_ABORT_EX("Bad chunk encoding: "
                          "missing LF at the end of chunk");
      }
      break;
    case PREV_TRAILER:
      if (c == '\r') {
        // No trailer
        state_ = PREV_END_LF;
      }
      else {
        state_ = TRAILER;
      }
      break;
    case TRAILER:
      if (c == '\r') {
        state_ = PREV_TRAILER_LF;
      }
      break;
    case PREV_TRAILER_LF:
      if (c == '\n') {
        state_ = PREV_TRAILER;
      }
      else {
        throw DL_ABORT_EX("Bad chunk encoding: "
                          "missing LF at the end of trailer");
      }
      break;
    case PREV_END_LF:
      if (c == '\n') {
        state_ = CHUNKS_COMPLETE;
      }
      else {
        throw DL_ABORT_EX("Bad chunk encoding: "
                          "missing LF at the end of chunks");
      }
      break;
    case CHUNKS_COMPLETE:
      goto fin;
    default:
      // unreachable
      assert(0);
    }
  }
fin:
  bytesProcessed_ += i;
  return outlen;
}

bool ChunkedDecodingStreamFilter::finished()
{
  return state_ == CHUNKS_COMPLETE && getDelegate()->finished();
}

void ChunkedDecodingStreamFilter::release() {}

const std::string& ChunkedDecodingStreamFilter::getName() const { return NAME; }

} // namespace aria2
