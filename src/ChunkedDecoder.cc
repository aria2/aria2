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
#include "ChunkedDecoder.h"
#include "util.h"
#include "message.h"
#include "DlAbortEx.h"
#include "StringFormat.h"
#include "A2STR.h"

namespace aria2 {

const std::string ChunkedDecoder::NAME("ChunkedDecoder");

ChunkedDecoder::ChunkedDecoder():_chunkSize(0), _state(READ_SIZE) {}

ChunkedDecoder::~ChunkedDecoder() {}

void ChunkedDecoder::init() {}

static bool readChunkSize(uint64_t& chunkSize, std::string& in)
{
  std::string::size_type crlfPos = in.find(A2STR::CRLF);
  if(crlfPos == std::string::npos) {
    return false;
  }
  std::string::size_type extPos = in.find(A2STR::SEMICOLON_C);
  if(extPos == std::string::npos || crlfPos < extPos) {
    extPos = crlfPos;
  }
  chunkSize = util::parseULLInt(in.substr(0, extPos), 16);
  in.erase(0, crlfPos+2);
  return true;
}

static bool readData(std::string& out, uint64_t& chunkSize, std::string& in)
{
  uint64_t readlen = std::min(chunkSize, static_cast<uint64_t>(in.size()));
  out.append(in.begin(), in.begin()+readlen);
  in.erase(0, readlen);
  chunkSize -= readlen;
  if(chunkSize == 0 && in.size() >= 2) {
    if(in.find(A2STR::CRLF) == 0) {
      in.erase(0, 2);
      return true;
    } else {
      throw DL_ABORT_EX(EX_INVALID_CHUNK_SIZE);
    }
  } else {
    return false;
  }
}

std::string ChunkedDecoder::decode(const unsigned char* inbuf, size_t inlen)
{
  _buf.append(&inbuf[0], &inbuf[inlen]);
  
  std::string outbuf;
  while(1) {
    if(_state == READ_SIZE) {
      if(readChunkSize(_chunkSize, _buf)) {
	if(_chunkSize == 0) {
	  _state = STREAM_END;
	  break;
	} else {
	  _state = READ_DATA;
	}
      } else {
	break;
      }
    } else if(_state == READ_DATA) {
      if(readData(outbuf, _chunkSize, _buf)) {
	_state = READ_SIZE;
      } else {
	break;
      }
    }
  }
  return outbuf;
}

bool ChunkedDecoder::finished()
{
  return _state == STREAM_END;
}

void ChunkedDecoder::release() {}

const std::string& ChunkedDecoder::getName() const
{
  return NAME;
}

} // namespace aria2
