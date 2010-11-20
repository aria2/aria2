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
#ifndef D_MSE_HANDSHAKE_H
#define D_MSE_HANDSHAKE_H

#include "common.h"

#include <vector>

#include "SharedHandle.h"
#include "BtConstants.h"
#include "SocketBuffer.h"
#include "Command.h"

namespace aria2 {

class Option;
class SocketCore;
class DHKeyExchange;
class ARC4Encryptor;
class ARC4Decryptor;
class DownloadContext;
class MessageDigest;

class MSEHandshake {
public:
  enum HANDSHAKE_TYPE {
    HANDSHAKE_NOT_YET = 0,
    HANDSHAKE_LEGACY,
    HANDSHAKE_ENCRYPTED
  };

  enum CRYPTO_TYPE {
    CRYPTO_NONE = 0,
    CRYPTO_PLAIN_TEXT = 0x01u,
    CRYPTO_ARC4 = 0x02u
  };

private:
  static const size_t PRIME_BITS = 768;
  
  static const size_t KEY_LENGTH = (PRIME_BITS+7)/8;
  
  static const size_t MAX_PAD_LENGTH = 512;

  static const size_t VC_LENGTH = 8;

  static const size_t CRYPTO_BITFIELD_LENGTH = 4;

  static const size_t MAX_BUFFER_LENGTH = 6*1024;

  cuid_t cuid_;
  SharedHandle<SocketCore> socket_;
  const Option* option_;

  unsigned char rbuf_[MAX_BUFFER_LENGTH];
  size_t rbufLength_;

  SocketBuffer socketBuffer_;

  CRYPTO_TYPE negotiatedCryptoType_;
  DHKeyExchange* dh_;
  SharedHandle<ARC4Encryptor> encryptor_;
  SharedHandle<ARC4Decryptor> decryptor_;
  unsigned char infoHash_[INFO_HASH_LENGTH];
  unsigned char secret_[KEY_LENGTH];
  bool initiator_;
  unsigned char initiatorVCMarker_[VC_LENGTH];
  size_t markerIndex_;
  uint16_t padLength_;
  uint16_t iaLength_;
  unsigned char* ia_;
  SharedHandle<MessageDigest> sha1_;

  static const unsigned char* PRIME;

  static const unsigned char* GENERATOR;

  static const unsigned char VC[VC_LENGTH];

  void encryptAndSendData(const unsigned char* data, size_t length);

  void createReq1Hash(unsigned char* md) const;

  void createReq23Hash(unsigned char* md, const unsigned char* infoHash) const;

  uint16_t decodeLength16(const unsigned char* buffer);

  uint16_t decodeLength16(const char* buffer)
  {
    return decodeLength16(reinterpret_cast<const unsigned char*>(buffer));
  }

  uint16_t verifyPadLength(const unsigned char* padlenbuf,
                           const char* padName);

  void verifyVC(const unsigned char* vcbuf);

  void verifyReq1Hash(const unsigned char* req1buf);

  size_t receiveNBytes(size_t bytes);

public:
  MSEHandshake(cuid_t cuid, const SharedHandle<SocketCore>& socket,
               const Option* op);

  ~MSEHandshake();
  
  HANDSHAKE_TYPE identifyHandshakeType();

  void initEncryptionFacility(bool initiator);

  bool sendPublicKey();

  bool receivePublicKey();

  void initCipher(const unsigned char* infoHash);

  bool sendInitiatorStep2();

  bool findInitiatorVCMarker();

  bool receiveInitiatorCryptoSelectAndPadDLength();

  bool receivePad();

  bool findReceiverHashMarker();

  bool receiveReceiverHashAndPadCLength
  (const std::vector<SharedHandle<DownloadContext> >& downloadContexts);

  bool receiveReceiverIALength();

  bool receiveReceiverIA();

  bool sendReceiverStep2();

  // returns plain text IA
  const unsigned char* getIA() const
  {
    return ia_;
  }

  size_t getIALength() const
  {
    return iaLength_;
  }

  const unsigned char* getInfoHash() const
  {
    return infoHash_;
  }

  CRYPTO_TYPE getNegotiatedCryptoType() const
  {
    return negotiatedCryptoType_;
  }

  const SharedHandle<ARC4Encryptor>& getEncryptor() const
  {
    return encryptor_;
  }

  const SharedHandle<ARC4Decryptor>& getDecryptor() const
  {
    return decryptor_;
  }

  const unsigned char* getBuffer() const
  {
    return rbuf_;
  }

  size_t getBufferLength() const
  {
    return rbufLength_;
  }

};

} // namespace aria2

#endif // D_MSE_HANDSHAKE_H
