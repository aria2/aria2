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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
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
#include "Util.h"
#include "BtRegistry.h"
#include "BtContext.h"
#include "prefs.h"
#include "Option.h"
#include <cstring>
#include <cassert>

namespace aria2 {

const unsigned char* MSEHandshake::PRIME = reinterpret_cast<const unsigned char*>("FFFFFFFFFFFFFFFFC90FDAA22168C234C4C6628B80DC1CD129024E088A67CC74020BBEA63B139B22514A08798E3404DDEF9519B3CD3A431B302B0A6DF25F14374FE1356D6D51C245E485B576625E7EC6F44C42E9A63A36210000000000090563");

const unsigned char* MSEHandshake::GENERATOR = reinterpret_cast<const unsigned char*>("2");

const unsigned char MSEHandshake::VC[] = { 0, 0, 0, 0, 0, 0, 0, 0 };

MSEHandshake::MSEHandshake(int32_t cuid,
			   const SocketHandle& socket,
			   const Option* op):
  _cuid(cuid),
  _socket(socket),
  _option(op),
  _logger(LogFactory::getInstance()),
  _rbufLength(0),
  _negotiatedCryptoType(CRYPTO_NONE),
  _dh(0),
  _encryptor(0),
  _decryptor(0),
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
  int32_t bufLength = 20;
  _socket->peekData(_rbuf, bufLength);
  if(bufLength != 20) {
    return HANDSHAKE_NOT_YET;
  }
  if(_rbuf[0] == BtHandshakeMessage::PSTR_LENGTH &&
     memcmp(BtHandshakeMessage::BT_PSTR, _rbuf+1, 19) == 0) {
    _logger->debug("CUID#%d - This is legacy BitTorrent handshake.", _cuid);
    return HANDSHAKE_LEGACY;
  } else {
    _logger->debug("CUID#%d - This may be encrypted BitTorrent handshake.", _cuid);
    return HANDSHAKE_ENCRYPTED;
  }
}

void MSEHandshake::initEncryptionFacility(bool initiator)
{
  delete _dh;
  _dh = new DHKeyExchange();
  _dh->init(PRIME, PRIME_BITS, GENERATOR, 160);
  _dh->generatePublicKey();
  _logger->debug("CUID#%d - DH initialized.", _cuid);

  _initiator = initiator;
}

void MSEHandshake::sendPublicKey()
{
  _logger->debug("CUID#%d - Sending public key.", _cuid);
  unsigned char buffer[KEY_LENGTH+MAX_PAD_LENGTH];
  _dh->getPublicKey(buffer, KEY_LENGTH);

  size_t padLength = SimpleRandomizer::getInstance()->getRandomNumber(MAX_PAD_LENGTH+1);
  _dh->generateNonce(buffer+KEY_LENGTH, padLength);
  _socket->writeData(buffer, KEY_LENGTH+padLength);
}

bool MSEHandshake::receivePublicKey()
{
  int32_t r = KEY_LENGTH-_rbufLength;
  if(r > receiveNBytes(r)) {
    return false;
  }
  _logger->debug("CUID#%d - public key received.", _cuid);
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
  MessageDigestHelper::digest(localCipherKey, sizeof(localCipherKey), "sha1",
			      s, sizeof(s));
  _encryptor = new ARC4Encryptor();
  _encryptor->init(localCipherKey, sizeof(localCipherKey));

  unsigned char peerCipherKey[20];
  memcpy(s, _initiator?"keyB":"keyA", 4);
  MessageDigestHelper::digest(peerCipherKey, sizeof(peerCipherKey), "sha1",
			      s, sizeof(s));
  _decryptor = new ARC4Decryptor();
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
    enc.encrypt(_initiatorVCMarker, sizeof(_initiatorVCMarker),	VC, sizeof(VC));
  }
}

ssize_t MSEHandshake::readDataAndDecrypt(unsigned char* data, size_t length)
{
  unsigned char temp[MAX_BUFFER_LENGTH];
  assert(MAX_BUFFER_LENGTH >= length);
  int32_t rlength = length;
  _socket->readData(temp, rlength);
  _decryptor->decrypt(data, rlength, temp, rlength);
  return rlength;
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
    _socket->writeData(temp, s);
    dptr += s;
    r -= s;
  }
}

