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
#include "GZipEncoder.h"

#include <cstring>

#include "StringFormat.h"
#include "DlAbortEx.h"
#include "util.h"

namespace aria2 {

namespace {
const int OUTBUF_LENGTH = 4096;
}

GZipEncoder::GZipEncoder():_strm(0), _finished(false) {}

GZipEncoder::~GZipEncoder()
{
  release();
}

void GZipEncoder::init()
{
  _finished = false;
  release();
  _strm = new z_stream();
  _strm->zalloc = Z_NULL;
  _strm->zfree = Z_NULL;
  _strm->opaque = Z_NULL;
  _strm->avail_in = 0;
  _strm->next_in = Z_NULL;

  if(Z_OK != deflateInit2(_strm, 9, Z_DEFLATED, 31, 9, Z_DEFAULT_STRATEGY)) {
    throw DL_ABORT_EX("Initializing z_stream failed.");
  }
}

void GZipEncoder::release()
{
  if(_strm) {
    deflateEnd(_strm);
    delete _strm;
    _strm = 0;
  }
}

std::string GZipEncoder::encode
(const unsigned char* in, size_t length, int flush)
{
  std::string out;

  _strm->avail_in = length;
  _strm->next_in = const_cast<unsigned char*>(in);

  unsigned char outbuf[OUTBUF_LENGTH];
  while(1) {
    _strm->avail_out = OUTBUF_LENGTH;
    _strm->next_out = outbuf;

    int ret = ::deflate(_strm, flush);

    if(ret == Z_STREAM_END) {
      _finished = true;
    } else if(ret != Z_OK) {
      throw DL_ABORT_EX(StringFormat("libz::deflate() failed. cause:%s",
                                     _strm->msg).str());
    }

    size_t produced = OUTBUF_LENGTH-_strm->avail_out;

    out.append(&outbuf[0], &outbuf[produced]);

    if(_strm->avail_out > 0) {
      break;
    }
  }
  return out;
}

bool GZipEncoder::finished()
{
  return _finished;
}

std::string GZipEncoder::str()
{
  _internalBuf += encode(0, 0, Z_FINISH);
  return _internalBuf;
}

GZipEncoder& GZipEncoder::operator<<(const char* s)
{
  _internalBuf += encode(reinterpret_cast<const unsigned char*>(s), strlen(s));
  return *this;
}

GZipEncoder& GZipEncoder::operator<<(const std::string& s)
{
  _internalBuf += encode
    (reinterpret_cast<const unsigned char*>(s.data()), s.size());
  return *this;
}

GZipEncoder& GZipEncoder::operator<<(int64_t i)
{
  std::string s = util::itos(i);
  (*this) << s;
  return *this;
}

} // namespace aria2
