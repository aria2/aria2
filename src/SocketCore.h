/* <!-- copyright */
/*
 * aria2 - The high speed download utility
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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
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
#ifndef _D_SOCKET_CORE_H_
#define _D_SOCKET_CORE_H_

#include "common.h"
#include <string>
#include <utility>
#include <sys/socket.h>

#ifdef HAVE_LIBSSL
// for SSL
# include <openssl/ssl.h>
# include <openssl/err.h>
#endif // HAVE_LIBSSL
#ifdef HAVE_LIBGNUTLS
# include <gnutls/gnutls.h>
#endif // HAVE_LIBGNUTLS

using namespace std;

class SocketCore {
  friend bool operator==(const SocketCore& s1, const SocketCore& s2);
  friend bool operator!=(const SocketCore& s1, const SocketCore& s2);
  friend bool operator<(const SocketCore& s1, const SocketCore& s2);
private:
  // socket type defined in <sys/socket.h>
  int _sockType;
  // socket endpoint descriptor
  int32_t sockfd;
  // reference counter for this object.
  int32_t use;
  bool blocking;
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
  int32_t peekBufLength;
  int32_t peekBufMax;

  int32_t shiftPeekData(char* data, int32_t len);
  void addPeekData(char* data, int32_t len);
  int32_t gnutlsRecv(char* data, int32_t len);
  int32_t gnutlsPeek(char* data, int32_t len);
#endif // HAVE_LIBGNUTLS

  void init();
  SocketCore(int32_t sockfd, int sockType);
  static int error();
  static const char *errorMsg();
  static const char *errorMsg(const int err);
public:
  SocketCore(int sockType = SOCK_STREAM);
  ~SocketCore();

  int32_t getSockfd() const { return sockfd; }

  bool isOpen() const { return sockfd != -1; }

  /**
   * Creates a socket and bind it with locahost's address and port.
   * @param port port to listen. If 0 is specified, os automaticaly
   * choose avaiable port.
   */
  void bind(uint16_t port);

  /**
   * Listens form connection on it.
   * Call bind(uint16_t) before calling this function.
   */
  void beginListen();

  /**
   * Stores host address and port of this socket to addrinfo.
   * @param addrinfo placeholder to store host address and port.
   */
  void getAddrInfo(pair<string, int32_t>& addrinfo) const;
  
  /**
   * Stores peer's address and port to peerinfo.
   * @param peerinfo placeholder to store peer's address and port.
   */
  void getPeerInfo(pair<string, int32_t>& peerinfo) const;

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
  void establishConnection(const string& host, int32_t port);

  void setNonBlockingMode();

  /**
   * Makes this socket blocking mode.
   */
  void setBlockingMode();

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
  bool isWritable(int32_t timeout) const;

  /**
   * Checks whether this socket is available for reading.
   * @param timeout the amount of time elapsed before the checking are timed
   * out.
   * @return true if the socket is available for reading,
   * otherwise returns false.
   */
  bool isReadable(int32_t timeout) const;

  /**
   * Writes characters into this socket. data is a pointer pointing the first
   * byte of the data and len is the length of data.
   * This method internally calls isWritable(). The parmeter timeout is used
   * for this method call.
   * @param data data to write
   * @param len length of data
   */
  void writeData(const char* data, int32_t len);
  void writeData(const string& msg)
  {
    writeData(msg.c_str(), msg.size());
  }

  void writeData(const char* data, size_t len, const string& host, uint16_t port);

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
   */
  void readData(char* data, int32_t& len);

  ssize_t readDataFrom(char* data, size_t len, struct sockaddr* sender,
		       socklen_t* senderLength);

  ssize_t readDataFrom(char*, size_t len,
		       pair<string /* numerichost */,
		       uint16_t /* port */>& sender);

  ssize_t readDataFrom(char* data, size_t len);

  /**
   * Reads up to len bytes from this socket, but bytes are not removed from
   * this socket.
   * This method internally calls isReadable(). The parameter timeout is used
   * for this method call.
   * @param data holder to store data.
   * @param len the maximum size data can store. This method assigns
   * the number of bytes read to len.
   */
  void peekData(char* data, int32_t& len);
  
  /**
   * Makes this socket secure.
   * If the system has not OpenSSL, then this method do nothing.
   * connection must be established  before calling this method.
   */
  void initiateSecureConnection();

  bool operator==(const SocketCore& s) {
    return sockfd == s.sockfd;
  }
  
  bool operator!=(const SocketCore& s) {
    return !(*this == s);
  }
  
  bool operator<(const SocketCore& s) {
    return sockfd < s.sockfd;
  }
};
#endif // _D_SOCKET_CORE_H_
