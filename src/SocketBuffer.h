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
#ifndef D_SOCKET_BUFFER_H
#define D_SOCKET_BUFFER_H

#include "common.h"

#include <string>
#include <deque>

#include "SharedHandle.h"

namespace aria2 {

class SocketCore;

class SocketBuffer {
private:
  class BufEntry {
  public:
    virtual ~BufEntry() {}
    virtual ssize_t send
    (const SharedHandle<SocketCore>& socket, size_t offset) = 0;
    virtual bool final(size_t offset) const = 0;
  };

  class ByteArrayBufEntry:public BufEntry {
  public:
    ByteArrayBufEntry(unsigned char* bytes, size_t length);
    virtual ~ByteArrayBufEntry();
    virtual ssize_t send
    (const SharedHandle<SocketCore>& socket, size_t offset);
    virtual bool final(size_t offset) const;
  private:
    unsigned char* bytes_;
    size_t length_;
  };

  class StringBufEntry:public BufEntry {
  public:
    StringBufEntry(const std::string& s);
    StringBufEntry();
    virtual ssize_t send
    (const SharedHandle<SocketCore>& socket, size_t offset);
    virtual bool final(size_t offset) const;
    void swap(std::string& s);
  private:
    std::string str_;
  };
    
  SharedHandle<SocketCore> socket_;

  std::deque<SharedHandle<BufEntry> > bufq_;

  // Offset of data in bufq_[0]. SocketBuffer tries to send bufq_[0],
  // but it cannot always send whole data. In this case, offset points
  // to the data to be sent in the next send() call.
  size_t offset_;
public:
  SocketBuffer(const SharedHandle<SocketCore>& socket);

  ~SocketBuffer();

  // Don't allow copying
  SocketBuffer(const SocketBuffer&);
  SocketBuffer& operator=(const SocketBuffer&);

  // Feeds data pointered by bytes with length len into queue.  This
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

#endif // D_SOCKET_BUFFER_H
