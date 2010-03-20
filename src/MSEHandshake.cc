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
#include "MSEHandshake.h"

#include <cstring>
#include <cassert>

#include "message.h"
#include "DlAbortEx.h"
#include "LogFactory.h"
#include "Logger.h"
#include "BtHandshakeMessage.h"
#include "Socket.h"
#include "a2netcompat.h"
#include "DHKeyExchange.h"
#include "ARC4Encryptor.h"
#include "ARC4Decryptor.h"
#include "MessageDigestHelper.h"
#include "SimpleRandomizer.h"
#include "util.h"
#include "DownloadContext.h"
#include "prefs.h"
#include "Option.h"
#include "StringFormat.h"
#include "bittorrent_helper.h"

namespace aria2 {

const unsigned char* MSEHandshake::PRIME = reinterpret_cast<const unsigned char*>("FFFFFFFFFFFFFFFFC90FDAA22168C234C4C6628B80DC1CD129024E088A67CC74020BBEA63B139B22514A08798E3404DDEF9519B3CD3A431B302B0A6DF25F14374FE1356D6D51C245E485B576625E7EC6F44C42E9A63A36210000000000090563");

const unsigned char* MSEHandshake::GENERATOR = reinterpret_cast<const unsigned char*>("2");

const unsigned char MSEHandshake::VC[] = { 0, 0, 0, 0, 0, 0, 0, 0 };

MSEHandshake::MSEHandshake(cuid_t cuid,
                           const SocketHandle& socket,
                           const Option* op):
  _cuid(cuid),
  _socket(socket),
  _option(op),
  _logger(LogFactory::getInstance()),
  _rbufLength(0),
  _socketBuffer(socket),
  _negotiatedCryptoType(CRYPTO_NONE),
  _dh(0),
  _initiator(true),
  _markerIndex(0),
  _padLength(0),
  _iaLength(0),
  _ia(0)
{}

MSEHandshake::~MSEHandshake()
{
  delete _dh;
  delete [] _ia;
}

MSEHandshake::HANDSHAKE_TYPE MSEHandshake::identifyHandshakeType()
{
  if(!_socket->isReadable(0)) {
    return HANDSHAKE_NOT_YET;
  }
  size_t r = 20-_rbufLength;
  _socket->readData(_rbuf+_rbufLength, r);
  if(r == 0 && !_socket->wantRead() && !_socket->wantWrite()) {
    throw DL_ABORT_EX(EX_EOF_FROM_PEER);
  }
  _rbufLength += r;
  if(_rbufLength < 20) {
    return HANDSHAKE_NOT_YET;
  }
  if(_rbuf[0] == BtHandshakeMessage::PSTR_LENGTH &&
     memcmp(BtHandshakeMessage::BT_PSTR, _rbuf+1, 19) == 0) {
    if(_logger->debug()) {
      _logger->debug("CUID#%d - This is legacy BitTorrent handshake.", _cuid);
    }
    return HANDSHAKE_LEGACY;
  } else {
    if(_logger->debug()) {
      _logger->debug("CUID#%d - This may be encrypted BitTorrent handshake.",
                     _cuid);
    }
    return HANDSHAKE_ENCRYPTED;
  }
}

void MSEHandshake::initEncryptionFacility(bool initiator)
{
  delete _dh;
  _dh = new DHKeyExchange();
  _dh->init(PRIME, PRIME_BITS, GENERATOR, 160);
  _dh->generatePublicKey();
  if(_logger->debug()) {
    _logger->debug("CUID#%d - DH initialized.", _cuid);
  }
  _initiator = initiator;
}

bool MSEHandshake::sendPublicKey()
{
  if(_socketBuffer.sendBufferIsEmpty()) {
    if(_logger->debug()) {
      _logger->debug("CUID#%d - Sending public key.", _cuid);
    }
    unsigned char buffer[KEY_LENGTH+MAX_PAD_LENGTH];
    _dh->getPublicKey(buffer, KEY_LENGTH);

    size_t padLength = SimpleRandomizer::getInstance()->getRandomNumber(MAX_PAD_LENGTH+1);
    _dh->generateNonce(buffer+KEY_LENGTH, padLength);
    _socketBuffer.pushStr(std::string(&buffer[0],
                                      &buffer[KEY_LENGTH+padLength]));
  }
  _socketBuffer.send();
  return _socketBuffer.sendBufferIsEmpty();
}

bool MSEHandshake::receivePublicKey()
{
  size_t r = KEY_LENGTH-_rbufLength;
  if(r > receiveNBytes(r)) {
    return false;
  }
  if(_logger->debug()) {
    _logger->debug("CUID#%d - public key received.", _cuid);
  }
  // TODO handle exception. in catch, resbufLength = 0;
  _dh->computeSecret(_secret, sizeof(_secret), _rbuf, _rbufLength);
  // reset _rbufLength
  _rbufLength = 0;
  return true;
}

void MSEHandshake::initCipher(const unsigned char* infoHash)
{
  memcpy(_infoHash, infoHash, INFO_HASH_LENGTH);
  //Initialize cipher
  unsigned char s[4+KEY_LENGTH+INFO_HASH_LENGTH];
  memcpy(s, _initiator?"keyA":"keyB", 4);
  memcpy(s+4, _secret, KEY_LENGTH);
  memcpy(s+4+KEY_LENGTH, infoHash, INFO_HASH_LENGTH);
  
  unsigned char localCipherKey[20];
  MessageDigestHelper::digest(localCipherKey, sizeof(localCipherKey),
                              MessageDigestContext::SHA1,
                              s, sizeof(s));
  _encryptor.reset(new ARC4Encryptor());
  _encryptor->init(localCipherKey, sizeof(localCipherKey));

  unsigned char peerCipherKey[20];
  memcpy(s, _initiator?"keyB":"keyA", 4);
  MessageDigestHelper::digest(peerCipherKey, sizeof(peerCipherKey),
                              MessageDigestContext::SHA1,
                              s, sizeof(s));
  _decryptor.reset(new ARC4Decryptor());
  _decryptor->init(peerCipherKey, sizeof(peerCipherKey));

  // discard first 1024 bytes ARC4 output.
  unsigned char from[1024];
  unsigned char to[1024];
  _encryptor->encrypt(to, 1024, from, 1024);
  _decryptor->decrypt(to, 1024, from, 1024);

  if(_initiator) {
    ARC4Encryptor enc;
    enc.init(peerCipherKey, sizeof(peerCipherKey));
    // discard first 1024 bytes ARC4 output.
    enc.encrypt(to, 1024, from, 1024);
    enc.encrypt(_initiatorVCMarker, sizeof(_initiatorVCMarker), VC, sizeof(VC));
  }
}

void MSEHandshake::encryptAndSendData(const unsigned char* data, size_t length)
{
  unsigned char temp[4096];
  const unsigned char* dptr = data;
  size_t s;
  size_t r = length;
  while(r > 0) {
    s = std::min(r, sizeof(temp));
    _encryptor->encrypt(temp, s, dptr, s);
    _socketBuffer.pushStr(std::string(&temp[0], &temp[s]));
    dptr += s;
    r -= s;
  }
}

void MSEHandshake::createReq1Hash(unsigned char* md) const
{
  unsigned char buffer[100];
  memcpy(buffer, "req1", 4);
  memcpy(buffer+4, _secret, KEY_LENGTH);
  MessageDigestHelper::digest(md, 20, MessageDigestContext::SHA1,
                              buffer, 4+KEY_LENGTH);
}

void MSEHandshake::createReq23Hash(unsigned char* md, const unsigned char* infoHash) const
{
  unsigned char x[24];
  memcpy(x, "req2", 4);
  memcpy(x+4, infoHash, INFO_HASH_LENGTH);
  unsigned char xh[20];
  MessageDigestHelper::digest(xh, sizeof(xh), MessageDigestContext::SHA1,
                              x, sizeof(x));

  unsigned char y[4+96];
  memcpy(y, "req3", 4);
  memcpy(y+4, _secret, KEY_LENGTH);
  unsigned char yh[20];
  MessageDigestHelper::digest(yh, sizeof(yh), MessageDigestContext::SHA1,
                              y, sizeof(y));
  
  for(size_t i = 0; i < 20; ++i) {
    md[i] = xh[i]^yh[i];
  }
}

uint16_t MSEHandshake::decodeLength16(const unsigned char* buffer)
{
  uint16_t be;
  _decryptor->decrypt(reinterpret_cast<unsigned char*>(&be),
                      sizeof(be),
                      buffer, sizeof(be));
  return ntohs(be);
}

bool MSEHandshake::sendInitiatorStep2()
{
  if(_socketBuffer.sendBufferIsEmpty()) {
    if(_logger->debug()) {
      _logger->debug("CUID#%d - Sending negotiation step2.", _cuid);
    }
    unsigned char md[20];
    createReq1Hash(md);
    _socketBuffer.pushStr(std::string(&md[0], &md[sizeof(md)]));

    createReq23Hash(md, _infoHash);
    _socketBuffer.pushStr(std::string(&md[0], &md[sizeof(md)]));

    {
      // buffer is filled in this order:
      //   VC(VC_LENGTH bytes),
      //   crypto_provide(CRYPTO_BITFIELD_LENGTH bytes),
      //   len(padC)(2bytes),
      //   padC(len(padC)bytes <= MAX_PAD_LENGTH),
      //   len(IA)(2bytes)
      unsigned char buffer[VC_LENGTH+CRYPTO_BITFIELD_LENGTH+2+MAX_PAD_LENGTH+2];

      // VC
      memcpy(buffer, VC, sizeof(VC));
      // crypto_provide
      unsigned char cryptoProvide[CRYPTO_BITFIELD_LENGTH];
      memset(cryptoProvide, 0, sizeof(cryptoProvide));
      if(_option->get(PREF_BT_MIN_CRYPTO_LEVEL) == V_PLAIN) {
        cryptoProvide[3] = CRYPTO_PLAIN_TEXT;
      }
      cryptoProvide[3] |= CRYPTO_ARC4;
      memcpy(buffer+VC_LENGTH, cryptoProvide, sizeof(cryptoProvide));

      // len(padC)
      uint16_t padCLength = SimpleRandomizer::getInstance()->getRandomNumber(MAX_PAD_LENGTH+1);
      {
        uint16_t padCLengthBE = htons(padCLength);
        memcpy(buffer+VC_LENGTH+CRYPTO_BITFIELD_LENGTH, &padCLengthBE,
               sizeof(padCLengthBE));
      }
      // padC
      memset(buffer+VC_LENGTH+CRYPTO_BITFIELD_LENGTH+2, 0, padCLength);
      // len(IA)
      // currently, IA is zero-length.
      uint16_t iaLength = 0;
      {
        uint16_t iaLengthBE = htons(iaLength);
        memcpy(buffer+VC_LENGTH+CRYPTO_BITFIELD_LENGTH+2+padCLength,
               &iaLengthBE,sizeof(iaLengthBE));
      }
      encryptAndSendData(buffer,
                         VC_LENGTH+CRYPTO_BITFIELD_LENGTH+2+padCLength+2);
    }
  }
  _socketBuffer.send();
  return _socketBuffer.sendBufferIsEmpty();
}

// This function reads exactly until the end of VC marker is reached.
bool MSEHandshake::findInitiatorVCMarker()
{
  // 616 is synchronization point of initiator
  size_t r = 616-KEY_LENGTH-_rbufLength;
  if(!_socket->isReadable(0)) {
    return false;
  }
  _socket->peekData(_rbuf+_rbufLength, r);
  if(r == 0) {
    if(_socket->wantRead() || _socket->wantWrite()) {
      return false;
    }
    throw DL_ABORT_EX(EX_EOF_FROM_PEER);
  }
  // find vc
  {
    std::string buf(&_rbuf[0], &_rbuf[_rbufLength+r]);
    std::string vc(&_initiatorVCMarker[0], &_initiatorVCMarker[VC_LENGTH]);
    if((_markerIndex = buf.find(vc)) == std::string::npos) {
      if(616-KEY_LENGTH <= _rbufLength+r) {
        throw DL_ABORT_EX("Failed to find VC marker.");
      } else {
        _socket->readData(_rbuf+_rbufLength, r);
        _rbufLength += r;
        return false;
      }
    }
  }
  assert(_markerIndex+VC_LENGTH-_rbufLength <= r);
  size_t toRead = _markerIndex+VC_LENGTH-_rbufLength;
  _socket->readData(_rbuf+_rbufLength, toRead);
  _rbufLength += toRead;
  if(_logger->debug()) {
    _logger->debug("CUID#%d - VC marker found at %u", _cuid, _markerIndex);
  }
  verifyVC(_rbuf+_markerIndex);
  // reset _rbufLength
  _rbufLength = 0;
  return true;
}

bool MSEHandshake::receiveInitiatorCryptoSelectAndPadDLength()
{
  size_t r = CRYPTO_BITFIELD_LENGTH+2/* PadD length*/-_rbufLength;
  if(r > receiveNBytes(r)) {
    return false;
  }
  //verifyCryptoSelect
  unsigned char* rbufptr = _rbuf;
  {
    unsigned char cryptoSelect[CRYPTO_BITFIELD_LENGTH];
    _decryptor->decrypt(cryptoSelect, sizeof(cryptoSelect),
                        rbufptr, sizeof(cryptoSelect));
    if(cryptoSelect[3]&CRYPTO_PLAIN_TEXT &&
       _option->get(PREF_BT_MIN_CRYPTO_LEVEL) == V_PLAIN) {
      if(_logger->debug()) {
        _logger->debug("CUID#%d - peer prefers plaintext.", _cuid);
      }
      _negotiatedCryptoType = CRYPTO_PLAIN_TEXT;
    }
    if(cryptoSelect[3]&CRYPTO_ARC4) {
      if(_logger->debug()) {
        _logger->debug("CUID#%d - peer prefers ARC4", _cuid);
      }
      _negotiatedCryptoType = CRYPTO_ARC4;
    }
    if(_negotiatedCryptoType == CRYPTO_NONE) {
      throw DL_ABORT_EX
        (StringFormat("CUID#%d - No supported crypto type selected.", _cuid).str());
    }
  }
  // padD length
  rbufptr += CRYPTO_BITFIELD_LENGTH;
  _padLength = verifyPadLength(rbufptr, "PadD");
  // reset _rbufLength
  _rbufLength = 0;
  return true;
}

bool MSEHandshake::receivePad()
{
  if(_padLength == 0) {
    return true;
  }
  size_t r = _padLength-_rbufLength;
  if(r > receiveNBytes(r)) {
    return false;
  }
  unsigned char temp[MAX_PAD_LENGTH];
  _decryptor->decrypt(temp, _padLength, _rbuf, _padLength);
  // reset _rbufLength
  _rbufLength = 0;
  return true;
}

bool MSEHandshake::findReceiverHashMarker()
{
  // 628 is synchronization limit of receiver.
  size_t r = 628-KEY_LENGTH-_rbufLength;
  if(!_socket->isReadable(0)) {
    return false;
  }
  _socket->peekData(_rbuf+_rbufLength, r);
  if(r == 0) {
    if(_socket->wantRead() || _socket->wantWrite()) {
      return false;
    }
    throw DL_ABORT_EX(EX_EOF_FROM_PEER);
  }
  // find hash('req1', S), S is _secret.
  {
    std::string buf(&_rbuf[0], &_rbuf[_rbufLength+r]);
    unsigned char md[20];
    createReq1Hash(md);
    std::string req1(&md[0], &md[sizeof(md)]);
    if((_markerIndex = buf.find(req1)) == std::string::npos) {
      if(628-KEY_LENGTH <= _rbufLength+r) {
        throw DL_ABORT_EX("Failed to find hash marker.");
      } else {
        _socket->readData(_rbuf+_rbufLength, r);
        _rbufLength += r;
        return false;
      }
    }
  }
  assert(_markerIndex+20-_rbufLength <= r);
  size_t toRead = _markerIndex+20-_rbufLength;
  _socket->readData(_rbuf+_rbufLength, toRead);
  _rbufLength += toRead;
  if(_logger->debug()) {
    _logger->debug("CUID#%d - Hash marker found at %u.", _cuid, _markerIndex);
  }
  verifyReq1Hash(_rbuf+_markerIndex);
  // reset _rbufLength
  _rbufLength = 0;
  return true;
}

bool MSEHandshake::receiveReceiverHashAndPadCLength
(const std::vector<SharedHandle<DownloadContext> >& downloadContexts)
{
  size_t r = 20+VC_LENGTH+CRYPTO_BITFIELD_LENGTH+2/*PadC length*/-_rbufLength;
  if(r > receiveNBytes(r)) {
    return false;
  }
  // resolve info hash
  // pointing to the position of HASH('req2', SKEY) xor HASH('req3', S)
  unsigned char* rbufptr = _rbuf;
  SharedHandle<DownloadContext> downloadContext;
  for(std::vector<SharedHandle<DownloadContext> >::const_iterator i =
        downloadContexts.begin(), eoi = downloadContexts.end();
      i != eoi; ++i) {
    unsigned char md[20];
    const BDE& torrentAttrs = (*i)->getAttribute(bittorrent::BITTORRENT);
    createReq23Hash(md, torrentAttrs[bittorrent::INFO_HASH].uc());
    if(memcmp(md, rbufptr, sizeof(md)) == 0) {
      if(_logger->debug()) {
        _logger->debug("CUID#%d - info hash found: %s", _cuid,
                       util::toHex
                       (torrentAttrs[bittorrent::INFO_HASH].s()).c_str());
      }
      downloadContext = *i;
      break;
    }
  }
  if(downloadContext.isNull()) {
    throw DL_ABORT_EX("Unknown info hash.");
  }
  initCipher(bittorrent::getInfoHash(downloadContext));
  // decrypt VC
  rbufptr += 20;
  verifyVC(rbufptr);
  // decrypt crypto_provide
  rbufptr += VC_LENGTH;
  {
    unsigned char cryptoProvide[CRYPTO_BITFIELD_LENGTH];
    _decryptor->decrypt(cryptoProvide, sizeof(cryptoProvide),
                        rbufptr, sizeof(cryptoProvide));
    // TODO choose the crypto type based on the preference.
    // For now, choose ARC4.
    if(cryptoProvide[3]&CRYPTO_PLAIN_TEXT &&
       _option->get(PREF_BT_MIN_CRYPTO_LEVEL) == V_PLAIN) {
      if(_logger->debug()) {
        _logger->debug("CUID#%d - peer provides plaintext.", _cuid);
      }
      _negotiatedCryptoType = CRYPTO_PLAIN_TEXT;
    } else if(cryptoProvide[3]&CRYPTO_ARC4) {
      if(_logger->debug()) {
        _logger->debug("CUID#%d - peer provides ARC4.", _cuid);
      }
      _negotiatedCryptoType = CRYPTO_ARC4;
    }
    if(_negotiatedCryptoType == CRYPTO_NONE) {
      throw DL_ABORT_EX
        (StringFormat("CUID#%d - No supported crypto type provided.", _cuid).str());
    }
  }
  // decrypt PadC length
  rbufptr += CRYPTO_BITFIELD_LENGTH;
  _padLength = verifyPadLength(rbufptr, "PadC");
  // reset _rbufLength
  _rbufLength = 0;
  return true;
}

bool MSEHandshake::receiveReceiverIALength()
{
  size_t r = 2-_rbufLength;
  assert(r > 0);
  if(r > receiveNBytes(r)) {
    return false;
  }
  _iaLength = decodeLength16(_rbuf);
  if(_logger->debug()) {
    _logger->debug("CUID#%d - len(IA)=%u.", _cuid, _iaLength);
  }
  // reset _rbufLength
  _rbufLength = 0;
  return true;
}

bool MSEHandshake::receiveReceiverIA()
{
  if(_iaLength == 0) {
    return true;
  }
  size_t r = _iaLength-_rbufLength;
  if(r > receiveNBytes(r)) {
    return false;
  }
  delete [] _ia;
  _ia = new unsigned char[_iaLength];
  _decryptor->decrypt(_ia, _iaLength, _rbuf, _iaLength);
  if(_logger->debug()) {
    _logger->debug("CUID#%d - IA received.", _cuid);
  }
  // reset _rbufLength
  _rbufLength = 0;
  return true;
}

bool MSEHandshake::sendReceiverStep2()
{
  if(_socketBuffer.sendBufferIsEmpty()) {
    // buffer is filled in this order:
    //   VC(VC_LENGTH bytes),
    //   cryptoSelect(CRYPTO_BITFIELD_LENGTH bytes),
    //   len(padD)(2bytes),
    //   padD(len(padD)bytes <= MAX_PAD_LENGTH)
    unsigned char buffer[VC_LENGTH+CRYPTO_BITFIELD_LENGTH+2+MAX_PAD_LENGTH];
    // VC
    memcpy(buffer, VC, sizeof(VC));
    // crypto_select
    unsigned char cryptoSelect[CRYPTO_BITFIELD_LENGTH];
    memset(cryptoSelect, 0, sizeof(cryptoSelect));
    cryptoSelect[3] = _negotiatedCryptoType;
    memcpy(buffer+VC_LENGTH, cryptoSelect, sizeof(cryptoSelect));
    // len(padD)
    uint16_t padDLength = SimpleRandomizer::getInstance()->getRandomNumber(MAX_PAD_LENGTH+1);
    {
      uint16_t padDLengthBE = htons(padDLength);
      memcpy(buffer+VC_LENGTH+CRYPTO_BITFIELD_LENGTH, &padDLengthBE,
             sizeof(padDLengthBE));
    }
    // padD, all zeroed
    memset(buffer+VC_LENGTH+CRYPTO_BITFIELD_LENGTH+2, 0, padDLength);
    encryptAndSendData(buffer, VC_LENGTH+CRYPTO_BITFIELD_LENGTH+2+padDLength);
  }
  _socketBuffer.send();
  return _socketBuffer.sendBufferIsEmpty();
}

uint16_t MSEHandshake::verifyPadLength(const unsigned char* padlenbuf, const char* padName)
{
  if(_logger->debug()) {
    _logger->debug("CUID#%d - Verifying Pad length for %s", _cuid, padName);
  }
  uint16_t padLength = decodeLength16(padlenbuf);
  if(_logger->debug()) {
    _logger->debug("CUID#%d - len(%s)=%u", _cuid, padName, padLength);
  }
  if(padLength > 512) {
    throw DL_ABORT_EX
      (StringFormat("Too large %s length: %u", padName, padLength).str());
  }
  return padLength;
}

void MSEHandshake::verifyVC(const unsigned char* vcbuf)
{
  if(_logger->debug()) {
    _logger->debug("CUID#%d - Verifying VC.", _cuid);
  }
  unsigned char vc[VC_LENGTH];
  _decryptor->decrypt(vc, sizeof(vc), vcbuf, sizeof(vc));
  if(memcmp(VC, vc, sizeof(VC)) != 0) {
    throw DL_ABORT_EX
      (StringFormat("Invalid VC: %s", util::toHex(vc, VC_LENGTH).c_str()).str());
  }
}

void MSEHandshake::verifyReq1Hash(const unsigned char* req1buf)
{
  if(_logger->debug()) {
    _logger->debug("CUID#%d - Verifying req hash.", _cuid);
  }
  unsigned char md[20];
  createReq1Hash(md);
  if(memcmp(md, req1buf, sizeof(md)) != 0) {
    throw DL_ABORT_EX("Invalid req1 hash found.");
  }
}

size_t MSEHandshake::receiveNBytes(size_t bytes)
{
  size_t r = bytes;
  if(r > 0) {
    if(!_socket->isReadable(0)) {
      return 0;
    }
    _socket->readData(_rbuf+_rbufLength, r);
    if(r == 0 && !_socket->wantRead() && !_socket->wantWrite()) {
      throw DL_ABORT_EX(EX_EOF_FROM_PEER);
    }
    _rbufLength += r;
  }
  return r;
}

} // namespace aria2
