/* <!-- copyright */
/*
 * aria2 - The high speed download utility
 *
 * Copyright (C) 2011 Tatsuhiro Tsujikawa
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
#ifndef D_SOCKET_RECV_BUFFER_H
#define D_SOCKET_RECV_BUFFER_H

#include "common.h"
#include "SharedHandle.h"

namespace aria2 {

class SocketCore;

class SocketRecvBuffer {
public:
  SocketRecvBuffer
  (const SharedHandle<SocketCore>& socket,
   size_t capacity = 16*1024);
  ~SocketRecvBuffer();
  // Reads data from socket as much as capacity allows. Returns the
  // number of bytes read.
  ssize_t recv();
  // Shifts buffer by offset bytes. offset must satisfy offset <=
  // getBufferLength().
  void shiftBuffer(size_t offset);
  // Truncates the contents of buffer to 0.
  void clearBuffer()
  {
    bufLen_ = 0;
  }

  const SharedHandle<SocketCore>& getSocket() const
  {
    return socket_;
  }

  const unsigned char* getBuffer() const
  {
    return buf_;
  }

  size_t getBufferLength() const
  {
    return bufLen_;
  }

  bool bufferEmpty() const
  {
    return bufLen_ == 0;
  }

  void pushBuffer(const unsigned char* data, size_t len);
private:
  SharedHandle<SocketCore> socket_;
  size_t capacity_;
  unsigned char* buf_;
  size_t bufLen_;
};

} // namespace aria2

#endif // D_SOCKET_RECV_BUFFER_H
