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
#include "GZipDecodingStreamFilter.h"

#include <cassert>

#include "fmt.h"
#include "DlAbortEx.h"

namespace aria2 {

const std::string GZipDecodingStreamFilter::NAME("GZipDecodingStreamFilter");

GZipDecodingStreamFilter::GZipDecodingStreamFilter(
    std::unique_ptr<StreamFilter> delegate)
    : StreamFilter{std::move(delegate)},
      strm_{nullptr},
      finished_{false},
      bytesProcessed_{0}
{
}

GZipDecodingStreamFilter::~GZipDecodingStreamFilter() { release(); }

void GZipDecodingStreamFilter::init()
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

void GZipDecodingStreamFilter::release()
{
  if (strm_) {
    inflateEnd(strm_);
    delete strm_;
    strm_ = nullptr;
  }
}

ssize_t
GZipDecodingStreamFilter::transform(const std::shared_ptr<BinaryStream>& out,
                                    const std::shared_ptr<Segment>& segment,
                                    const unsigned char* inbuf, size_t inlen)
{
  bytesProcessed_ = 0;
  ssize_t outlen = 0;
  if (inlen == 0) {
    return outlen;
  }

  strm_->avail_in = inlen;
  strm_->next_in = const_cast<unsigned char*>(inbuf);

  unsigned char outbuf[OUTBUF_LENGTH];
  while (1) {
    strm_->avail_out = OUTBUF_LENGTH;
    strm_->next_out = outbuf;

    int ret = ::inflate(strm_, Z_NO_FLUSH);

    if (ret == Z_STREAM_END) {
      finished_ = true;
    }
    else if (ret != Z_OK && ret != Z_BUF_ERROR) {
      throw DL_ABORT_EX(fmt("libz::inflate() failed. cause:%s", strm_->msg));
    }

    size_t produced = OUTBUF_LENGTH - strm_->avail_out;

    outlen += getDelegate()->transform(out, segment, outbuf, produced);
    if (strm_->avail_out > 0) {
      break;
    }
  }
  assert(inlen >= strm_->avail_in);
  bytesProcessed_ = inlen - strm_->avail_in;
  return outlen;
}

bool GZipDecodingStreamFilter::finished()
{
  return finished_ && getDelegate()->finished();
}

const std::string& GZipDecodingStreamFilter::getName() const { return NAME; }

} // namespace aria2
