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
#ifndef _D_SOCKET_CORE_H_
#define _D_SOCKET_CORE_H_

#include <string>
#include <utility>
#include "common.h"

#ifdef HAVE_LIBSSL
// for SSL
# include <openssl/ssl.h>
#endif // HAVE_LIBSSL

using namespace std;

class SocketCore {
  friend class Socket;
private:
  // socket endpoint descriptor
  int sockfd;
  // reference counter for this object.
  int use;
  bool secure;
#ifdef HAVE_LIBSSL
  // for SSL
  SSL_CTX* sslCtx;
  SSL* ssl;
#endif // HAVE_LIBSSL
  void init();
  SocketCore(int sockfd);
public:
  SocketCore();
  ~SocketCore();

  void beginListen();
  void getAddrInfo(pair<string, int>& addrinfo) const;
  SocketCore* acceptConnection() const;
  /**
   * Connects to the server named host and the destination port is port.
   * This method make socket non-blocking mode.
   * To make the socket blocking mode, call setBlockingMode() after
   * the connection is established.
   */
  void establishConnection(string host, int port);

  void setBlockingMode() const;

  // Closes the connection which this socket object has
  void closeConnection();

  // examines whether the socket of this SocketCore object is available for writing.
  // returns true if the socket is available for writing, otherwise returns false.
  bool isWritable(int timeout) const;

  // examines whether the socket of this SocketCore object is available for reading.
  // returns true if the socket is available for reading, otherwise returns false.
  bool isReadable(int timeout) const;

  // writes characters into the socket. data is a pointer pointing the first
  // byte of the data and len is the length of the data.
  void writeData(const char* data, int len, int timeout = 5) const;

  // Reads up to len bytes from this socket.
  // data is a pointer pointing the first
  // byte of the data, which must be allocated before this method is called.
  // len is the size of the allocated memory. When this method returns
  // successfully, len is replaced by the size of the read data.
  void readData(char* data, int& len, int timeout = 5) const;
  // Reads up to len bytes from this socket, but bytes are not removed from
  // this socket.
  void peekData(char* data, int& len, int timeout = 5) const;
  
  /**
   * Makes this socket secure.
   * If the system has not OpenSSL, then this method do nothing.
   */
  void initiateSecureConnection() ;
};

#endif // _D_SOCKET_CORE_H_