void MSEHandshake::createReq1Hash(unsigned char* md) const
{
  unsigned char buffer[100];
  memcpy(buffer, "req1", 4);
  memcpy(buffer+4, _secret, KEY_LENGTH);
  MessageDigestHelper::digest(md, 20, "sha1", buffer, 4+KEY_LENGTH);
}

void MSEHandshake::createReq23Hash(unsigned char* md, const unsigned char* infoHash) const
{
  unsigned char x[24];
  memcpy(x, "req2", 4);
  memcpy(x+4, infoHash, INFO_HASH_LENGTH);
  unsigned char xh[20];
  MessageDigestHelper::digest(xh, sizeof(xh), "sha1", x, sizeof(x));

  unsigned char y[4+96];
  memcpy(y, "req3", 4);
  memcpy(y+4, _secret, KEY_LENGTH);
  unsigned char yh[20];
  MessageDigestHelper::digest(yh, sizeof(yh), "sha1", y, sizeof(y));
  
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

void MSEHandshake::sendInitiatorStep2()
{
  _logger->debug("CUID#%d - Sending negotiation step2.", _cuid);
  unsigned char md[20];
  createReq1Hash(md);
  _socket->writeData(md, sizeof(md));

  createReq23Hash(md, _infoHash);
  _socket->writeData(md, sizeof(md));

  {
    unsigned char buffer[8+4+2+MAX_PAD_LENGTH+2];

    // VC
    memcpy(buffer, VC, sizeof(VC));
    // crypto_provide
    unsigned char cryptoProvide[4];
    memset(cryptoProvide, 0, sizeof(cryptoProvide));
    if(_option->get(PREF_BT_MIN_CRYPTO_LEVEL) == V_PLAIN) {
      cryptoProvide[3] = CRYPTO_PLAIN_TEXT;
    }
    cryptoProvide[3] |= CRYPTO_ARC4;
    memcpy(buffer+8, cryptoProvide, sizeof(cryptoProvide));

    // len(padC)
    uint16_t padCLength = SimpleRandomizer::getInstance()->getRandomNumber(MAX_PAD_LENGTH+1);
    {
      uint16_t padCLengthBE = htons(padCLength);
      memcpy(buffer+8+4, &padCLengthBE, sizeof(padCLengthBE));
    }
    // padC
    memset(buffer+8+4+2, 0, padCLength);
    // len(IA)
    // currently, IA is zero-length.
    uint16_t iaLength = 0;
    {
      uint16_t iaLengthBE = htons(iaLength);
      memcpy(buffer+8+4+2+padCLength, &iaLengthBE, sizeof(iaLengthBE));
    }
    encryptAndSendData(buffer, 8+4+2+padCLength+2);
  }
}

// This function reads exactly until the end of VC marker is reached.
bool MSEHandshake::findInitiatorVCMarker()
{
  // 616 is synchronization point of initiator
  int32_t r = 616-KEY_LENGTH-_rbufLength;
  if(!_socket->isReadable(0)) {
    return false;
  }
  _socket->peekData(_rbuf+_rbufLength, r);
  if(r == 0) {
    throw new DlAbortEx(EX_EOF_FROM_PEER);
  }
  // find vc
  {
    std::string buf(&_rbuf[0], &_rbuf[_rbufLength+r]);
    std::string vc(&_initiatorVCMarker[0], &_initiatorVCMarker[VC_LENGTH]);
    if((_markerIndex = buf.find(vc)) == std::string::npos) {
      if(616-KEY_LENGTH <= _rbufLength+r) {
	throw new DlAbortEx("Failed to find VC marker.");
      } else {
	_socket->readData(_rbuf+_rbufLength, r);
	_rbufLength += r;
	return false;
      }
    }
  }
  assert(_markerIndex+VC_LENGTH-_rbufLength <= (size_t)r);
  int32_t toRead = _markerIndex+VC_LENGTH-_rbufLength;
  _socket->readData(_rbuf+_rbufLength, toRead);
  _rbufLength += toRead;

  _logger->debug("CUID#%d - VC marker found at %u", _cuid, _markerIndex);
  verifyVC(_rbuf+_markerIndex);
  // reset _rbufLength
  _rbufLength = 0;
  return true;
}

bool MSEHandshake::receiveInitiatorCryptoSelectAndPadDLength()
{
  int32_t r = CRYPTO_BITFIELD_LENGTH+2/* PadD length*/-_rbufLength;
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
      _logger->debug("CUID#%d - peer prefers plaintext.", _cuid);
      _negotiatedCryptoType = CRYPTO_PLAIN_TEXT;
    }
    if(cryptoSelect[3]&CRYPTO_ARC4) {
      _logger->debug("CUID#%d - peer prefers ARC4", _cuid);
      _negotiatedCryptoType = CRYPTO_ARC4;
    }
    if(_negotiatedCryptoType == CRYPTO_NONE) {
      throw new DlAbortEx("CUID#%d - No supported crypto type selected.", _cuid);
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
  int32_t r = _padLength-_rbufLength;
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
  int32_t r = 628-KEY_LENGTH-_rbufLength;
  if(!_socket->isReadable(0)) {
    return false;
  }
  _socket->peekData(_rbuf+_rbufLength, r);
  if(r == 0) {
    throw new DlAbortEx(EX_EOF_FROM_PEER);
  }
  // find hash('req1', S), S is _secret.
  {
    std::string buf(&_rbuf[0], &_rbuf[_rbufLength+r]);
    unsigned char md[20];
    createReq1Hash(md);
    std::string req1(&md[0], &md[sizeof(md)]);
    if((_markerIndex = buf.find(req1)) == std::string::npos) {
      if(628-KEY_LENGTH <= _rbufLength+r) {
	throw new DlAbortEx("Failed to find hash marker.");
      } else {
	_socket->readData(_rbuf+_rbufLength, r);
	_rbufLength += r;
	return false;
      }
    }
  }
  assert(_markerIndex+20-_rbufLength <= (size_t)r);
  int32_t toRead = _markerIndex+20-_rbufLength;
  _socket->readData(_rbuf+_rbufLength, toRead);
  _rbufLength += toRead;

  _logger->debug("CUID#%d - Hash marker found at %u.", _cuid, _markerIndex);
  verifyReq1Hash(_rbuf+_markerIndex);
  // reset _rbufLength
  _rbufLength = 0;
  return true;
}

bool MSEHandshake::receiveReceiverHashAndPadCLength()
{
  int32_t r = 20+VC_LENGTH+CRYPTO_BITFIELD_LENGTH+2/*PadC length*/-_rbufLength;
  if(r > receiveNBytes(r)) {
    return false;
  }
  // resolve info hash
  std::deque<SharedHandle<BtContext> > btContexts = BtRegistry::getAllBtContext();
  // pointing to the position of HASH('req2', SKEY) xor HASH('req3', S)
  unsigned char* rbufptr = _rbuf;
  SharedHandle<BtContext> btContext = 0;
  for(std::deque<SharedHandle<BtContext> >::const_iterator i = btContexts.begin();
      i != btContexts.end(); ++i) {
    unsigned char md[20];
    createReq23Hash(md, (*i)->getInfoHash());
    if(memcmp(md, rbufptr, sizeof(md)) == 0) {
      _logger->debug("CUID#%d - info hash found: %s", _cuid, (*i)->getInfoHashAsString().c_str());
      btContext = *i;
      break;
    }
  }
  if(btContext.isNull()) {
    throw new DlAbortEx("Unknown info hash.");
  }
  initCipher(btContext->getInfoHash());

  // decrypt VC
  rbufptr += 20;
  verifyVC(rbufptr);
  // decrypt crypto_provide
  rbufptr += VC_LENGTH;
  {
    unsigned char cryptoProvide[4];
    _decryptor->decrypt(cryptoProvide, sizeof(cryptoProvide),
			 rbufptr, sizeof(cryptoProvide));
    // TODO choose the crypto type based on the preference.
    // For now, choose ARC4.
    if(cryptoProvide[3]&CRYPTO_PLAIN_TEXT &&
       _option->get(PREF_BT_MIN_CRYPTO_LEVEL) == V_PLAIN) {
      _logger->debug("CUID#%d - peer provides plaintext.", _cuid);
      _negotiatedCryptoType = CRYPTO_PLAIN_TEXT;
    } else if(cryptoProvide[3]&CRYPTO_ARC4) {
      _logger->debug("CUID#%d - peer provides ARC4.", _cuid);
      _negotiatedCryptoType = CRYPTO_ARC4;
    }
    if(_negotiatedCryptoType == CRYPTO_NONE) {
      throw new DlAbortEx("CUID#%d - No supported crypto type provided.", _cuid);
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
  int32_t r = 2-_rbufLength;
  assert(r > 0);
  if(r > receiveNBytes(r)) {
    return false;
  }
  _iaLength = decodeLength16(_rbuf);
  _logger->debug("CUID#%d - len(IA)=%u.", _cuid, _iaLength);
  // reset _rbufLength
  _rbufLength = 0;
  return true;
}

bool MSEHandshake::receiveReceiverIA()
{
  if(_iaLength == 0) {
    return true;
  }
  int32_t r = _iaLength-_rbufLength;
  if(r > receiveNBytes(r)) {
    return false;
  }
  delete [] _ia;
  _ia = new unsigned char[_iaLength];
  _decryptor->decrypt(_ia, _iaLength, _rbuf, _iaLength);
  _logger->debug("CUID#%d - IA received.", _cuid);
  // reset _rbufLength
  _rbufLength = 0;
  return true;
}

void MSEHandshake::sendReceiverStep2()
{
  unsigned char buffer[8+4+2+MAX_PAD_LENGTH];
  // VC
  memcpy(buffer, VC, sizeof(VC));
  // crypto_select
  unsigned char cryptoSelect[4];
  memset(cryptoSelect, 0, sizeof(cryptoSelect));
  cryptoSelect[3] = _negotiatedCryptoType;
  memcpy(buffer+8, cryptoSelect, sizeof(cryptoSelect));
  // len(padD)
  uint16_t padDLength = SimpleRandomizer::getInstance()->getRandomNumber(MAX_PAD_LENGTH+1);
  {
    uint16_t padDLengthBE = htons(padDLength);
    memcpy(buffer+8+4, &padDLengthBE, sizeof(padDLengthBE));
  }
  // padD, all zeroed
  memset(buffer+8+4+2, 0, padDLength);
  encryptAndSendData(buffer, 8+4+2+padDLength);
}

uint16_t MSEHandshake::verifyPadLength(const unsigned char* padlenbuf, const std::string& padName)
{
  _logger->debug("CUID#%d - Veryfying Pad length for %s", _cuid, padName.c_str());

  uint16_t padLength = decodeLength16(padlenbuf);
  _logger->debug("CUID#%d - len(%s)=%u", _cuid, padName.c_str(), padLength);
  if(padLength > 512) {
    throw new DlAbortEx("Too large %s length: %u", padName.c_str(), padLength);
  }
  return padLength;
}

void MSEHandshake::verifyVC(const unsigned char* vcbuf)
{
  _logger->debug("CUID#%d - Veryfying VC.", _cuid);
  unsigned char vc[VC_LENGTH];
  _decryptor->decrypt(vc, sizeof(vc), vcbuf, sizeof(vc));
  if(memcmp(VC, vc, sizeof(VC)) != 0) {
    throw new DlAbortEx("Invalid VC: %s", Util::toHex(vc, VC_LENGTH).c_str());
  }
}

void MSEHandshake::verifyReq1Hash(const unsigned char* req1buf)
{
  _logger->debug("CUID#%d - Verifying req hash.", _cuid);
  unsigned char md[20];
  createReq1Hash(md);
  if(memcmp(md, req1buf, sizeof(md)) != 0) {
    throw new DlAbortEx("Invalid req1 hash found.");
  }
}

ssize_t MSEHandshake::receiveNBytes(size_t bytes)
{
  int32_t r = bytes;
  if(r > 0) {
    if(!_socket->isReadable(0)) {
      return 0;
    }
    _socket->readData(_rbuf+_rbufLength, r);
    if(r == 0) {
      throw new DlAbortEx(EX_EOF_FROM_PEER);
    }
    _rbufLength += r;
  }
  return r;
}

const unsigned char* MSEHandshake::getIA() const
{
  return _ia;
}

size_t MSEHandshake::getIALength() const
{
  return _iaLength;
}

const unsigned char* MSEHandshake::getInfoHash() const
{
  return _infoHash;
}

MSEHandshake::CRYPTO_TYPE MSEHandshake::getNegotiatedCryptoType() const
{
  return _negotiatedCryptoType;
}

SharedHandle<ARC4Encryptor> MSEHandshake::getEncryptor() const
{
  return _encryptor;
}

SharedHandle<ARC4Decryptor> MSEHandshake::getDecryptor() const
{
  return _decryptor;
}

} // namespace aria2
