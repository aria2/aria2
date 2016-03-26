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
#ifndef D_GZIP_STREAM_FILTER_H
#define D_GZIP_STREAM_FILTER_H

#include "StreamFilter.h"
#include <zlib.h>

#include "a2functional.h"

namespace aria2 {

// GZipDecodingStreamFilter can decode both gzip and deflate format.
class GZipDecodingStreamFilter : public StreamFilter {
private:
  z_stream* strm_;

  bool finished_;

  size_t bytesProcessed_;

  static const size_t OUTBUF_LENGTH = 16_k;

public:
  GZipDecodingStreamFilter(std::unique_ptr<StreamFilter> delegate = nullptr);

  virtual ~GZipDecodingStreamFilter();

  virtual void init() CXX11_OVERRIDE;

  virtual ssize_t transform(const std::shared_ptr<BinaryStream>& out,
                            const std::shared_ptr<Segment>& segment,
                            const unsigned char* inbuf,
                            size_t inlen) CXX11_OVERRIDE;

  virtual bool finished() CXX11_OVERRIDE;

  virtual void release() CXX11_OVERRIDE;

  virtual const std::string& getName() const CXX11_OVERRIDE;

  virtual size_t getBytesProcessed() const CXX11_OVERRIDE
  {
    return bytesProcessed_;
  }

  static const std::string NAME;
};

} // namespace aria2

#endif // D_GZIP_STREAM_FILTER_H
