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

#ifndef WIN_TLS_SESSION_H
#define WIN_TLS_SESSION_H

#include <vector>

#include "common.h"
#include "TLSSession.h"
#include "WinTLSContext.h"

namespace aria2 {

namespace wintls {
struct Buffer {
private:
  size_t off_, free_, cap_;
  std::vector<char> buf_;

public:
  inline Buffer() : off_(0), free_(0), cap_(0) {}

  inline size_t size() const { return off_; }

  inline size_t free() const { return free_; }

  inline void resize(size_t len)
  {
    if (cap_ >= len) {
      return;
    }
    buf_.resize(len);
    cap_ = buf_.size();
    free_ = cap_ - off_;
  }

  inline char* data() { return buf_.data(); }

  inline char* end() { return buf_.data() + off_; }

  inline void eat(size_t len)
  {
    off_ -= len;
    if (off_) {
      memmove(buf_.data(), buf_.data() + len, off_);
    }
    free_ = cap_ - off_;
  }

  inline void clear() { eat(off_); }

  inline void advance(size_t len)
  {
    off_ += len;
    free_ = cap_ - off_;
  }

  inline void write(const void* data, size_t len)
  {
    if (!len) {
      return;
    }
    resize(off_ + len);
    memcpy(end(), data, len);
    advance(len);
  }
};
} // namespace wintls

class TLSBuffer : public ::SecBuffer {
public:
  TLSBuffer() : ::SecBuffer{} {}

  explicit TLSBuffer(ULONG type, ULONG size, void* data)
  {
    cbBuffer = size;
    BufferType = type;
    pvBuffer = data;
  }
};

class WinTLSSession : public TLSSession {
  enum state_t {
    st_constructed,
    st_initialized,
    st_handshake_write,
    st_handshake_write_last,
    st_handshake_read,
    st_handshake_done,
    st_connected,
    st_closing,
    st_closed,
    st_error
  };

public:
  WinTLSSession(WinTLSContext* ctx);

  // MUST deallocate all resources
  virtual ~WinTLSSession();

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

  virtual size_t getRecvBufferedLength() CXX11_OVERRIDE;

private:
  // Obtains TLS record size limits.  This function returns 0 if it
  // succeeds, or -1.  status_ and state_ are updated according to the
  // result.
  int obtainTLSRecordSizes();
  // Ensures the buffer size so that maximum TLS record can be sent.
  void ensureSendBuffer();
  // Sends TLS record specified in sendRecordBuffers_.  It uses
  // recordBytesSent_ to track down how many bytes have been sent.
  // This function returns 0 if it succeeds, or negative error codes.
  int sendTLSRecord();
  // Returns the number of bytes in the remaining TLS record size.
  size_t getLeftTLSRecordSize() const;

  std::string hostname_;
  sock_t sockfd_;
  TLSSessionSide side_;
  CredHandle* cred_;
  CtxtHandle handle_;

  // Buffer for already encrypted writes.  This is only used in
  // handshake.
  wintls::Buffer writeBuf_;
  // While the sendRecordBuffers_ holds encrypted messages,
  // writeBuffered_ has the corresponding size of unencrypted data
  // used to produce the messages.
  size_t writeBuffered_;
  // Buffer for still encrypted reads
  wintls::Buffer readBuf_;
  // Buffer for already decrypted reads
  wintls::Buffer decBuf_;

  state_t state_;

  SECURITY_STATUS status_;
  // The number of maximum size for TLS record header, body, and
  // trailer.
  SecPkgContext_StreamSizes streamSizes_;
  // Underlying buffer for outgoing TLS record.
  std::vector<unsigned char> sendBuffer_;
  // How many bytes has been sent for current TLS record held in
  // sendRecordBuffers_.
  size_t recordBytesSent_;
  // This holds current outgoing TLS record.
  std::array<TLSBuffer, 4> sendRecordBuffers_;
};

} // namespace aria2

#endif // TLS_SESSION_H
