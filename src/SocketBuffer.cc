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
#include "SocketBuffer.h"

#include <cassert>
#include <algorithm>

#include "SocketCore.h"
#include "DlAbortEx.h"
#include "message.h"
#include "fmt.h"

namespace aria2 {

SocketBuffer::ByteArrayBufEntry::ByteArrayBufEntry
(unsigned char* bytes, size_t length)
  : bytes_(bytes), length_(length)
{}

SocketBuffer::ByteArrayBufEntry::~ByteArrayBufEntry()
{
  delete [] bytes_;
}

ssize_t SocketBuffer::ByteArrayBufEntry::send
(const SharedHandle<SocketCore>& socket, size_t offset)
{
  return socket->writeData(bytes_+offset, length_-offset);
}

bool SocketBuffer::ByteArrayBufEntry::final(size_t offset) const
{
  return length_ <= offset;
}

SocketBuffer::StringBufEntry::StringBufEntry(const std::string& s)
  : str_(s)
{}

SocketBuffer::StringBufEntry::StringBufEntry() {}

ssize_t SocketBuffer::StringBufEntry::send
(const SharedHandle<SocketCore>& socket, size_t offset)
{
  return socket->writeData(str_.data()+offset, str_.size()-offset);
}

bool SocketBuffer::StringBufEntry::final(size_t offset) const
{
  return str_.size() <= offset;
}

void SocketBuffer::StringBufEntry::swap(std::string& s)
{
  str_.swap(s);
}

SocketBuffer::SocketBuffer(const SharedHandle<SocketCore>& socket):
  socket_(socket), offset_(0) {}

SocketBuffer::~SocketBuffer() {}

void SocketBuffer::pushBytes(unsigned char* bytes, size_t len)
{
  if(len > 0) {
    bufq_.push_back(SharedHandle<BufEntry>(new ByteArrayBufEntry(bytes, len)));
  }
}

void SocketBuffer::pushStr(const std::string& data)
{
  if(data.size() > 0) {
    bufq_.push_back(SharedHandle<BufEntry>(new StringBufEntry(data)));
  }
}

ssize_t SocketBuffer::send()
{
  size_t totalslen = 0;
  while(!bufq_.empty()) {
    const SharedHandle<BufEntry>& buf = bufq_[0];
    ssize_t slen = buf->send(socket_, offset_);
    if(slen == 0 && !socket_->wantRead() && !socket_->wantWrite()) {
      throw DL_ABORT_EX(fmt(EX_SOCKET_SEND, "Connection closed."));
    }
    totalslen += slen;
    offset_ += slen;
    if(buf->final(offset_)) {
      bufq_.pop_front();
      offset_ = 0;
    } else {
      break;
    }
  }
  return totalslen;
}

bool SocketBuffer::sendBufferIsEmpty() const
{
  return bufq_.empty();
}

} // namespace aria2
