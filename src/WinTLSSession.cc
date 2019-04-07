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

#include "WinTLSSession.h"

#include <cassert>
#include <sstream>

#include "LogFactory.h"
#include "a2functional.h"
#include "fmt.h"
#include "util.h"

#ifndef SECBUFFER_ALERT
#  define SECBUFFER_ALERT 17
#endif

#ifndef SZ_ALG_MAX_SIZE
#  define SZ_ALG_MAX_SIZE 64
#endif
#ifndef SECPKGCONTEXT_CIPHERINFO_V1
#  define SECPKGCONTEXT_CIPHERINFO_V1 1
#endif
#ifndef SECPKG_ATTR_CIPHER_INFO
#  define SECPKG_ATTR_CIPHER_INFO 0x64
#endif

namespace {
using namespace aria2;

struct WinSecPkgContext_CipherInfo {
  DWORD dwVersion;
  DWORD dwProtocol;
  DWORD dwCipherSuite;
  DWORD dwBaseCipherSuite;
  WCHAR szCipherSuite[SZ_ALG_MAX_SIZE];
  WCHAR szCipher[SZ_ALG_MAX_SIZE];
  DWORD dwCipherLen;
  DWORD dwCipherBlockLen; // in bytes
  WCHAR szHash[SZ_ALG_MAX_SIZE];
  DWORD dwHashLen;
  WCHAR szExchange[SZ_ALG_MAX_SIZE];
  DWORD dwMinExchangeLen;
  DWORD dwMaxExchangeLen;
  WCHAR szCertificate[SZ_ALG_MAX_SIZE];
  DWORD dwKeyType;
};

static const ULONG kReqFlags =
    ISC_REQ_SEQUENCE_DETECT | ISC_REQ_REPLAY_DETECT | ISC_REQ_CONFIDENTIALITY |
    ISC_REQ_ALLOCATE_MEMORY | ISC_REQ_USE_SUPPLIED_CREDS | ISC_REQ_STREAM;

static const ULONG kReqAFlags =
    ASC_REQ_SEQUENCE_DETECT | ASC_REQ_REPLAY_DETECT | ASC_REQ_CONFIDENTIALITY |
    ASC_REQ_EXTENDED_ERROR | ASC_REQ_ALLOCATE_MEMORY | ASC_REQ_STREAM;

class TLSBufferDesc : public ::SecBufferDesc {
public:
  explicit TLSBufferDesc(SecBuffer* arr, ULONG buffers)
  {
    ulVersion = SECBUFFER_VERSION;
    cBuffers = buffers;
    pBuffers = arr;
  }
};

inline static std::string getCipherSuite(CtxtHandle* handle)
{
  WinSecPkgContext_CipherInfo info = {SECPKGCONTEXT_CIPHERINFO_V1};
  if (QueryContextAttributes(handle, SECPKG_ATTR_CIPHER_INFO, &info) ==
      SEC_E_OK) {
    return wCharToUtf8(info.szCipherSuite);
  }
  return "Unknown";
}

inline static uint32_t getProtocolVersion(CtxtHandle* handle)
{
  WinSecPkgContext_CipherInfo info = {SECPKGCONTEXT_CIPHERINFO_V1};
  if (QueryContextAttributes(handle, SECPKG_ATTR_CIPHER_INFO, &info) ==
      SEC_E_OK) {
    return info.dwProtocol;
  }
  // XXX Assume the best?!
  return std::numeric_limits<uint32_t>::max();
}

} // namespace

