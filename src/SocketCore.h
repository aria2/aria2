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
#ifdef HAVE_LIBGNUTLS
# include <gnutls/gnutls.h>
#endif // HAVE_LIBGNUTLS

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
#ifdef HAVE_LIBGNUTLS
  gnutls_session_t sslSession;
  gnutls_certificate_credentials_t sslXcred;
  char* peekBuf;
  int peekBufLength;
  int peekBufMax;

  int shiftPeekData(char* data, int len);
  void addPeekData(char* data, int len);
  int gnutlsRecv(char* data, int len);
  int gnutlsPeek(char* data, int len);
#endif // HAVE_LIBGNUTLS

  void init();
  SocketCore(int sockfd);
public:
  SocketCore();
  ~SocketCore();

  /**
   * Creates a socket and listens form connection on it.
   * @param port port to listen. If 0 is specified, os automaticaly
   * choose avaiable port.
   */
  void beginListen(int port = 0);

  /**
   * Stores host address and port of this socket to addrinfo.
   * @param addrinfo placeholder to store host address and port.
   */
  void getAddrInfo(pair<string, int>& addrinfo) const;
  
  /**
   * Stores peer's address and port to peerinfo.
   * @param peerinfo placeholder to store peer's address and port.
   */
  void getPeerInfo(pair<string, int>& peerinfo) const;

  /**
   * Accepts incoming connection on this socket.
   * You must call beginListen() before calling this method.
   * @return accepted socket. The caller must delete it after using it.
   */
  SocketCore* acceptConnection() const;

  /**
   * Connects to the server named host and the destination port is port.
   * This method makes socket non-blocking mode.
   * To make the socket blocking mode again, call setBlockingMode() after
   * the connection is established.
   * @param host hostname or ip address to connect to
   * @param port service port number to connect to
   */
  void establishConnection(string host, int port);

  /**
   * Makes this socket blocking mode.
   */
  void setBlockingMode() const;

  /**
   * Closes the connection of this socket.
   */
  void closeConnection();

  /**
   * Checks whether this socket is available for writing.
   * @param timeout the amount of time elapsed before the checking are timed
   * out.
   * @return true if the socket is available for writing,
   * otherwise returns false.
   */
  bool isWritable(int timeout) const;

  /**
   * Checks whether this socket is available for reading.
   * @param timeout the amount of time elapsed before the checking are timed
   * out.
   * @return true if the socket is available for reading,
   * otherwise returns false.
   */
  bool isReadable(int timeout) const;

  /**
   * Writes characters into this socket. data is a pointer pointing the first
   * byte of the data and len is the length of data.
   * This method internally calls isWritable(). The parmeter timeout is used
   * for this method call.
   * @param data data to write
   * @param len length of data
   * @param timeout the amount of time elapsed before isWritable()
   * are timed out.
   */
  void writeData(const char* data, int len, int timeout = 0);

  /**
   * Reads up to len bytes from this socket.
   * data is a pointer pointing the first
   * byte of the data, which must be allocated before this method is called.
   * len is the size of the allocated memory. When this method returns
   * successfully, len is replaced by the size of the read data.
   * This method internally calls isReadable(). The parameter timeout is used
   * for this method call.
   * @param data holder to store data.
   * @param len the maximum size data can store. This method assigns
   * the number of bytes read to len.
   * @param timeout the amount of time elapsed before isReadable() are timed
   * out.
   */
  void readData(char* data, int& len, int timeout = 0);

  /**
   * Reads up to len bytes from this socket, but bytes are not removed from
   * this socket.
   * This method internally calls isReadable(). The parameter timeout is used
   * for this method call.
   * @param data holder to store data.
   * @param len the maximum size data can store. This method assigns
   * the number of bytes read to len.
   * @param timeout the amount of time elapsed before isReadable() are timed
   * out.
   */
  void peekData(char* data, int& len, int timeout = 0);
  
  /**
   * Makes this socket secure.
   * If the system has not OpenSSL, then this method do nothing.
   * connection must be established  before calling this method.
   */
  void initiateSecureConnection() ;
};

#endif // _D_SOCKET_CORE_H_
