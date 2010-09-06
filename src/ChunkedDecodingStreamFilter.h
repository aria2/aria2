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
#ifndef D_CHUNKED_DECODING_STREAM_FILTER_H
#define D_CHUNKED_DECODING_STREAM_FILTER_H

#include "StreamFilter.h"

namespace aria2 {

class ChunkedDecodingStreamFilter : public StreamFilter {
private:
  class StateHandler {
  public:
    virtual ~StateHandler() {}

    virtual bool execute
    (ChunkedDecodingStreamFilter* filter,
     ssize_t& outlen,
     size_t& inbufOffset,
     const unsigned char* inbuf,
     size_t inlen,
     const SharedHandle<BinaryStream>& out,
     const SharedHandle<Segment>& segment) const = 0;
  };

  StateHandler* state_;

  std::string buf_;

  uint64_t chunkSize_;

  size_t bytesProcessed_;

  static size_t MAX_BUF_SIZE;

  bool readChunkSize
  (size_t& inbufOffset, const unsigned char* inbuf, size_t inlen);

  bool readTrailer
  (size_t& inbufOffset, const unsigned char* inbuf, size_t inlen);

  bool readData
  (ssize_t& outlen,
   size_t& inbufOffset,
   const unsigned char* inbuf,
   size_t inlen,
   const SharedHandle<BinaryStream>& out,
   const SharedHandle<Segment>& segment);

  bool readDataEnd
  (size_t& inbufOffset, const unsigned char* inbuf, size_t inlen);

  class ReadChunkSizeStateHandler:public StateHandler {
  public:
    virtual bool execute
    (ChunkedDecodingStreamFilter* filter,
     ssize_t& outlen,
     size_t& inbufOffset,
     const unsigned char* inbuf,
     size_t inlen,
     const SharedHandle<BinaryStream>& out,
     const SharedHandle<Segment>& segment) const
    {
      return filter->readChunkSize(inbufOffset, inbuf, inlen);
    }
  };

  class ReadTrailerStateHandler:public StateHandler {
  public:
    virtual bool execute
    (ChunkedDecodingStreamFilter* filter,
     ssize_t& outlen,
     size_t& inbufOffset,
     const unsigned char* inbuf,
     size_t inlen,
     const SharedHandle<BinaryStream>& out,
     const SharedHandle<Segment>& segment) const
    {
      return filter->readTrailer(inbufOffset, inbuf, inlen);
    }
  };

  class ReadDataStateHandler:public StateHandler {
  public:
    virtual bool execute
    (ChunkedDecodingStreamFilter* filter,
     ssize_t& outlen,
     size_t& inbufOffset,
     const unsigned char* inbuf,
     size_t inlen,
     const SharedHandle<BinaryStream>& out,
     const SharedHandle<Segment>& segment) const
    {
      return filter->readData(outlen, inbufOffset, inbuf, inlen, out, segment);
    }
  };

  class ReadDataEndStateHandler:public StateHandler {
  public:
    virtual bool execute
    (ChunkedDecodingStreamFilter* filter,
     ssize_t& outlen,
     size_t& inbufOffset,
     const unsigned char* inbuf,
     size_t inlen,
     const SharedHandle<BinaryStream>& out,
     const SharedHandle<Segment>& segment) const
    {
      return filter->readDataEnd(inbufOffset, inbuf, inlen);
    }
  };

  class StreamEndStatehandler:public StateHandler {
  public:
    virtual bool execute
    (ChunkedDecodingStreamFilter* filter,
     ssize_t& outlen,
     size_t& inbufOffset,
     const unsigned char* inbuf,
     size_t inlen,
     const SharedHandle<BinaryStream>& out,
     const SharedHandle<Segment>& segment) const
    {
      return false;
    }
  };

  static ReadChunkSizeStateHandler* readChunkSizeStateHandler_;
  static ReadTrailerStateHandler* readTrailerStateHandler_;
  static ReadDataStateHandler* readDataStateHandler_;
  static ReadDataEndStateHandler* readDataEndStateHandler_;
  static StreamEndStatehandler* streamEndStateHandler_;
public:
  ChunkedDecodingStreamFilter
  (const SharedHandle<StreamFilter>& delegate = SharedHandle<StreamFilter>());

  virtual ~ChunkedDecodingStreamFilter();

  virtual void init();

  virtual ssize_t transform
  (const SharedHandle<BinaryStream>& out,
   const SharedHandle<Segment>& segment,
   const unsigned char* inbuf, size_t inlen);
  
  virtual bool finished();

  virtual void release();

  virtual const std::string& getName() const;

  virtual size_t getBytesProcessed() const
  {
    return bytesProcessed_;
  }

  static const std::string NAME;
};

} // namespace aria2

#endif // D_CHUNKED_DECODING_STREAM_FILTER_H