namespace aria2 {

TLSSession* TLSSession::make(TLSContext* ctx)
{
  return new WinTLSSession(static_cast<WinTLSContext*>(ctx));
}

WinTLSSession::WinTLSSession(WinTLSContext* ctx)
    : sockfd_(0),
      side_(ctx->getSide()),
      cred_(ctx->getCredHandle()),
      writeBuffered_(0),
      state_(st_constructed),
      status_(SEC_E_OK),
      recordBytesSent_(0)
{
  memset(&handle_, 0, sizeof(handle_));
}

WinTLSSession::~WinTLSSession()
{
  ::DeleteSecurityContext(&handle_);
  state_ = st_error;
}

int WinTLSSession::init(sock_t sockfd)
{
  if (state_ != st_constructed) {
    status_ = SEC_E_INVALID_HANDLE;
    return TLS_ERR_ERROR;
  }
  sockfd_ = sockfd;
  state_ = st_initialized;

  return TLS_ERR_OK;
}

int WinTLSSession::setSNIHostname(const std::string& hostname)
{
  if (state_ != st_initialized) {
    status_ = SEC_E_INVALID_HANDLE;
    return TLS_ERR_ERROR;
  }
  hostname_ = hostname;
  return TLS_ERR_OK;
}

int WinTLSSession::closeConnection()
{
  if (state_ != st_connected && state_ != st_closing) {
    if (state_ != st_error) {
      status_ = SEC_E_INVALID_HANDLE;
      state_ = st_error;
    }
    A2_LOG_DEBUG("WinTLS: Cannot close connection");
    return TLS_ERR_ERROR;
  }

  if (state_ == st_connected) {
    A2_LOG_DEBUG("WinTLS: Closing connection");
    state_ = st_closing;

    DWORD dwShut = SCHANNEL_SHUTDOWN;
    TLSBuffer shut(SECBUFFER_TOKEN, sizeof(dwShut), &dwShut);
    TLSBufferDesc shutDesc(&shut, 1);
    status_ = ::ApplyControlToken(&handle_, &shutDesc);
    if (status_ != SEC_E_OK) {
      state_ = st_error;
      return TLS_ERR_ERROR;
    }
    TLSBuffer ctx(SECBUFFER_EMPTY, 0, nullptr);
    TLSBufferDesc desc(&ctx, 1);
    ULONG flags = 0;
    if (side_ == TLS_CLIENT) {
      SEC_CHAR* host = hostname_.empty()
                           ? nullptr
                           : const_cast<SEC_CHAR*>(hostname_.c_str());
      status_ = ::InitializeSecurityContext(cred_, &handle_, host, kReqFlags, 0,
                                            0, nullptr, 0, &handle_, &desc,
                                            &flags, nullptr);
    }
    else {
      status_ = ::AcceptSecurityContext(cred_, &handle_, nullptr, kReqAFlags, 0,
                                        &handle_, &desc, &flags, nullptr);
    }
    if ((status_ == SEC_E_OK || status_ == SEC_I_CONTEXT_EXPIRED) &&
        getLeftTLSRecordSize() == 0) {
      size_t len = ctx.cbBuffer;
      ssize_t rv = writeData(ctx.pvBuffer, ctx.cbBuffer);
      ::FreeContextBuffer(ctx.pvBuffer);
      if (rv == TLS_ERR_WOULDBLOCK) {
        return rv;
      }

      // Ignore error here because we probably don't handle those
      // errors gracefully.  Just shutdown connection.  If rv is
      // positive, then data is sent or buffered
      if (rv > 0 && rv - len != 0) {
        return TLS_ERR_WOULDBLOCK;
      }
    }
  }

  A2_LOG_DEBUG("WinTLS: Closed Connection");
  state_ = st_closed;
  return TLS_ERR_OK;
}

int WinTLSSession::checkDirection()
{
  if (state_ == st_handshake_write || state_ == st_handshake_write_last) {
    return TLS_WANT_WRITE;
  }
  if (state_ == st_handshake_read) {
    return TLS_WANT_READ;
  }
  if (readBuf_.size() || decBuf_.size()) {
    return TLS_WANT_READ;
  }
  if (getLeftTLSRecordSize() || writeBuf_.size()) {
    return TLS_WANT_WRITE;
  }
  return TLS_WANT_READ;
}

namespace {
// Fills |iov| of length |len| to send remaining data in |buffers|.
// We have already sent |offset| bytes.  This function returns the
// number of |iov| filled.  It assumes the array |buffers| is at least
// |len| elements.
size_t fillSendIOV(a2iovec* iov, size_t len, TLSBuffer* buffers, size_t offset)
{
  size_t iovcnt = 0;
  for (size_t i = 0; i < len; ++i) {
    if (offset < buffers[i].cbBuffer) {
      iov[iovcnt].A2IOVEC_BASE =
          static_cast<char*>(buffers[i].pvBuffer) + offset;
      iov[iovcnt].A2IOVEC_LEN = buffers[i].cbBuffer - offset;
      ++iovcnt;
      offset = 0;
    }
    else {
      offset -= buffers[i].cbBuffer;
    }
  }
  return iovcnt;
}
} // namespace

size_t WinTLSSession::getLeftTLSRecordSize() const
{
  return sendRecordBuffers_[0].cbBuffer + sendRecordBuffers_[1].cbBuffer +
         sendRecordBuffers_[2].cbBuffer - recordBytesSent_;
}

int WinTLSSession::sendTLSRecord()
{
  A2_LOG_DEBUG(fmt("WinTLS: TLS record %" PRIu64 " bytes left",
                   static_cast<uint64_t>(getLeftTLSRecordSize())));

  while (getLeftTLSRecordSize()) {
    std::array<a2iovec, 3> iov;
    auto iovcnt = fillSendIOV(iov.data(), iov.size(), sendRecordBuffers_.data(),
                              recordBytesSent_);

    DWORD nwrite;
    auto rv =
        WSASend(sockfd_, iov.data(), iovcnt, &nwrite, 0, nullptr, nullptr);
    if (rv != 0) {
      auto errnum = ::WSAGetLastError();
      if (errnum == WSAEINTR) {
        continue;
      }

      if (errnum == WSAEWOULDBLOCK) {
        return TLS_ERR_WOULDBLOCK;
      }

      A2_LOG_ERROR("WinTLS: Connection error while writing");
      status_ = SEC_E_INCOMPLETE_MESSAGE;
      state_ = st_error;
      return TLS_ERR_ERROR;
    }

    recordBytesSent_ += nwrite;
  }

  recordBytesSent_ = 0;
  sendRecordBuffers_[0].cbBuffer = 0;
  sendRecordBuffers_[1].cbBuffer = 0;
  sendRecordBuffers_[2].cbBuffer = 0;

  return 0;
}

ssize_t WinTLSSession::writeData(const void* data, size_t len)
{
  if (state_ == st_handshake_write || state_ == st_handshake_write_last ||
      state_ == st_handshake_read) {
    // Renegotiating
    std::string hn, err;
    TLSVersion ver;
    auto connect = tlsConnect(hn, ver, err);
    if (connect != TLS_ERR_OK) {
      return connect;
    }
    // Continue.
  }

  if (state_ != st_connected && state_ != st_closing) {
    status_ = SEC_E_INVALID_HANDLE;
    return TLS_ERR_ERROR;
  }

  A2_LOG_DEBUG(fmt("WinTLS: Write request: %" PRIu64 " buffered: %" PRIu64,
                   (uint64_t)len, (uint64_t)recordBytesSent_));

  auto rv = sendTLSRecord();
  if (rv != 0) {
    return rv;
  }

  auto left = len;
  auto bytes = static_cast<const char*>(data);
  if (writeBuffered_) {
    // There was buffered data, hence we need to "remove" that data from the
    // incoming buffer to avoid writing it again
    if (len < writeBuffered_) {
      // We didn't get called with the same data again, obviously.
      status_ = SEC_E_INVALID_HANDLE;
      status_ = st_error;
      return TLS_ERR_ERROR;
    }
    // just advance the buffer by writeBuffered_ bytes
    bytes += writeBuffered_;
    left -= writeBuffered_;
    writeBuffered_ = 0;
  }
  if (!left) {
    // The buffer contained the full remainder. At this point, the buffer has
    // been written, so the request is done in its entirety;
    return len;
  }

  // Buffered data was already written ;)
  // If there was no buffered data, this will be len - len = 0.
  len -= left;
  while (left) {
    // Set up an outgoing message, according to streamSizes_
    writeBuffered_ =
        std::min(left, static_cast<size_t>(streamSizes_.cbMaximumMessage));

    sendRecordBuffers_ = {
        TLSBuffer(SECBUFFER_STREAM_HEADER, streamSizes_.cbHeader,
                  sendBuffer_.data()),
        TLSBuffer(SECBUFFER_DATA, writeBuffered_,
                  sendBuffer_.data() + streamSizes_.cbHeader),
        TLSBuffer(SECBUFFER_STREAM_TRAILER, streamSizes_.cbTrailer,
                  sendBuffer_.data() + streamSizes_.cbHeader + writeBuffered_),
        TLSBuffer(SECBUFFER_EMPTY, 0, nullptr),
    };

    TLSBufferDesc desc(sendRecordBuffers_.data(), sendRecordBuffers_.size());
    std::copy_n(bytes, writeBuffered_,
                static_cast<char*>(sendRecordBuffers_[1].pvBuffer));
    status_ = ::EncryptMessage(&handle_, 0, &desc, 0);
    if (status_ != SEC_E_OK) {
      if (state_ != st_closing) {
        A2_LOG_ERROR(fmt("WinTLS: Failed to encrypt a message! %s",
                         getLastErrorString().c_str()));
      }
      else {
        // On closing state, don't log this message in error level
        // because it seems that the encryption tends to fail in that
        // state.
        A2_LOG_DEBUG(fmt("WinTLS: Failed to encrypt a message! %s",
                         getLastErrorString().c_str()));
      }
      state_ = st_error;
      return TLS_ERR_ERROR;
    }

    A2_LOG_DEBUG(fmt("WinTLS: Write TLS record header: %" PRIu64
                     " body: %" PRIu64 " trailer: %" PRIu64,
                     static_cast<uint64_t>(sendRecordBuffers_[0].cbBuffer),
                     static_cast<uint64_t>(sendRecordBuffers_[1].cbBuffer),
                     static_cast<uint64_t>(sendRecordBuffers_[2].cbBuffer)));

    auto rv = sendTLSRecord();
    if (rv == TLS_ERR_WOULDBLOCK) {
      if (len == 0) {
        return TLS_ERR_WOULDBLOCK;
      }
      return len;
    }

    if (rv != 0) {
      return rv;
    }

    len += writeBuffered_;
    bytes += writeBuffered_;
    left -= writeBuffered_;
    writeBuffered_ = 0;
  }

  A2_LOG_DEBUG(fmt("WinTLS: Write result: %" PRIu64, (uint64_t)len));

  return len;
}

ssize_t WinTLSSession::readData(void* data, size_t len)
{
  A2_LOG_DEBUG(fmt("WinTLS: Read request: %" PRIu64 " buffered: %" PRIu64,
                   (uint64_t)len, (uint64_t)readBuf_.size()));
  if (len == 0) {
    return 0;
  }

  // Can be filled from decBuffer entirely?
  if (decBuf_.size() >= len) {
    A2_LOG_DEBUG("WinTLS: Fullfilling req from buffer");
    memcpy(data, decBuf_.data(), len);
    decBuf_.eat(len);
    return len;
  }

  if (state_ == st_closing || state_ == st_closed || state_ == st_error) {
    auto nread = decBuf_.size();
    if (nread) {
      assert(nread < len);
      memcpy(data, decBuf_.data(), nread);
      decBuf_.clear();
      A2_LOG_DEBUG("WinTLS: Sending out decrypted buffer after EOF");
      return nread;
    }

    A2_LOG_DEBUG("WinTLS: Read request aborted. Connection already closed");
    return state_ == st_error ? TLS_ERR_ERROR : 0;
  }

  if (state_ == st_handshake_write || state_ == st_handshake_write_last ||
      state_ == st_handshake_read) {
    // Renegotiating
    std::string hn, err;
    TLSVersion ver;
    auto connect = tlsConnect(hn, ver, err);
    if (connect != TLS_ERR_OK) {
      return connect;
    }
    // Continue.
  }

  if (state_ != st_connected) {
    status_ = SEC_E_INVALID_HANDLE;
    return TLS_ERR_ERROR;
  }

  // Read as many bytes as available from the connection, up to len + 4k.
  readBuf_.resize(len + 4_k);
  while (readBuf_.free()) {
    ssize_t read = ::recv(sockfd_, readBuf_.end(), readBuf_.free(), 0);
    errno = ::WSAGetLastError();
    if (read < 0 && errno == WSAEINTR) {
      continue;
    }
    if (read < 0 && errno == WSAEWOULDBLOCK) {
      break;
    }
    if (read < 0) {
      status_ = errno;
      state_ = st_error;
      return TLS_ERR_ERROR;
    }
    if (read == 0) {
      A2_LOG_DEBUG("WinTLS: Connection abruptly closed!");
      // At least try to gracefully close our write end.
      closeConnection();
      break;
    }
    readBuf_.advance(read);
  }

  // Try to decrypt as many messages as possible from the readBuf_.
  while (readBuf_.size()) {
    TLSBuffer bufs[] = {
        TLSBuffer(SECBUFFER_DATA, readBuf_.size(), readBuf_.data()),
        TLSBuffer(SECBUFFER_EMPTY, 0, nullptr),
        TLSBuffer(SECBUFFER_EMPTY, 0, nullptr),
        TLSBuffer(SECBUFFER_EMPTY, 0, nullptr),
    };
    TLSBufferDesc desc(bufs, 4);
    status_ = ::DecryptMessage(&handle_, &desc, 0, nullptr);
    if (status_ == SEC_E_INCOMPLETE_MESSAGE) {
      // Need to stop now, and wait for more bytes to arrive on the socket.
      break;
    }

    if (status_ != SEC_E_OK && status_ != SEC_I_CONTEXT_EXPIRED &&
        status_ != SEC_I_RENEGOTIATE) {
      A2_LOG_ERROR(fmt("WinTLS: Failed to decrypt a message! %s",
                       getLastErrorString().c_str()));
      state_ = st_error;
      return TLS_ERR_ERROR;
    }

    // Decrypted message successfully.  Inspired from curl schannel.c.
    if (bufs[1].BufferType == SECBUFFER_DATA && bufs[1].cbBuffer > 0) {
      decBuf_.write(bufs[1].pvBuffer, bufs[1].cbBuffer);
    }
    if (bufs[3].BufferType == SECBUFFER_EXTRA && bufs[3].cbBuffer > 0) {
      readBuf_.eat(readBuf_.size() - bufs[3].cbBuffer);
    }
    else {
      readBuf_.clear();
    }

    if (status_ == SEC_I_RENEGOTIATE) {
      // Renegotiation basically means performing another handshake
      state_ = st_initialized;
      A2_LOG_INFO("WinTLS: Renegotiate");
      std::string hn, err;
      TLSVersion ver;
      auto connect = tlsConnect(hn, ver, err);
      if (connect == TLS_ERR_WOULDBLOCK) {
        break;
      }
      if (connect == TLS_ERR_ERROR) {
        return connect;
      }
      // Still good.
    }
    if (status_ == SEC_I_CONTEXT_EXPIRED) {
      // Connection is gone now, but the buffered bytes are still valid.
      A2_LOG_DEBUG("WinTLS: Connection gracefully closed!");
      closeConnection();
      break;
    }
  }

  len = std::min(decBuf_.size(), len);
  if (len == 0) {
    if (state_ != st_connected) {
      return state_ == st_error ? TLS_ERR_ERROR : 0;
    }

    return TLS_ERR_WOULDBLOCK;
  }
  memcpy(data, decBuf_.data(), len);
  decBuf_.eat(len);
  return len;
}

int WinTLSSession::tlsConnect(const std::string& hostname, TLSVersion& version,
                              std::string& handshakeErr)
{
  // Handshaking will require sending multiple read/write exchanges until the
  // handshake is actually done. The client will first generate the initial
  // handshake message, then write that to the server, read the response
  // message, and write and/or read additional messages until the handshake is
  // either complete and successful, or something went wrong.
  // The server works analog to that.

  A2_LOG_DEBUG("WinTLS: Starting/Resuming TLS Connect");
  ULONG flags = 0;

restart:

  switch (state_) {
  default:
    A2_LOG_ERROR("WinTLS: Invalid state");
    status_ = SEC_E_INVALID_HANDLE;
    return TLS_ERR_ERROR;

  case st_initialized: {
    if (side_ == TLS_SERVER) {
      goto read;
    }

    if (!hostname.empty()) {
      setSNIHostname(hostname);
    }
    A2_LOG_DEBUG("WinTLS: Initializing handshake");
    TLSBuffer buf(SECBUFFER_EMPTY, 0, nullptr);
    TLSBufferDesc desc(&buf, 1);
    SEC_CHAR* host =
        hostname_.empty() ? nullptr : const_cast<SEC_CHAR*>(hostname_.c_str());
    status_ = ::InitializeSecurityContext(cred_, nullptr, host, kReqFlags, 0, 0,
                                          nullptr, 0, &handle_, &desc, &flags,
                                          nullptr);
    if (status_ != SEC_I_CONTINUE_NEEDED) {
      // Has to be SEC_I_CONTINUE_NEEDED, as we did not actually send data
      // at this point.
      state_ = st_error;
      return TLS_ERR_ERROR;
    }

    // Queue the initial message...
    writeBuf_.write(buf.pvBuffer, buf.cbBuffer);
    FreeContextBuffer(buf.pvBuffer);

    // ... and start sending it
    state_ = st_handshake_write;
  }
    // Fall through

  case st_handshake_write_last:
  case st_handshake_write: {
    A2_LOG_DEBUG("WinTLS: Writing handshake");

    // Write the currently queued handshake message until all data is sent.
    while (writeBuf_.size()) {
      ssize_t writ = ::send(sockfd_, writeBuf_.data(), writeBuf_.size(), 0);
      errno = ::WSAGetLastError();
      if (writ < 0 && errno == WSAEINTR) {
        continue;
      }
      if (writ < 0 && errno == WSAEWOULDBLOCK) {
        return TLS_ERR_WOULDBLOCK;
      }
      if (writ <= 0) {
        status_ = errno;
        state_ = st_error;
        return TLS_ERR_ERROR;
      }
      writeBuf_.eat(writ);
    }

    if (state_ == st_handshake_write_last) {
      state_ = st_handshake_done;
      goto restart;
    }

    // Have to read one or more response messages.
    state_ = st_handshake_read;
  }
    // Fall through

  case st_handshake_read: {
  read:
    A2_LOG_DEBUG("WinTLS: Reading handshake...");

    // All write buffered data is invalid at this point!
    writeBuf_.clear();

    // Read as many bytes as possible, up to 4k new bytes.
    // We do not know how many bytes will arrive from the server at this
    // point.
    readBuf_.resize(readBuf_.size() + 4_k);
    while (readBuf_.free()) {
      ssize_t read = ::recv(sockfd_, readBuf_.end(), readBuf_.free(), 0);
      errno = ::WSAGetLastError();
      if (read < 0 && errno == WSAEINTR) {
        continue;
      }
      if (read < 0 && errno == WSAEWOULDBLOCK) {
        break;
      }
      if (read <= 0) {
        status_ = errno;
        state_ = st_error;
        return TLS_ERR_ERROR;
      }
      if (read == 0) {
        A2_LOG_DEBUG("WinTLS: Connection abruptly closed during handshake!");
        status_ = SEC_E_INCOMPLETE_MESSAGE;
        state_ = st_error;
        return TLS_ERR_ERROR;
      }
      readBuf_.advance(read);
      break;
    }
    if (!readBuf_.size()) {
      return TLS_ERR_WOULDBLOCK;
    }

    // Need to copy the data, as Schannel is free to mess with it. But we
    // might later need unmodified data from the original read buffer.
    auto bufcopy = make_unique<char[]>(readBuf_.size());
    memcpy(bufcopy.get(), readBuf_.data(), readBuf_.size());

    // Set up buffers. inbufs will be the raw bytes the library has to decode.
    // outbufs will contain generated responses, if any.
    TLSBuffer inbufs[] = {
        TLSBuffer(SECBUFFER_TOKEN, readBuf_.size(), bufcopy.get()),
        TLSBuffer(SECBUFFER_EMPTY, 0, nullptr),
    };
    TLSBufferDesc indesc(inbufs, 2);
    TLSBuffer outbufs[] = {
        TLSBuffer(SECBUFFER_TOKEN, 0, nullptr),
        TLSBuffer(SECBUFFER_ALERT, 0, nullptr),
    };
    TLSBufferDesc outdesc(outbufs, 2);
    if (side_ == TLS_CLIENT) {
      SEC_CHAR* host = hostname_.empty()
                           ? nullptr
                           : const_cast<SEC_CHAR*>(hostname_.c_str());
      status_ = ::InitializeSecurityContext(cred_, &handle_, host, kReqFlags, 0,
                                            0, &indesc, 0, nullptr, &outdesc,
                                            &flags, nullptr);
    }
    else {
      status_ = ::AcceptSecurityContext(
          cred_, state_ == st_initialized ? nullptr : &handle_, &indesc,
          kReqAFlags, 0, state_ == st_initialized ? &handle_ : nullptr,
          &outdesc, &flags, nullptr);
    }
    if (status_ == SEC_E_INCOMPLETE_MESSAGE) {
      // Not enough raw bytes read yet to decode a full message.
      return TLS_ERR_WOULDBLOCK;
    }
    if (status_ != SEC_E_OK && status_ != SEC_I_CONTINUE_NEEDED) {
      state_ = st_error;
      return TLS_ERR_ERROR;
    }

    // Raw bytes where not entirely consumed, i.e. readBuf_ still contains
    // unprocessed data from the next message?
    if (inbufs[1].BufferType == SECBUFFER_EXTRA && inbufs[1].cbBuffer > 0) {
      readBuf_.eat(readBuf_.size() - inbufs[1].cbBuffer);
    }
    else {
      readBuf_.clear();
    }

    // Check if the library produced a new outgoing message and queue it.
    for (auto& buf : outbufs) {
      if (buf.BufferType == SECBUFFER_TOKEN && buf.cbBuffer > 0) {
        writeBuf_.write(buf.pvBuffer, buf.cbBuffer);
        FreeContextBuffer(buf.pvBuffer);
        state_ = st_handshake_write;
      }
    }

    // Need to read additional messages?
    if (status_ == SEC_I_CONTINUE_NEEDED) {
      A2_LOG_DEBUG("WinTLS: Continuing with handshake");
      goto restart;
    }

    if (side_ == TLS_CLIENT && flags != kReqFlags) {
      A2_LOG_ERROR(fmt("WinTLS: Channel setup failed. Schannel provider did "
                       "not fulfill requested flags. "
                       "Excepted: %lu Actual: %lu",
                       kReqFlags, flags));
      status_ = SEC_E_INTERNAL_ERROR;
      state_ = st_error;
      return TLS_ERR_ERROR;
    }

    if (state_ == st_handshake_write) {
      A2_LOG_DEBUG("WinTLS: Continuing with handshake (last write)");
      state_ = st_handshake_write_last;
      goto restart;
    }
  }
    // Fall through

  case st_handshake_done:
    if (obtainTLSRecordSizes() != 0) {
      return TLS_ERR_ERROR;
    }
    ensureSendBuffer();

    // All ready now :D
    state_ = st_connected;
    A2_LOG_INFO(
        fmt("WinTLS: connected with: %s", getCipherSuite(&handle_).c_str()));
    switch (getProtocolVersion(&handle_)) {
    case 0x302:
      version = TLS_PROTO_TLS11;
      break;
    case 0x303:
      version = TLS_PROTO_TLS12;
      break;
    default:
      assert(0);
      abort();
    }
    return TLS_ERR_OK;
  }

  A2_LOG_ERROR("WinTLS: Unreachable reached during tlsConnect! This is a bug!");
  state_ = st_error;
  return TLS_ERR_ERROR;
}

int WinTLSSession::tlsAccept(TLSVersion& version)
{
  std::string host, err;
  return tlsConnect(host, version, err);
}

std::string WinTLSSession::getLastErrorString()
{
  std::stringstream ss;
  wchar_t* buf = nullptr;
  auto rv = FormatMessageW(
      FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM |
          FORMAT_MESSAGE_IGNORE_INSERTS,
      nullptr, status_, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPWSTR)&buf,
      1024, nullptr);
  if (rv && buf) {
    ss << "Error: " << wCharToUtf8(buf) << "(" << std::hex << status_ << ")";
    LocalFree(buf);
  }
  else {
    ss << "Error: " << std::hex << status_;
  }
  return ss.str();
}

size_t WinTLSSession::getRecvBufferedLength() { return decBuf_.size(); }

int WinTLSSession::obtainTLSRecordSizes()
{
  status_ = ::QueryContextAttributes(&handle_, SECPKG_ATTR_STREAM_SIZES,
                                     &streamSizes_);
  if (status_ != SEC_E_OK || !streamSizes_.cbMaximumMessage) {
    A2_LOG_ERROR("WinTLS: Unable to obtain stream sizes");
    state_ = st_error;
    return -1;
  }

  return 0;
}

void WinTLSSession::ensureSendBuffer()
{
  auto sum = streamSizes_.cbHeader + streamSizes_.cbMaximumMessage +
             streamSizes_.cbTrailer;
  if (sendBuffer_.size() < sum) {
    sendBuffer_.resize(sum);
  }
}

} // namespace aria2
