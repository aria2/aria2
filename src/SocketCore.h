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

#ifdef HAVE_EPOLL
# include <sys/epoll.h>
#endif // HAVE_EPOLL

#include <string>
#include <cstdlib>
#include <utility>

#ifdef HAVE_LIBSSL
// for SSL
# include <openssl/ssl.h>
# include <openssl/err.h>
#endif // HAVE_LIBSSL
#ifdef HAVE_LIBGNUTLS
# include <gnutls/gnutls.h>
#endif // HAVE_LIBGNUTLS

#include "SharedHandle.h"
#include "a2io.h"
#include "a2netcompat.h"
#include "a2time.h"

namespace aria2 {

#ifdef ENABLE_SSL
class TLSContext;
#endif // ENABLE_SSL

class SocketCore {
  friend bool operator==(const SocketCore& s1, const SocketCore& s2);
  friend bool operator!=(const SocketCore& s1, const SocketCore& s2);
  friend bool operator<(const SocketCore& s1, const SocketCore& s2);
private:
  // socket type defined in <sys/socket.h>
  int _sockType;
  // socket endpoint descriptor
  sock_t sockfd;

#ifdef HAVE_EPOLL

  // file descriptor used for epoll
  int _epfd;

  struct epoll_event _epEvent;

#endif // HAVE_EPOLL

  enum PollMethod {
    POLL_METHOD_EPOLL, POLL_METHOD_SELECT
  };

  static PollMethod _pollMethod;

  static int _protocolFamily;

  static SharedHandle<struct sockaddr_storage> _bindAddr;

  static socklen_t _bindAddrLen;

  bool blocking;
  int secure;

  bool _wantRead;
  bool _wantWrite;

#if ENABLE_SSL
  static SharedHandle<TLSContext> _tlsContext;
#endif // ENABLE_SSL

#ifdef HAVE_LIBSSL
  // for SSL
  SSL* ssl;

  int sslHandleEAGAIN(int ret);
#endif // HAVE_LIBSSL
#ifdef HAVE_LIBGNUTLS
  gnutls_session_t sslSession;
  char* peekBuf;
  size_t peekBufLength;
  size_t peekBufMax;

  size_t shiftPeekData(char* data, size_t len);
  void addPeekData(char* data, size_t len);
  ssize_t gnutlsRecv(char* data, size_t len);
  ssize_t gnutlsPeek(char* data, size_t len);

  void gnutlsRecordCheckDirection();
#endif // HAVE_LIBGNUTLS

  void init();

  void bind(const struct sockaddr* addr, socklen_t addrlen);

#ifdef HAVE_EPOLL

  void initEPOLL();

#endif // HAVE_EPOLL

  SocketCore(sock_t sockfd, int sockType);

  static int error();
  static const char *errorMsg();
  static const char *errorMsg(const int err);
public:
  SocketCore(int sockType = SOCK_STREAM);
  ~SocketCore();

  sock_t getSockfd() const { return sockfd; }

  bool isOpen() const { return sockfd != (sock_t) -1; }

  /**
   * Creates a socket and bind it with locahost's address and port.
   * flags is set to struct addrinfo's ai_flags.
   * @param port port to listen. If 0 is specified, os automaticaly
   * choose avaiable port.
   */
  void bind(uint16_t port, int flags = AI_PASSIVE);

  /**
   * Listens form connection on it.
   * Call bind(uint16_t) before calling this function.
   */
  void beginListen();

  /**
   * Stores host address and port of this socket to addrinfo.
   * @param addrinfo placeholder to store host address and port.
   */
  void getAddrInfo(std::pair<std::string, uint16_t>& addrinfo) const;

  /**
   * Stores peer's address and port to peerinfo.
   * @param peerinfo placeholder to store peer's address and port.
   */
  void getPeerInfo(std::pair<std::string, uint16_t>& peerinfo) const;

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
  void establishConnection(const std::string& host, uint16_t port);

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
  bool isWritable(time_t timeout);

  /**
   * Checks whether this socket is available for reading.
   * @param timeout the amount of time elapsed before the checking are timed
   * out.
   * @return true if the socket is available for reading,
   * otherwise returns false.
   */
  bool isReadable(time_t timeout);

