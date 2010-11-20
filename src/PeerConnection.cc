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
#include "PeerConnection.h"

#include <cstring>
#include <cassert>
#include <algorithm>

#include "message.h"
#include "DlAbortEx.h"
#include "LogFactory.h"
#include "Logger.h"
#include "BtHandshakeMessage.h"
#include "Socket.h"
#include "a2netcompat.h"
#include "ARC4Encryptor.h"
#include "ARC4Decryptor.h"
#include "fmt.h"
#include "util.h"
#include "Peer.h"

namespace aria2 {

PeerConnection::PeerConnection
(cuid_t cuid, const SharedHandle<Peer>& peer, const SocketHandle& socket)
  : cuid_(cuid),
    peer_(peer),
    socket_(socket),
    resbuf_(new unsigned char[MAX_PAYLOAD_LEN]),
    resbufLength_(0),
    currentPayloadLength_(0),
    lenbufLength_(0),
    socketBuffer_(socket),
    encryptionEnabled_(false),
    prevPeek_(false)
{}

PeerConnection::~PeerConnection()
{
  delete [] resbuf_;
}

void PeerConnection::pushStr(const std::string& data)
{
  if(encryptionEnabled_) {
    const size_t len = data.size();
    unsigned char* chunk = new unsigned char[len];
    try {
      encryptor_->encrypt
        (chunk, len, reinterpret_cast<const unsigned char*>(data.data()), len);
    } catch(RecoverableException& e) {
      delete [] chunk;
      throw;
    }
    socketBuffer_.pushBytes(chunk, len);
  } else {
    socketBuffer_.pushStr(data);
  }
}

void PeerConnection::pushBytes(unsigned char* data, size_t len)
{
  if(encryptionEnabled_) {
    unsigned char* chunk = new unsigned char[len];
    try {
      encryptor_->encrypt(chunk, len, data, len);
    } catch(RecoverableException& e) {
      delete [] data;
      delete [] chunk;
      throw;
    }
    delete [] data;
    socketBuffer_.pushBytes(chunk, len);
  } else {
    socketBuffer_.pushBytes(data, len);
  }
}

bool PeerConnection::receiveMessage(unsigned char* data, size_t& dataLength) {
  if(resbufLength_ == 0 && 4 > lenbufLength_) {
    if(!socket_->isReadable(0)) {
      return false;
    }
    // read payload size, 32bit unsigned integer
    size_t remaining = 4-lenbufLength_;
    size_t temp = remaining;
    readData(lenbuf_+lenbufLength_, remaining, encryptionEnabled_);
    if(remaining == 0) {
      if(socket_->wantRead() || socket_->wantWrite()) {
        return false;
      }
      // we got EOF
      A2_LOG_DEBUG(fmt("CUID#%lld - In PeerConnection::receiveMessage(),"
                       " remain=%lu",
                       cuid_,
                       static_cast<unsigned long>(temp)));
      peer_->setDisconnectedGracefully(true);
      throw DL_ABORT_EX(EX_EOF_FROM_PEER);
    }
    lenbufLength_ += remaining;
    if(4 > lenbufLength_) {
      // still 4-lenbufLength_ bytes to go
      return false;
    }
    uint32_t payloadLength;
    memcpy(&payloadLength, lenbuf_, sizeof(payloadLength));
    payloadLength = ntohl(payloadLength);
    if(payloadLength > MAX_PAYLOAD_LEN) {
      throw DL_ABORT_EX(fmt(EX_TOO_LONG_PAYLOAD, payloadLength));
    }
    currentPayloadLength_ = payloadLength;
  }
  if(!socket_->isReadable(0)) {
    return false;
  }
  // we have currentPayloadLen-resbufLen bytes to read
  size_t remaining = currentPayloadLength_-resbufLength_;
  size_t temp = remaining;
  if(remaining > 0) {
    readData(resbuf_+resbufLength_, remaining, encryptionEnabled_);
    if(remaining == 0) {
      if(socket_->wantRead() || socket_->wantWrite()) {
        return false;
      }
      // we got EOF
      A2_LOG_DEBUG(fmt("CUID#%lld - In PeerConnection::receiveMessage(),"
                       " payloadlen=%lu, remaining=%lu",
                       cuid_,
                       static_cast<unsigned long>(currentPayloadLength_),
                       static_cast<unsigned long>(temp)));
      peer_->setDisconnectedGracefully(true);
      throw DL_ABORT_EX(EX_EOF_FROM_PEER);
    }
    resbufLength_ += remaining;
    if(currentPayloadLength_ > resbufLength_) {
      return false;
    }
  }
  // we got whole payload.
  resbufLength_ = 0;
  lenbufLength_ = 0;
  if(data) {
    memcpy(data, resbuf_, currentPayloadLength_);
  }
  dataLength = currentPayloadLength_;
  return true;
}

bool PeerConnection::receiveHandshake(unsigned char* data, size_t& dataLength,
                                      bool peek) {
  assert(BtHandshakeMessage::MESSAGE_LENGTH >= resbufLength_);
  bool retval = true;
  if(prevPeek_ && !peek && resbufLength_) {
    // We have data in previous peek.
    // There is a chance that socket is readable because of EOF, for example,
    // official bttrack shutdowns socket after sending first 48 bytes of
    // handshake in its NAT checking.
    // So if there are data in resbuf_, return it without checking socket
    // status.
    prevPeek_ = false;
    retval = BtHandshakeMessage::MESSAGE_LENGTH <= resbufLength_;
  } else {
    prevPeek_ = peek;
    size_t remaining = BtHandshakeMessage::MESSAGE_LENGTH-resbufLength_;
    if(remaining > 0 && !socket_->isReadable(0)) {
      dataLength = 0;
      return false;
    }
    if(remaining > 0) {
      size_t temp = remaining;
      readData(resbuf_+resbufLength_, remaining, encryptionEnabled_);
      if(remaining == 0) {
        if(socket_->wantRead() || socket_->wantWrite()) {
          return false;
        }
        // we got EOF
        A2_LOG_DEBUG
          (fmt("CUID#%lld - In PeerConnection::receiveHandshake(), remain=%lu",
               cuid_,
               static_cast<unsigned long>(temp)));
        peer_->setDisconnectedGracefully(true);
        throw DL_ABORT_EX(EX_EOF_FROM_PEER);
      }
      resbufLength_ += remaining;
      if(BtHandshakeMessage::MESSAGE_LENGTH > resbufLength_) {
        retval = false;
      }
    }
  }
  size_t writeLength = std::min(resbufLength_, dataLength);
  memcpy(data, resbuf_, writeLength);
  dataLength = writeLength;
  if(retval && !peek) {
    resbufLength_ = 0;
  }
  return retval;
}

void PeerConnection::readData
(unsigned char* data, size_t& length, bool encryption)
{
  if(encryption) {
    unsigned char temp[MAX_PAYLOAD_LEN];
    assert(MAX_PAYLOAD_LEN >= length);
    socket_->readData(temp, length);
    decryptor_->decrypt(data, length, temp, length);
  } else {
    socket_->readData(data, length);
  }
}

void PeerConnection::enableEncryption
(const SharedHandle<ARC4Encryptor>& encryptor,
 const SharedHandle<ARC4Decryptor>& decryptor)
{
  encryptor_ = encryptor;
  decryptor_ = decryptor;

  encryptionEnabled_ = true;
}

void PeerConnection::presetBuffer(const unsigned char* data, size_t length)
{
  size_t nwrite = std::min((size_t)MAX_PAYLOAD_LEN, length);
  memcpy(resbuf_, data, nwrite);
  resbufLength_ = length;
}

bool PeerConnection::sendBufferIsEmpty() const
{
  return socketBuffer_.sendBufferIsEmpty();
}

ssize_t PeerConnection::sendPendingData()
{
  ssize_t writtenLength = socketBuffer_.send();
  A2_LOG_DEBUG(fmt("sent %ld byte(s).", static_cast<long int>(writtenLength)));
  return writtenLength;
}

unsigned char* PeerConnection::detachBuffer()
{
  unsigned char* detachbuf = resbuf_;
  resbuf_ = new unsigned char[MAX_PAYLOAD_LEN];
  return detachbuf;
}

} // namespace aria2
