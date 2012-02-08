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
#include "fmt.h"
#include "util.h"
#include "Peer.h"

namespace aria2 {

PeerConnection::PeerConnection
(cuid_t cuid, const SharedHandle<Peer>& peer, const SocketHandle& socket)
  : cuid_(cuid),
    peer_(peer),
    socket_(socket),
    maxPayloadLength_(MAX_PAYLOAD_LEN),
    resbuf_(new unsigned char[maxPayloadLength_]),
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

void PeerConnection::pushBytes(unsigned char* data, size_t len)
{
  if(encryptionEnabled_) {
    encryptor_->encrypt(len, data, data);
  }
  socketBuffer_.pushBytes(data, len);
}

bool PeerConnection::receiveMessage(unsigned char* data, size_t& dataLength) {
  if(resbufLength_ == 0 && 4 > lenbufLength_) {
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
    if(payloadLength > maxPayloadLength_) {
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
  if(BtHandshakeMessage::MESSAGE_LENGTH < resbufLength_) {
    throw DL_ABORT_EX
      ("More than BtHandshakeMessage::MESSAGE_LENGTH bytes are buffered.");
  }
  bool retval = true;
  size_t remaining = BtHandshakeMessage::MESSAGE_LENGTH-resbufLength_;
  if(remaining > 0) {
    size_t temp = remaining;
    readData(resbuf_+resbufLength_, remaining, encryptionEnabled_);
    if(remaining == 0 && !socket_->wantRead() && !socket_->wantWrite()) {
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
  socket_->readData(data, length);
  if(encryption) {
    decryptor_->encrypt(length, data, data);
  }
}

void PeerConnection::enableEncryption
(const SharedHandle<ARC4Encryptor>& encryptor,
 const SharedHandle<ARC4Encryptor>& decryptor)
{
  encryptor_ = encryptor;
  decryptor_ = decryptor;

  encryptionEnabled_ = true;
}

void PeerConnection::presetBuffer(const unsigned char* data, size_t length)
{
  size_t nwrite = std::min(maxPayloadLength_, length);
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
  resbuf_ = new unsigned char[maxPayloadLength_];
  return detachbuf;
}

void PeerConnection::reserveBuffer(size_t minSize)
{
  if(maxPayloadLength_ < minSize) {
    maxPayloadLength_ = minSize;
    unsigned char *buf = new unsigned char[maxPayloadLength_];
    memcpy(buf, resbuf_, resbufLength_);
    delete [] resbuf_;
    resbuf_ = buf;
  }
}

} // namespace aria2