  /**
   * Writes data into this socket. data is a pointer pointing the first
   * byte of the data and len is the length of data.
   * If the underlying socket is in blocking mode, this method may block until
   * all data is sent.
   * If the underlying socket is in non-blocking mode, this method may return
   * even if all data is sent. The size of written data is returned. If
   * underlying socket gets EAGAIN, _wantRead or _wantWrite is set accordingly.
   * This method sets _wantRead and _wantWrite to false before do anything else.
   * @param data data to write
   * @param len length of data
   */
  ssize_t writeData(const char* data, size_t len);
  ssize_t writeData(const std::string& msg)
  {
    return writeData(msg.c_str(), msg.size());
  }
  ssize_t writeData(const unsigned char* data, size_t len)
  {
    return writeData(reinterpret_cast<const char*>(data), len);
  }

  ssize_t writeData(const char* data, size_t len,
		    const std::string& host, uint16_t port);

  ssize_t writeData(const unsigned char* data, size_t len,
		    const std::string& host,
		    uint16_t port)
  {
    return writeData(reinterpret_cast<const char*>(data), len, host, port);
  }

  /**
   * Reads up to len bytes from this socket.
   * data is a pointer pointing the first
   * byte of the data, which must be allocated before this method is called.
   * len is the size of the allocated memory. When this method returns
   * successfully, len is replaced by the size of the read data.
   * If the underlying socket is in blocking mode, this method may block until
   * at least 1byte is received.
   * If the underlying socket is in non-blocking mode, this method may return
   * even if no single byte is received. If the underlying socket gets EAGAIN,
   * _wantRead or _wantWrite is set accordingly.
   * This method sets _wantRead and _wantWrite to false before do anything else.
   * @param data holder to store data.
   * @param len the maximum size data can store. This method assigns
   * the number of bytes read to len.
   */
  void readData(char* data, size_t& len);

  void readData(unsigned char* data, size_t& len)
  {
    readData(reinterpret_cast<char*>(data), len);
  }

  ssize_t readDataFrom(char* data, size_t len,
		       std::pair<std::string /* numerichost */,
		       uint16_t /* port */>& sender);

  ssize_t readDataFrom(unsigned char* data, size_t len,
		       std::pair<std::string /* numerichost */,
		       uint16_t /* port */>& sender)
  {
    return readDataFrom(reinterpret_cast<char*>(data), len, sender);
  }

  /**
   * Reads up to len bytes from this socket, but bytes are not removed from
   * this socket.
   * This method internally calls isReadable(). The parameter timeout is used
   * for this method call.
   * @param data holder to store data.
   * @param len the maximum size data can store. This method assigns
   * the number of bytes read to len.
   */
  void peekData(char* data, size_t& len);

  void peekData(unsigned char* data, size_t& len)
  {
    peekData(reinterpret_cast<char*>(data), len);
  }

  /**
   * Makes this socket secure.
   * If the system has not OpenSSL, then this method do nothing.
   * connection must be established  before calling this method.
   *
   * If you are going to verify peer's certificate, hostname must be supplied.
   */
  bool initiateSecureConnection(const std::string& hostname="");

  void prepareSecureConnection();

  bool operator==(const SocketCore& s) {
    return sockfd == s.sockfd;
  }

  bool operator!=(const SocketCore& s) {
    return !(*this == s);
  }

  bool operator<(const SocketCore& s) {
    return sockfd < s.sockfd;
  }

  std::string getSocketError() const;

  /**
   * Returns true if the underlying socket gets EAGAIN in the previous
   * readData() or writeData() and the socket needs more incoming data to
   * continue the operation.
   */
  bool wantRead() const;

  /**
   * Returns true if the underlying socket gets EAGAIN in the previous
   * readData() or writeData() and the socket needs to write more data.
   */
  bool wantWrite() const;

#ifdef HAVE_EPOLL
  static void useEpoll();
#endif // HAVE_EPOLL
  static void useSelect();

#ifdef ENABLE_SSL
  static void setTLSContext(const SharedHandle<TLSContext>& tlsContext);
#endif // ENABLE_SSL

  static void setProtocolFamily(int protocolFamily)
  {
    _protocolFamily = protocolFamily;
  }

  // Bind socket to interface. interface may be specified as a
  // hostname, IP address or interface name like eth0.  If the given
  // interface is not found or binding socket is failed, exception
  // will be thrown.  Set _protocolFamily before calling this function
  // if you limit protocol family.
  static void bindAddress(const std::string& interface);
};

} // namespace aria2

#endif // _D_SOCKET_CORE_H_
