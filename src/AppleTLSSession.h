/* <!-- copyright */
/*
 * aria2 - The high speed download utility
 *
 * Copyright (C) 2013 Nils Maier
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
#ifndef APPLE_TLS_SESSION_H
#define APPLE_TLS_SESSION_H

#include "common.h"
#include "TLSSession.h"
#include "AppleTLSContext.h"

namespace aria2 {

class AppleTLSSession : public TLSSession {
  enum state_t {
    st_constructed,
    st_initialized,
    st_connected,
    st_closed,
    st_error
  };

public:
  AppleTLSSession(AppleTLSContext* ctx);

  // MUST deallocate all resources
  virtual ~AppleTLSSession();

  // Initializes SSL/TLS session. The |sockfd| is the underlying
  // transport socket. This function returns TLS_ERR_OK if it
  // succeeds, or TLS_ERR_ERROR.
  virtual int init(sock_t sockfd) CXX11_OVERRIDE;

  // Sets |hostname| for TLS SNI extension. This is only meaningful for
  // client side session. This function returns TLS_ERR_OK if it
  // succeeds, or TLS_ERR_ERROR.
  virtual int setSNIHostname(const std::string& hostname) CXX11_OVERRIDE;

  // Closes the SSL/TLS session. Don't close underlying transport
  // socket. This function returns TLS_ERR_OK if it succeeds, or
  // TLS_ERR_ERROR.
  virtual int closeConnection() CXX11_OVERRIDE;

  // Returns TLS_WANT_READ if SSL/TLS session needs more data from
  // remote endpoint to proceed, or TLS_WANT_WRITE if SSL/TLS session
  // needs to write more data to proceed. If SSL/TLS session needs
  // neither read nor write data at the moment, return value is
  // undefined.
  virtual int checkDirection() CXX11_OVERRIDE;

  // Sends |data| with length |len|. This function returns the number
  // of bytes sent if it succeeds, or TLS_ERR_WOULDBLOCK if the
  // underlying transport blocks, or TLS_ERR_ERROR.
  virtual ssize_t writeData(const void* data, size_t len) CXX11_OVERRIDE;

  // Receives data into |data| with length |len|. This function returns
  // the number of bytes received if it succeeds, or TLS_ERR_WOULDBLOCK
  // if the underlying transport blocks, or TLS_ERR_ERROR.
  virtual ssize_t readData(void* data, size_t len) CXX11_OVERRIDE;

  // Performs client side handshake. The |hostname| is the hostname of
  // the remote endpoint and is used to verify its certificate. This
  // function returns TLS_ERR_OK if it succeeds, or TLS_ERR_WOULDBLOCK
  // if the underlying transport blocks, or TLS_ERR_ERROR.
  // When returning TLS_ERR_ERROR, provide certificate validation error
  // in |handshakeErr|.
  virtual int tlsConnect(const std::string& hostname, TLSVersion& version,
                         std::string& handshakeErr) CXX11_OVERRIDE;

  // Performs server side handshake. This function returns TLS_ERR_OK
  // if it succeeds, or TLS_ERR_WOULDBLOCK if the underlying transport
  // blocks, or TLS_ERR_ERROR.
  virtual int tlsAccept(TLSVersion& version) CXX11_OVERRIDE;

  // Returns last error string
  virtual std::string getLastErrorString() CXX11_OVERRIDE;

  virtual size_t getRecvBufferedLength() CXX11_OVERRIDE { return 0; }

private:
  static OSStatus SocketWrite(SSLConnectionRef conn, const void* data,
                              size_t* len)
  {
    return ((AppleTLSSession*)conn)->sockWrite(data, len);
  }

  static OSStatus SocketRead(SSLConnectionRef conn, void* data, size_t* len)
  {
    return ((AppleTLSSession*)conn)->sockRead(data, len);
  }

  SSLContextRef sslCtx_;
  sock_t sockfd_;
  state_t state_;
  OSStatus lastError_;
  size_t writeBuffered_;

  OSStatus sockWrite(const void* data, size_t* len);
  OSStatus sockRead(void* data, size_t* len);
};
} // namespace aria2

#endif // TLS_SESSION_H
