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

#include "AppleTLSSession.h"

#include <CoreFoundation/CoreFoundation.h>

#include "fmt.h"
#include "LogFactory.h"

#define ioErr -36
#define paramErr -50
#define errSSLServerAuthCompleted -9841

namespace {
  static const SSLProtocol kTLSProtocol11_h = (SSLProtocol)(kSSLProtocolAll + 1);
  static const SSLProtocol kTLSProtocol12_h = (SSLProtocol)(kSSLProtocolAll + 2);
}

namespace aria2 {

TLSSession* TLSSession::make(TLSContext* ctx)
{
  return new AppleTLSSession(static_cast<AppleTLSContext*>(ctx));
}

AppleTLSSession::AppleTLSSession(AppleTLSContext* ctx)
  : ctx_(ctx),
    sslCtx_(0),
    sockfd_(0),
    state_(st_constructed),
    lastError_(noErr),
    writeBuffered_(0)
{
  lastError_ = SSLNewContext(ctx->getSide() == TLS_SERVER, &sslCtx_) == noErr;
  if (lastError_ == noErr) {
    state_ = st_error;
    return;
  }
#if defined(__MAC_10_8)
  (void)SSLSetProtocolVersionMin(sslCtx_, kSSLProtocol3);
  (void)SSLSetProtocolVersionMax(sslCtx_, kTLSProtocol12);
#else
  (void)SSLSetProtocolVersionEnabled(sslCtx_, kSSLProtocolAll, false);
  (void)SSLSetProtocolVersionEnabled(sslCtx_, kSSLProtocol3, true);
  (void)SSLSetProtocolVersionEnabled(sslCtx_, kTLSProtocol1, true);
  (void)SSLSetProtocolVersionEnabled(sslCtx_, kTLSProtocol11_h, true);
  (void)SSLSetProtocolVersionEnabled(sslCtx_, kTLSProtocol12_h, true);
#endif
  (void)SSLSetEnableCertVerify(sslCtx_, ctx->getVerifyPeer());
}

AppleTLSSession::~AppleTLSSession()
{
  closeConnection();
  if (sslCtx_) {
    SSLDisposeContext(sslCtx_);
    sslCtx_ = 0;
  }
  state_ = st_error;
}

int AppleTLSSession::init(sock_t sockfd)
{
  if (state_ != st_constructed) {
    lastError_ = noErr;
    return TLS_ERR_ERROR;
  }
  lastError_ = SSLSetIOFuncs(sslCtx_, SocketRead, SocketWrite);
  if (lastError_ != noErr) {
    state_ = st_error;
    return TLS_ERR_ERROR;
  }
  lastError_ = SSLSetConnection(sslCtx_, this);
  if (lastError_ != noErr) {
    state_ = st_error;
    return TLS_ERR_ERROR;
  }
  sockfd_ = sockfd;
  state_ = st_initialized;
  return TLS_ERR_OK;
}

int AppleTLSSession::setSNIHostname(const std::string& hostname)
{
  if (state_ != st_initialized) {
    lastError_ = noErr;
    return TLS_ERR_ERROR;
  }
  lastError_ = SSLSetPeerDomainName(sslCtx_, hostname.c_str(), hostname.length());
  return (lastError_ != noErr) ? TLS_ERR_ERROR : TLS_ERR_OK;
}

int AppleTLSSession::closeConnection()
{
  if (state_ != st_connected) {
    lastError_ = noErr;
    return TLS_ERR_ERROR;
  }
  lastError_ = SSLClose(sslCtx_);
  state_ = st_closed;
  return lastError_ == noErr ?  TLS_ERR_OK : TLS_ERR_ERROR;
}

int AppleTLSSession::checkDirection() {
  if (writeBuffered_) {
    return TLS_WANT_WRITE;
  }
  if (state_ == st_connected) {
    size_t buffered;
    lastError_ = SSLGetBufferedReadSize(sslCtx_, &buffered);
    if (lastError_ == noErr && buffered) {
      return TLS_WANT_READ;
    }
  }
  return 0;
}

ssize_t AppleTLSSession::writeData(const void* data, size_t len)
{
  if (state_ != st_connected) {
    lastError_ = noErr;
    return TLS_ERR_ERROR;
  }
  size_t processed = 0;
  if (writeBuffered_) {
    lastError_ = SSLWrite(sslCtx_, 0, 0, &processed);
    switch (lastError_) {
      case noErr:
        processed = writeBuffered_;
        writeBuffered_ = 0;
        return processed;
      case errSSLWouldBlock:
        return TLS_ERR_WOULDBLOCK;
      case errSSLClosedGraceful:
      case errSSLClosedNoNotify:
        closeConnection();
        return TLS_ERR_ERROR;
      default:
        closeConnection();
        state_ = st_error;
        return TLS_ERR_ERROR;
    }
  }

  lastError_ = SSLWrite(sslCtx_, data, len, &processed);
  switch (lastError_) {
    case noErr:
      return processed;
    case errSSLWouldBlock:
      writeBuffered_ = len;
      return TLS_ERR_WOULDBLOCK;
    case errSSLClosedGraceful:
    case errSSLClosedNoNotify:
      closeConnection();
      return TLS_ERR_ERROR;
    default:
      closeConnection();
      state_ = st_error;
      return TLS_ERR_ERROR;
  }
}
OSStatus AppleTLSSession::sockWrite(const void* data, size_t* len)
{
  size_t remain = *len;
  const uint8_t *buffer = static_cast<const uint8_t*>(data);
  *len = 0;
  while (remain) {
    ssize_t w = write(sockfd_, buffer, remain);
    if (w <= 0) {
      switch (errno) {
        case EAGAIN:
          return errSSLWouldBlock;
        default:
          return errSSLClosedAbort;
      }
    }
    remain -= w;
    buffer += w;
    *len += w;
  }
  return noErr;
}
ssize_t AppleTLSSession::readData(void* data, size_t len)
{
  if (state_ != st_connected) {
    lastError_ = noErr;
    return TLS_ERR_ERROR;
  }
  size_t processed = 0;
  lastError_ = SSLRead(sslCtx_, data, len, &processed);
  switch (lastError_) {
    case noErr:
      return processed;
    case errSSLWouldBlock:
      if (processed) {
        return processed;
      }
      return TLS_ERR_WOULDBLOCK;
    case errSSLClosedGraceful:
    case errSSLClosedNoNotify:
      closeConnection();
      return TLS_ERR_ERROR;
    default:
      closeConnection();
      state_ = st_error;
      return TLS_ERR_ERROR;
  }
}

OSStatus AppleTLSSession::sockRead(void* data, size_t* len)
{
  size_t remain = *len;
  uint8_t *buffer = static_cast<uint8_t*>(data);
  *len = 0;
  while (remain) {
    ssize_t r = read(sockfd_, buffer, remain);
    if (r == 0) {
      return errSSLClosedGraceful;
    }
    if (r < 0) {
      switch (errno) {
        case ENOENT:
          return errSSLClosedGraceful;
        case ECONNRESET:
          return errSSLClosedAbort;
        case EAGAIN:
          return errSSLWouldBlock;
        default:
          return errSSLClosedAbort;
      }
    }
    remain -= r;
    buffer += r;
    *len += r;
  }
  return noErr;
}

int AppleTLSSession::tlsConnect(const std::string& hostname, std::string& handshakeErr)
{
  if (state_ != st_initialized) {
    return TLS_ERR_ERROR;
  }
  if (!hostname.empty()) {
    setSNIHostname(hostname);
  }
  lastError_ = SSLHandshake(sslCtx_);
  switch (lastError_) {
    case noErr:
      state_ = st_connected;
      return TLS_ERR_OK;
    case errSSLWouldBlock:
      return TLS_ERR_WOULDBLOCK;
    case errSSLServerAuthCompleted:
      return tlsConnect(hostname, handshakeErr);
    default:
      handshakeErr = getLastErrorString();
      return TLS_ERR_ERROR;
  }
}

int AppleTLSSession::tlsAccept()
{
  std::string hostname, err;
  return tlsConnect(hostname, err);
}

std::string AppleTLSSession::getLastErrorString()
{
  switch (lastError_) {
    case errSSLProtocol:
      return "Protocol error";
    case errSSLNegotiation:
      return "No common cipher suites";
    case errSSLFatalAlert:
      return "Received fatal alert";
    case errSSLSessionNotFound:
      return "Unknown session";
    case errSSLClosedGraceful:
      return "Closed gracefully";
    case errSSLClosedAbort:
      return "Connection aborted";
    case errSSLXCertChainInvalid:
      return "Invalid certificate chain";
    case errSSLBadCert:
      return "Invalid certificate format";
    case errSSLCrypto:
      return "Cryptographic error";
    case paramErr:
    case errSSLInternal:
      return "Internal SSL error";
    case errSSLUnknownRootCert:
      return "Self-signed certificate";
    case errSSLNoRootCert:
      return "No root certificate";
    case errSSLCertExpired:
      return "Certificate expired";
    case errSSLCertNotYetValid:
      return "Certificate not yet valid";
    case errSSLClosedNoNotify:
      return "Closed without notification";
    case errSSLBufferOverflow:
      return "Buffer not large enough";
    case errSSLBadCipherSuite:
      return "Bad cipher suite";
    case errSSLPeerUnexpectedMsg:
      return "Unexpected peer message";
    case errSSLPeerBadRecordMac:
      return "Bad MAC";
    case errSSLPeerDecryptionFail:
      return "Decryption failure";
    case errSSLHostNameMismatch:
      return "Invalid hostname";
    case errSSLConnectionRefused:
      return "Connection refused";
    default:
      return fmt("Unspecified error %d", lastError_);
  }
}

}
