/* <!-- copyright */
/*
 * aria2 - a simple utility for downloading files faster
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
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */
/* copyright --> */
#ifndef _D_SOCKET_H_
#define _D_SOCKET_H_

#include <string>

using namespace std;

class Socket {
public:
  // socket endpoint descriptor
  int sockfd;
public:
  Socket();
  Socket(const Socket& s);
  ~Socket();

  Socket& operator=(const Socket& s);

  /**
   * Connects to the server named host and the destination port is port.
   * This method make socket non-blocking mode.
   * To make the socket blocking mode, call setNonBlockingMode() after
   * the connection is established.
   */
  void establishConnection(string host, int port);

  void setNonBlockingMode();

  // Closes the connection which this socket object has
  void closeConnection();

  // examines whether the socket of this Socket object is available for writing.
  // returns true if the socket is available for writing, otherwise returns false.
  bool isWritable(int timeout);

  // examines whether the socket of this Socket object is available for reading.
  // returns true if the socket is available for reading, otherwise returns false.
  bool isReadable(int timeout);

  // writes characters into the socket. data is a pointer pointing the first
  // byte of the data and len is the length of the data.
  void writeData(const char* data, int len, int timeout = 5);

  // Reads up to len bytes from this socket.
  // data is a pointer pointing the first
  // byte of the data, which must be allocated before this method is called.
  // len is the size of the allocated memory. When this method returns
  // successfully, len is replaced by the size of the read data.
  void readData(char* data, int& len, int timeout = 5);
  // Reads up to len bytes from this socket, but bytes are not removed from
  // this socket.
  void peekData(char* data, int& len, int timeout = 5);
};

#endif // _D_SOCKET_H_
