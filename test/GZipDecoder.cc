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
#include "GZipDecoder.h"
#include "fmt.h"
#include "DlAbortEx.h"

namespace aria2 {

const std::string GZipDecoder::NAME("GZipDecoder");

GZipDecoder::GZipDecoder() : strm_(nullptr), finished_(false) {}

GZipDecoder::~GZipDecoder() { release(); }

void GZipDecoder::init()
{
  finished_ = false;
  release();
  strm_ = new z_stream();
  strm_->zalloc = Z_NULL;
  strm_->zfree = Z_NULL;
  strm_->opaque = Z_NULL;
  strm_->avail_in = 0;
  strm_->next_in = Z_NULL;

  // initialize z_stream with gzip/zlib format auto detection enabled.
  if (Z_OK != inflateInit2(strm_, 47)) {
    throw DL_ABORT_EX("Initializing z_stream failed.");
  }
}

void GZipDecoder::release()
{
  if (strm_) {
    inflateEnd(strm_);
    delete strm_;
    strm_ = nullptr;
  }
}

std::string GZipDecoder::decode(const unsigned char* in, size_t length)
{
  std::string out;

  if (length == 0) {
    return out;
  }

  strm_->avail_in = length;
  strm_->next_in = const_cast<unsigned char*>(in);

  unsigned char outbuf[OUTBUF_LENGTH];
  while (1) {
    strm_->avail_out = OUTBUF_LENGTH;
    strm_->next_out = outbuf;

    int ret = ::inflate(strm_, Z_NO_FLUSH);

    if (ret == Z_STREAM_END) {
      finished_ = true;
    }
    else if (ret != Z_OK) {
      throw DL_ABORT_EX(fmt("libz::inflate() failed. cause:%s", strm_->msg));
    }

    size_t produced = OUTBUF_LENGTH - strm_->avail_out;

    out.append(&outbuf[0], &outbuf[produced]);

    if (strm_->avail_out > 0) {
      break;
    }
  }
  return out;
}

bool GZipDecoder::finished() { return finished_; }

const std::string& GZipDecoder::getName() const { return NAME; }

} // namespace aria2
