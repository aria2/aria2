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

#include "SocketCore.h"
#include "DlAbortEx.h"
#include "message.h"
#include "StringFormat.h"

namespace aria2 {

SocketBuffer::SocketBuffer(const SharedHandle<SocketCore>& socket):
  _socket(socket), _offset(0) {}

SocketBuffer::~SocketBuffer() {}

void SocketBuffer::pushBytes(unsigned char* bytes, size_t len)
{
  _bufq.push_back(BufEntry::createBytes(bytes, len));
}

void SocketBuffer::pushStr(const std::string& data)
{
  _bufq.push_back(BufEntry::createStr(data));
}

ssize_t SocketBuffer::send()
{
  size_t totalslen = 0;
  while(!_bufq.empty()) {
    BufEntry& buf = _bufq[0];
    const char* data;
    ssize_t r;
    if(buf.type == TYPE_BYTES) {
      data = reinterpret_cast<const char*>(buf.bytes);
      r = buf.bytesLen-_offset;
    } else {
      const std::string& str = *buf.str;
      data = str.data();
      r = str.size()-_offset;
    }
    ssize_t slen = _socket->writeData(data+_offset, r);
    if(slen == 0 && !_socket->wantRead() && !_socket->wantWrite()) {
      throw DL_ABORT_EX(StringFormat(EX_SOCKET_SEND,
                                     "Connection closed.").str());
    }
    totalslen += slen;
    if(slen < r) {
      _offset += slen;
      break;
    } else {
      _offset = 0;
      buf.deleteBuf();
      _bufq.pop_front();
    }
  }
  return totalslen;
}

bool SocketBuffer::sendBufferIsEmpty() const
{
  return _bufq.empty();
}

} // namespace aria2
