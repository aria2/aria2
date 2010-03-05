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
#ifndef _D_SOCKET_BUFFER_H_
#define _D_SOCKET_BUFFER_H_

#include "common.h"

#include <string>
#include <deque>

#include "SharedHandle.h"

namespace aria2 {

class SocketCore;

class SocketBuffer {
private:
  enum BUF_TYPE {
    TYPE_BYTES,
    TYPE_STR
  };
  struct BufEntry {
    BUF_TYPE type;
    unsigned char* bytes;
    size_t bytesLen;
    std::string* str;

    void deleteBuf()
    {
      if(type == TYPE_BYTES) {
        delete [] bytes;
      } else if(type == TYPE_STR) {
        delete str;
      }
    }
      
    BufEntry(unsigned char* bytes, size_t len):
      type(TYPE_BYTES), bytes(bytes), bytesLen(len) {}

    BufEntry(const std::string& str):
      type(TYPE_STR), str(new std::string(str)) {}
  };
    
  SharedHandle<SocketCore> _socket;

  std::deque<BufEntry> _bufq;

  // Offset of data in _bufq[0]. SocketBuffer tries to send _bufq[0],
  // but it cannot always send whole data. In this case, offset points
  // to the data to be sent in the next send() call.
  size_t _offset;
public:
  SocketBuffer(const SharedHandle<SocketCore>& socket);

  ~SocketBuffer();

  // Feeds data pointered by bytes with length len. into queue.  This
  // object gets ownership of bytes, so caller must not delete or
  // later bytes after this call. This function doesn't send data.
  void pushBytes(unsigned char* bytes, size_t len);

  // Feeds data into queue. This function doesn't send data.
  void pushStr(const std::string& data);

  // Sends data in queue.  Returns the number of bytes sent.
  ssize_t send();

  // Returns true if queue is empty.
  bool sendBufferIsEmpty() const;

};

} // namespace aria2

#endif // _D_SOCKET_BUFFER_H_
