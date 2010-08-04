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
#ifndef _D_PEER_CONNECTION_H_
#define _D_PEER_CONNECTION_H_

#include "common.h"

#include <unistd.h>

#include "SharedHandle.h"
#include "SocketBuffer.h"
#include "Command.h"

namespace aria2 {

class Logger;
class Peer;
class SocketCore;
class ARC4Encryptor;
class ARC4Decryptor;

// The maximum length of payload. Messages beyond that length are
// dropped.
#define MAX_PAYLOAD_LEN (16*1024+128)

class PeerConnection {
private:
  cuid_t cuid_;
  SharedHandle<Peer> peer_;
  SharedHandle<SocketCore> socket_;
  Logger* logger_;

  unsigned char* resbuf_;
  size_t resbufLength_;
  size_t currentPayloadLength_;
  unsigned char lenbuf_[4];
  size_t lenbufLength_;

  SocketBuffer socketBuffer_;

  bool encryptionEnabled_;
  SharedHandle<ARC4Encryptor> encryptor_;
  SharedHandle<ARC4Decryptor> decryptor_;

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
                        const SharedHandle<ARC4Decryptor>& decryptor);

  void presetBuffer(const unsigned char* data, size_t length);

  bool sendBufferIsEmpty() const;
  
  ssize_t sendPendingData();

  const unsigned char* getBuffer() const
  {
    return resbuf_;
  }

  unsigned char* detachBuffer()
  {
    unsigned char* detachbuf = resbuf_;
    resbuf_ = new unsigned char[MAX_PAYLOAD_LEN];
    return detachbuf;
  }
};

typedef SharedHandle<PeerConnection> PeerConnectionHandle;
typedef WeakHandle<PeerConnection> PeerConnectionWeakHandle;

} // namespace aria2

#endif // _D_PEER_CONNECTION_H_
