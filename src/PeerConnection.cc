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
#include "SocketCore.h"
#include "a2netcompat.h"
#include "ARC4Encryptor.h"
#include "fmt.h"
#include "util.h"
#include "Peer.h"

namespace aria2 {

namespace {
enum {
  // Before reading first byte of message length
  BT_MSG_PREV_READ_LENGTH,
  // Reading 4 bytes message length
  BT_MSG_READ_LENGTH,
  // Reading message payload following message length
  BT_MSG_READ_PAYLOAD
};
} // namespace

PeerConnection::PeerConnection(cuid_t cuid, const std::shared_ptr<Peer>& peer,
                               const std::shared_ptr<SocketCore>& socket)
    : cuid_(cuid),
      peer_(peer),
      socket_(socket),
      msgState_(BT_MSG_PREV_READ_LENGTH),
      bufferCapacity_(MAX_BUFFER_CAPACITY),
      resbuf_(make_unique<unsigned char[]>(bufferCapacity_)),
      resbufLength_(0),
      currentPayloadLength_(0),
      resbufOffset_(0),
      msgOffset_(0),
      socketBuffer_(socket),
      encryptionEnabled_(false),
      prevPeek_(false)
{
}

PeerConnection::~PeerConnection() = default;

void PeerConnection::pushBytes(std::vector<unsigned char> data,
                               std::unique_ptr<ProgressUpdate> progressUpdate)
{
  if (encryptionEnabled_) {
    encryptor_->encrypt(data.size(), data.data(), data.data());
  }
  socketBuffer_.pushBytes(std::move(data), std::move(progressUpdate));
}

bool PeerConnection::receiveMessage(unsigned char* data, size_t& dataLength)
{
  while (1) {
    bool done = false;
    size_t i;
    for (i = resbufOffset_; i < resbufLength_ && !done; ++i) {
      unsigned char c = resbuf_[i];
      switch (msgState_) {
      case (BT_MSG_PREV_READ_LENGTH):
        msgOffset_ = i;
        currentPayloadLength_ = 0;
        msgState_ = BT_MSG_READ_LENGTH;
      // Fall through
      case (BT_MSG_READ_LENGTH):
        currentPayloadLength_ <<= 8;
        currentPayloadLength_ += c;
        // The message length is uint32_t
        if (i - msgOffset_ == 3) {
          if (currentPayloadLength_ + 4 > bufferCapacity_) {
            throw DL_ABORT_EX(fmt(EX_TOO_LONG_PAYLOAD, currentPayloadLength_));
          }
          if (currentPayloadLength_ == 0) {
            // Length == 0 means keep-alive message.
            done = true;
            msgState_ = BT_MSG_PREV_READ_LENGTH;
          }
          else {
            msgState_ = BT_MSG_READ_PAYLOAD;
          }
        }
        break;
      case (BT_MSG_READ_PAYLOAD):
        // We chose the bufferCapacity_ so that whole message,
        // including 4 bytes length and payload, in it. So here we
        // just make sure that it happens.
        if (resbufLength_ - msgOffset_ >= 4 + currentPayloadLength_) {
          i = msgOffset_ + 4 + currentPayloadLength_ - 1;
          done = true;
          msgState_ = BT_MSG_PREV_READ_LENGTH;
        }
        else {
          // We need another read.
          i = resbufLength_ - 1;
        }
        break;
      }
    }
    resbufOffset_ = i;
    if (done) {
      if (data) {
        std::copy_n(resbuf_.get() + msgOffset_ + 4, currentPayloadLength_,
                    data);
      }
      dataLength = currentPayloadLength_;
      return true;
    }
    else {
      assert(resbufOffset_ == resbufLength_);
      if (resbufLength_ != 0) {
        if (resbufLength_ - msgOffset_ == currentPayloadLength_ + 4) {
          // All bytes in buffer have been processed, so clear it
          // away.
          resbufLength_ = 0;
          resbufOffset_ = 0;
          msgOffset_ = 0;
        }
        else {
          // Shift buffer so that resbuf_[msgOffset_] moves to
          // rebuf_[0].
          memmove(resbuf_.get(), resbuf_.get() + msgOffset_,
                  resbufLength_ - msgOffset_);
          resbufLength_ -= msgOffset_;
          resbufOffset_ = resbufLength_;
          msgOffset_ = 0;
        }
      }
      size_t nread;
      // To reduce the amount of copy involved in buffer shift, large
      // payload will be read exactly.
      if (currentPayloadLength_ > 4_k) {
        nread = currentPayloadLength_ + 4 - resbufLength_;
      }
      else {
        nread = bufferCapacity_ - resbufLength_;
      }
      readData(resbuf_.get() + resbufLength_, nread, encryptionEnabled_);
      if (nread == 0) {
        if (socket_->wantRead() || socket_->wantWrite()) {
          break;
        }
        else {
          peer_->setDisconnectedGracefully(true);
          throw DL_ABORT_EX(EX_EOF_FROM_PEER);
        }
      }
      else {
        resbufLength_ += nread;
      }
    }
  }
  return false;
}

bool PeerConnection::receiveHandshake(unsigned char* data, size_t& dataLength,
                                      bool peek)
{
  if (BtHandshakeMessage::MESSAGE_LENGTH < resbufLength_) {
    throw DL_ABORT_EX(
        "More than BtHandshakeMessage::MESSAGE_LENGTH bytes are buffered.");
  }
  bool retval = true;
  size_t remaining = BtHandshakeMessage::MESSAGE_LENGTH - resbufLength_;
  if (remaining > 0) {
    size_t temp = remaining;
    readData(resbuf_.get() + resbufLength_, remaining, encryptionEnabled_);
    if (remaining == 0 && !socket_->wantRead() && !socket_->wantWrite()) {
      // we got EOF
      A2_LOG_DEBUG(fmt("CUID#%" PRId64
                       " - In PeerConnection::receiveHandshake(), remain=%lu",
                       cuid_, static_cast<unsigned long>(temp)));
      peer_->setDisconnectedGracefully(true);
      throw DL_ABORT_EX(EX_EOF_FROM_PEER);
    }
    resbufLength_ += remaining;
    if (BtHandshakeMessage::MESSAGE_LENGTH > resbufLength_) {
      retval = false;
    }
  }
  size_t writeLength = std::min(resbufLength_, dataLength);
  std::copy_n(resbuf_.get(), writeLength, data);
  dataLength = writeLength;
  if (retval && !peek) {
    resbufLength_ = 0;
  }
  return retval;
}

void PeerConnection::readData(unsigned char* data, size_t& length,
                              bool encryption)
{
  socket_->readData(data, length);
  if (encryption) {
    decryptor_->encrypt(length, data, data);
  }
}

void PeerConnection::enableEncryption(std::unique_ptr<ARC4Encryptor> encryptor,
                                      std::unique_ptr<ARC4Encryptor> decryptor)
{
  encryptor_ = std::move(encryptor);
  decryptor_ = std::move(decryptor);

  encryptionEnabled_ = true;
}

void PeerConnection::presetBuffer(const unsigned char* data, size_t length)
{
  size_t nwrite = std::min(bufferCapacity_, length);
  std::copy_n(data, nwrite, resbuf_.get());
  resbufLength_ = length;
}

bool PeerConnection::sendBufferIsEmpty() const
{
  return socketBuffer_.sendBufferIsEmpty();
}

size_t PeerConnection::getBufferEntrySize() const
{
  return socketBuffer_.getBufferEntrySize();
}

ssize_t PeerConnection::sendPendingData()
{
  ssize_t writtenLength = socketBuffer_.send();
  A2_LOG_DEBUG(fmt("sent %ld byte(s).", static_cast<long int>(writtenLength)));
  return writtenLength;
}

const unsigned char* PeerConnection::getMsgPayloadBuffer() const
{
  return resbuf_.get() + msgOffset_ + 4;
}

void PeerConnection::reserveBuffer(size_t minSize)
{
  if (bufferCapacity_ < minSize) {
    bufferCapacity_ = minSize;
    auto buf = make_unique<unsigned char[]>(bufferCapacity_);
    std::copy_n(resbuf_.get(), resbufLength_, buf.get());
    resbuf_ = std::move(buf);
  }
}

} // namespace aria2
