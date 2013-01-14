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
#include "LogFactory.h"

namespace aria2 {

SocketBuffer::ByteArrayBufEntry::ByteArrayBufEntry
(unsigned char* bytes, size_t length, ProgressUpdate* progressUpdate)
  : BufEntry(progressUpdate), bytes_(bytes), length_(length)
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

size_t SocketBuffer::ByteArrayBufEntry::getLength() const
{
  return length_;
}

const unsigned char* SocketBuffer::ByteArrayBufEntry::getData() const
{
  return bytes_;
}

SocketBuffer::StringBufEntry::StringBufEntry(const std::string& s,
                                             ProgressUpdate* progressUpdate)
  : BufEntry(progressUpdate), str_(s)
{}

// SocketBuffer::StringBufEntry::StringBufEntry() {}

ssize_t SocketBuffer::StringBufEntry::send
(const SharedHandle<SocketCore>& socket, size_t offset)
{
  return socket->writeData(str_.data()+offset, str_.size()-offset);
}

bool SocketBuffer::StringBufEntry::final(size_t offset) const
{
  return str_.size() <= offset;
}

size_t SocketBuffer::StringBufEntry::getLength() const
{
  return str_.size();
}

const unsigned char* SocketBuffer::StringBufEntry::getData() const
{
  return reinterpret_cast<const unsigned char*>(str_.c_str());
}

void SocketBuffer::StringBufEntry::swap(std::string& s)
{
  str_.swap(s);
}

SocketBuffer::SocketBuffer(const SharedHandle<SocketCore>& socket):
  socket_(socket), offset_(0) {}

SocketBuffer::~SocketBuffer() {}

void SocketBuffer::pushBytes(unsigned char* bytes, size_t len,
                             ProgressUpdate* progressUpdate)
{
  if(len > 0) {
    bufq_.push_back(SharedHandle<BufEntry>
                    (new ByteArrayBufEntry(bytes, len, progressUpdate)));
  }
}

void SocketBuffer::pushStr(const std::string& data,
                           ProgressUpdate* progressUpdate)
{
  if(data.size() > 0) {
    bufq_.push_back(SharedHandle<BufEntry>
                    (new StringBufEntry(data, progressUpdate)));
  }
}

ssize_t SocketBuffer::send()
{
  a2iovec iov[A2_IOV_MAX];
  size_t totalslen = 0;
  while(!bufq_.empty()) {
    size_t num;
    ssize_t amount = 24*1024;
    ssize_t firstlen = bufq_[0]->getLength() - offset_;
    amount -= firstlen;
    iov[0].A2IOVEC_BASE =
      reinterpret_cast<char*>(const_cast<unsigned char*>
                              (bufq_[0]->getData() + offset_));
    iov[0].A2IOVEC_LEN = firstlen;

    for(num = 1; num < A2_IOV_MAX && num < bufq_.size() && amount > 0; ++num) {
      const SharedHandle<BufEntry>& buf = bufq_[num];
      ssize_t len = buf->getLength();
      if(amount >= len) {
        amount -= len;
        iov[num].A2IOVEC_BASE =
          reinterpret_cast<char*>(const_cast<unsigned char*>(buf->getData()));
        iov[num].A2IOVEC_LEN = len;
      } else {
        break;
      }
    }
    ssize_t slen = socket_->writeVector(iov, num);
    if(slen == 0 && !socket_->wantRead() && !socket_->wantWrite()) {
      throw DL_ABORT_EX(fmt(EX_SOCKET_SEND, "Connection closed."));
    }
    // A2_LOG_NOTICE(fmt("num=%zu, amount=%d, bufq.size()=%zu, SEND=%d",
    //                   num, amount, bufq_.size(), slen));
    totalslen += slen;

    if(firstlen > slen) {
      offset_ += slen;
      bufq_[0]->progressUpdate(slen, false);
      goto fin;
    } else {
      slen -= firstlen;
      bufq_[0]->progressUpdate(firstlen, true);
      bufq_.pop_front();
      offset_ = 0;
      for(size_t i = 1; i < num; ++i) {
        const SharedHandle<BufEntry>& buf = bufq_[0];
        ssize_t len = buf->getLength();
        if(len > slen) {
          offset_ = slen;
          bufq_[0]->progressUpdate(slen, false);
          goto fin;
          break;
        } else {
          slen -= len;
          bufq_[0]->progressUpdate(len, true);
          bufq_.pop_front();
        }
      }
    }
  }
 fin:
  return totalslen;
}

bool SocketBuffer::sendBufferIsEmpty() const
{
  return bufq_.empty();
}

} // namespace aria2
