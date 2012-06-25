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
#ifndef D_PEER_CONNECTION_H
#define D_PEER_CONNECTION_H

#include "common.h"

#include <unistd.h>

#include "SharedHandle.h"
#include "SocketBuffer.h"
#include "Command.h"

namespace aria2 {

class Peer;
class SocketCore;
class ARC4Encryptor;

// The maximum length of buffer. If the message length (including 4
// bytes length and payload length) is larger than this value, it is
// dropped.
#define MAX_BUFFER_CAPACITY (16*1024+128)

class PeerConnection {
private:
  cuid_t cuid_;
  SharedHandle<Peer> peer_;
  SharedHandle<SocketCore> socket_;

  int msgState_;
  // The capacity of the buffer resbuf_
  size_t bufferCapacity_;
  // The internal buffer of incoming handshakes and messages
  unsigned char* resbuf_;
  // The number of bytes written in resbuf_
  size_t resbufLength_;
  // The length of message (not handshake) currently receiving
  uint32_t currentPayloadLength_;
  // The number of bytes processed in resbuf_
  size_t resbufOffset_;
  // The offset in resbuf_ where the 4 bytes message length begins
  size_t msgOffset_;

  SocketBuffer socketBuffer_;

  bool encryptionEnabled_;
  SharedHandle<ARC4Encryptor> encryptor_;
  SharedHandle<ARC4Encryptor> decryptor_;

  bool prevPeek_;

  void readData(unsigned char* data, size_t& length, bool encryption);

  ssize_t sendData(const unsigned char* data, size_t length, bool encryption);

public:
  PeerConnection
  (cuid_t cuid,
   const SharedHandle<Peer>& peer,
   const SharedHandle<SocketCore>& socket);

  ~PeerConnection();

  // Pushes data into send buffer. After this call, this object gets
  // ownership of data, so caller must not delete or alter it.
  void pushBytes(unsigned char* data, size_t len);

  void pushStr(const std::string& data);

  bool receiveMessage(unsigned char* data, size_t& dataLength);

  /**
   * Returns true if a handshake message is fully received, otherwise returns
   * false.
   * In both cases, 'msg' is filled with received bytes and the filled length
   * is assigned to 'length'.
   */
  bool receiveHandshake
  (unsigned char* data, size_t& dataLength, bool peek = false);

  void enableEncryption(const SharedHandle<ARC4Encryptor>& encryptor,
                        const SharedHandle<ARC4Encryptor>& decryptor);

  void presetBuffer(const unsigned char* data, size_t length);

  bool sendBufferIsEmpty() const;
  
  ssize_t sendPendingData();

  const unsigned char* getBuffer() const
  {
    return resbuf_;
  }

  size_t getBufferLength() const
  {
    return resbufLength_;
  }

  // Returns the pointer to the message in wire format.  This method
  // must be called after receiveMessage() returned true.
  const unsigned char* getMsgPayloadBuffer() const;

  // Reserves buffer at least minSize. Reallocate memory if current
  // buffer length < minSize
  void reserveBuffer(size_t minSize);

  size_t getBufferCapacity()
  {
    return bufferCapacity_;
  }
};

typedef SharedHandle<PeerConnection> PeerConnectionHandle;

} // namespace aria2

#endif // D_PEER_CONNECTION_H
