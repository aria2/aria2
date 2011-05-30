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

#include "fmt.h"
#include "DlAbortEx.h"
#include "util.h"

namespace aria2 {

namespace {
const int OUTBUF_LENGTH = 4096;
} // namespace

GZipEncoder::GZipEncoder():strm_(0) {}

GZipEncoder::~GZipEncoder()
{
  release();
}

void GZipEncoder::init()
{
  release();
  strm_ = new z_stream();
  strm_->zalloc = Z_NULL;
  strm_->zfree = Z_NULL;
  strm_->opaque = Z_NULL;
  strm_->avail_in = 0;
  strm_->next_in = Z_NULL;

  if(Z_OK != deflateInit2
     (strm_, Z_DEFAULT_COMPRESSION, Z_DEFLATED, 31, 9, Z_DEFAULT_STRATEGY)) {
    throw DL_ABORT_EX("Initializing z_stream failed.");
  }
}

void GZipEncoder::release()
{
  if(strm_) {
    deflateEnd(strm_);
    delete strm_;
    strm_ = 0;
  }
}

std::string GZipEncoder::encode
(const unsigned char* in, size_t length, int flush)
{
  strm_->avail_in = length;
  strm_->next_in = const_cast<unsigned char*>(in);
  std::string out;
  unsigned char outbuf[OUTBUF_LENGTH];
  while(1) {
    strm_->avail_out = OUTBUF_LENGTH;
    strm_->next_out = outbuf;
    int ret = ::deflate(strm_, flush);
    if(ret == Z_STREAM_ERROR) {
      throw DL_ABORT_EX(fmt("libz::deflate() failed. cause:%s",
                            strm_->msg));
    }
    size_t produced = OUTBUF_LENGTH-strm_->avail_out;
    out.append(&outbuf[0], &outbuf[produced]);
    if(strm_->avail_out > 0) {
      break;
    }
  }
  return out;
}

std::string GZipEncoder::str()
{
  internalBuf_ += encode(0, 0, Z_FINISH);
  return internalBuf_;
}

GZipEncoder& GZipEncoder::operator<<(const char* s)
{
  internalBuf_ += encode(reinterpret_cast<const unsigned char*>(s), strlen(s));
  return *this;
}

GZipEncoder& GZipEncoder::operator<<(const std::string& s)
{
  internalBuf_ += encode
    (reinterpret_cast<const unsigned char*>(s.data()), s.size());
  return *this;
}

GZipEncoder& GZipEncoder::operator<<(int64_t i)
{
  std::string s = util::itos(i);
  (*this) << s;
  return *this;
}

GZipEncoder& GZipEncoder::write(const char* s, size_t length)
{
  internalBuf_ += encode(reinterpret_cast<const unsigned char*>(s), length);
  return *this;
}

} // namespace aria2
