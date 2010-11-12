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
#include "MessageDigest.h"
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
  cuid_(cuid),
  socket_(socket),
  option_(op),
  logger_(LogFactory::getInstance()),
  rbufLength_(0),
  socketBuffer_(socket),
  negotiatedCryptoType_(CRYPTO_NONE),
  dh_(0),
  initiator_(true),
  markerIndex_(0),
  padLength_(0),
  iaLength_(0),
  ia_(0),
  sha1_(MessageDigest::sha1())
{}

MSEHandshake::~MSEHandshake()
{
  delete dh_;
  delete [] ia_;
}

MSEHandshake::HANDSHAKE_TYPE MSEHandshake::identifyHandshakeType()
{
  if(!socket_->isReadable(0)) {
    return HANDSHAKE_NOT_YET;
  }
  size_t r = 20-rbufLength_;
  socket_->readData(rbuf_+rbufLength_, r);
  if(r == 0 && !socket_->wantRead() && !socket_->wantWrite()) {
    throw DL_ABORT_EX(EX_EOF_FROM_PEER);
  }
  rbufLength_ += r;
  if(rbufLength_ < 20) {
    return HANDSHAKE_NOT_YET;
  }
  if(rbuf_[0] == BtHandshakeMessage::PSTR_LENGTH &&
     memcmp(BtHandshakeMessage::BT_PSTR, rbuf_+1, 19) == 0) {
    if(logger_->debug()) {
      logger_->debug("CUID#%s - This is legacy BitTorrent handshake.",
                     util::itos(cuid_).c_str());
    }
    return HANDSHAKE_LEGACY;
  } else {
    if(logger_->debug()) {
      logger_->debug("CUID#%s - This may be encrypted BitTorrent handshake.",
                     util::itos(cuid_).c_str());
    }
    return HANDSHAKE_ENCRYPTED;
  }
}

void MSEHandshake::initEncryptionFacility(bool initiator)
{
  delete dh_;
  dh_ = new DHKeyExchange();
  dh_->init(PRIME, PRIME_BITS, GENERATOR, 160);
  dh_->generatePublicKey();
  if(logger_->debug()) {
    logger_->debug("CUID#%s - DH initialized.", util::itos(cuid_).c_str());
  }
  initiator_ = initiator;
}

bool MSEHandshake::sendPublicKey()
{
  if(socketBuffer_.sendBufferIsEmpty()) {
    if(logger_->debug()) {
      logger_->debug("CUID#%s - Sending public key.",
                     util::itos(cuid_).c_str());
    }
    unsigned char buffer[KEY_LENGTH+MAX_PAD_LENGTH];
    dh_->getPublicKey(buffer, KEY_LENGTH);

    size_t padLength = SimpleRandomizer::getInstance()->getRandomNumber(MAX_PAD_LENGTH+1);
    dh_->generateNonce(buffer+KEY_LENGTH, padLength);
    socketBuffer_.pushStr(std::string(&buffer[0],
                                      &buffer[KEY_LENGTH+padLength]));
  }
  socketBuffer_.send();
  return socketBuffer_.sendBufferIsEmpty();
}

bool MSEHandshake::receivePublicKey()
{
  size_t r = KEY_LENGTH-rbufLength_;
  if(r > receiveNBytes(r)) {
    return false;
  }
  if(logger_->debug()) {
    logger_->debug("CUID#%s - public key received.", util::itos(cuid_).c_str());
  }
  // TODO handle exception. in catch, resbufLength = 0;
  dh_->computeSecret(secret_, sizeof(secret_), rbuf_, rbufLength_);
  // reset rbufLength_
  rbufLength_ = 0;
  return true;
}

void MSEHandshake::initCipher(const unsigned char* infoHash)
{
  memcpy(infoHash_, infoHash, INFO_HASH_LENGTH);
  //Initialize cipher
  unsigned char s[4+KEY_LENGTH+INFO_HASH_LENGTH];
  memcpy(s, initiator_?"keyA":"keyB", 4);
  memcpy(s+4, secret_, KEY_LENGTH);
  memcpy(s+4+KEY_LENGTH, infoHash, INFO_HASH_LENGTH);
  
  unsigned char localCipherKey[20];
  sha1_->reset();
  MessageDigestHelper::digest(localCipherKey, sizeof(localCipherKey),
                              sha1_, s, sizeof(s));
  encryptor_.reset(new ARC4Encryptor());
  encryptor_->init(localCipherKey, sizeof(localCipherKey));

  unsigned char peerCipherKey[20];
  memcpy(s, initiator_?"keyB":"keyA", 4);
  sha1_->reset();
  MessageDigestHelper::digest(peerCipherKey, sizeof(peerCipherKey),
                              sha1_, s, sizeof(s));
  decryptor_.reset(new ARC4Decryptor());
  decryptor_->init(peerCipherKey, sizeof(peerCipherKey));

  // discard first 1024 bytes ARC4 output.
  unsigned char from[1024];
  unsigned char to[1024];
  encryptor_->encrypt(to, 1024, from, 1024);
  decryptor_->decrypt(to, 1024, from, 1024);

  if(initiator_) {
    ARC4Encryptor enc;
    enc.init(peerCipherKey, sizeof(peerCipherKey));
    // discard first 1024 bytes ARC4 output.
    enc.encrypt(to, 1024, from, 1024);
    enc.encrypt(initiatorVCMarker_, sizeof(initiatorVCMarker_), VC, sizeof(VC));
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
    encryptor_->encrypt(temp, s, dptr, s);
    socketBuffer_.pushStr(std::string(&temp[0], &temp[s]));
    dptr += s;
    r -= s;
  }
}

void MSEHandshake::createReq1Hash(unsigned char* md) const
{
  unsigned char buffer[100];
  memcpy(buffer, "req1", 4);
  memcpy(buffer+4, secret_, KEY_LENGTH);
  sha1_->reset();
  MessageDigestHelper::digest(md, 20, sha1_, buffer, 4+KEY_LENGTH);
}

void MSEHandshake::createReq23Hash(unsigned char* md, const unsigned char* infoHash) const
{
  unsigned char x[24];
  memcpy(x, "req2", 4);
  memcpy(x+4, infoHash, INFO_HASH_LENGTH);
  unsigned char xh[20];
  sha1_->reset();
  MessageDigestHelper::digest(xh, sizeof(xh), sha1_, x, sizeof(x));

  unsigned char y[4+96];
  memcpy(y, "req3", 4);
  memcpy(y+4, secret_, KEY_LENGTH);
  unsigned char yh[20];
  sha1_->reset();
  MessageDigestHelper::digest(yh, sizeof(yh), sha1_, y, sizeof(y));
  
  for(size_t i = 0; i < 20; ++i) {
    md[i] = xh[i]^yh[i];
  }
}

uint16_t MSEHandshake::decodeLength16(const unsigned char* buffer)
{
  uint16_t be;
  decryptor_->decrypt(reinterpret_cast<unsigned char*>(&be),
                      sizeof(be),
                      buffer, sizeof(be));
  return ntohs(be);
}

bool MSEHandshake::sendInitiatorStep2()
{
  if(socketBuffer_.sendBufferIsEmpty()) {
    if(logger_->debug()) {
      logger_->debug("CUID#%s - Sending negotiation step2.",
                     util::itos(cuid_).c_str());
    }
    unsigned char md[20];
    createReq1Hash(md);
    socketBuffer_.pushStr(std::string(&md[0], &md[sizeof(md)]));

    createReq23Hash(md, infoHash_);
    socketBuffer_.pushStr(std::string(&md[0], &md[sizeof(md)]));

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
      if(option_->get(PREF_BT_MIN_CRYPTO_LEVEL) == V_PLAIN) {
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
  socketBuffer_.send();
  return socketBuffer_.sendBufferIsEmpty();
}

// This function reads exactly until the end of VC marker is reached.
bool MSEHandshake::findInitiatorVCMarker()
{
  // 616 is synchronization point of initiator
  size_t r = 616-KEY_LENGTH-rbufLength_;
  if(!socket_->isReadable(0)) {
    return false;
  }
  socket_->peekData(rbuf_+rbufLength_, r);
  if(r == 0) {
    if(socket_->wantRead() || socket_->wantWrite()) {
      return false;
    }
    throw DL_ABORT_EX(EX_EOF_FROM_PEER);
  }
  // find vc
  {
    std::string buf(&rbuf_[0], &rbuf_[rbufLength_+r]);
    std::string vc(&initiatorVCMarker_[0], &initiatorVCMarker_[VC_LENGTH]);
    if((markerIndex_ = buf.find(vc)) == std::string::npos) {
      if(616-KEY_LENGTH <= rbufLength_+r) {
        throw DL_ABORT_EX("Failed to find VC marker.");
      } else {
        socket_->readData(rbuf_+rbufLength_, r);
        rbufLength_ += r;
        return false;
      }
    }
  }
  assert(markerIndex_+VC_LENGTH-rbufLength_ <= r);
  size_t toRead = markerIndex_+VC_LENGTH-rbufLength_;
  socket_->readData(rbuf_+rbufLength_, toRead);
  rbufLength_ += toRead;
  if(logger_->debug()) {
    logger_->debug("CUID#%s - VC marker found at %lu",
                   util::itos(cuid_).c_str(),
                   static_cast<unsigned long>(markerIndex_));
  }
  verifyVC(rbuf_+markerIndex_);
  // reset rbufLength_
  rbufLength_ = 0;
  return true;
}

bool MSEHandshake::receiveInitiatorCryptoSelectAndPadDLength()
{
  size_t r = CRYPTO_BITFIELD_LENGTH+2/* PadD length*/-rbufLength_;
  if(r > receiveNBytes(r)) {
    return false;
  }
  //verifyCryptoSelect
  unsigned char* rbufptr = rbuf_;
  {
    unsigned char cryptoSelect[CRYPTO_BITFIELD_LENGTH];
    decryptor_->decrypt(cryptoSelect, sizeof(cryptoSelect),
                        rbufptr, sizeof(cryptoSelect));
    if(cryptoSelect[3]&CRYPTO_PLAIN_TEXT &&
       option_->get(PREF_BT_MIN_CRYPTO_LEVEL) == V_PLAIN) {
      if(logger_->debug()) {
        logger_->debug("CUID#%s - peer prefers plaintext.",
                       util::itos(cuid_).c_str());
      }
      negotiatedCryptoType_ = CRYPTO_PLAIN_TEXT;
    }
    if(cryptoSelect[3]&CRYPTO_ARC4) {
      if(logger_->debug()) {
        logger_->debug("CUID#%s - peer prefers ARC4",
                       util::itos(cuid_).c_str());
      }
      negotiatedCryptoType_ = CRYPTO_ARC4;
    }
    if(negotiatedCryptoType_ == CRYPTO_NONE) {
      throw DL_ABORT_EX
        (StringFormat("CUID#%s - No supported crypto type selected.",
                      util::itos(cuid_).c_str()).str());
    }
  }
  // padD length
  rbufptr += CRYPTO_BITFIELD_LENGTH;
  padLength_ = verifyPadLength(rbufptr, "PadD");
  // reset rbufLength_
  rbufLength_ = 0;
  return true;
}

bool MSEHandshake::receivePad()
{
  if(padLength_ == 0) {
    return true;
  }
  size_t r = padLength_-rbufLength_;
  if(r > receiveNBytes(r)) {
    return false;
  }
  unsigned char temp[MAX_PAD_LENGTH];
  decryptor_->decrypt(temp, padLength_, rbuf_, padLength_);
  // reset rbufLength_
  rbufLength_ = 0;
  return true;
}

bool MSEHandshake::findReceiverHashMarker()
{
  // 628 is synchronization limit of receiver.
  size_t r = 628-KEY_LENGTH-rbufLength_;
  if(!socket_->isReadable(0)) {
    return false;
  }
  socket_->peekData(rbuf_+rbufLength_, r);
  if(r == 0) {
    if(socket_->wantRead() || socket_->wantWrite()) {
      return false;
    }
    throw DL_ABORT_EX(EX_EOF_FROM_PEER);
  }
  // find hash('req1', S), S is secret_.
  {
    std::string buf(&rbuf_[0], &rbuf_[rbufLength_+r]);
    unsigned char md[20];
    createReq1Hash(md);
    std::string req1(&md[0], &md[sizeof(md)]);
    if((markerIndex_ = buf.find(req1)) == std::string::npos) {
      if(628-KEY_LENGTH <= rbufLength_+r) {
        throw DL_ABORT_EX("Failed to find hash marker.");
      } else {
        socket_->readData(rbuf_+rbufLength_, r);
        rbufLength_ += r;
        return false;
      }
    }
  }
  assert(markerIndex_+20-rbufLength_ <= r);
  size_t toRead = markerIndex_+20-rbufLength_;
  socket_->readData(rbuf_+rbufLength_, toRead);
  rbufLength_ += toRead;
  if(logger_->debug()) {
    logger_->debug("CUID#%s - Hash marker found at %lu.",
                   util::itos(cuid_).c_str(),
                   static_cast<unsigned long>(markerIndex_));
  }
  verifyReq1Hash(rbuf_+markerIndex_);
  // reset rbufLength_
  rbufLength_ = 0;
  return true;
}

bool MSEHandshake::receiveReceiverHashAndPadCLength
(const std::vector<SharedHandle<DownloadContext> >& downloadContexts)
{
  size_t r = 20+VC_LENGTH+CRYPTO_BITFIELD_LENGTH+2/*PadC length*/-rbufLength_;
  if(r > receiveNBytes(r)) {
    return false;
  }
  // resolve info hash
  // pointing to the position of HASH('req2', SKEY) xor HASH('req3', S)
  unsigned char* rbufptr = rbuf_;
  SharedHandle<DownloadContext> downloadContext;
  for(std::vector<SharedHandle<DownloadContext> >::const_iterator i =
        downloadContexts.begin(), eoi = downloadContexts.end();
      i != eoi; ++i) {
    unsigned char md[20];
    const unsigned char* infohash = bittorrent::getInfoHash(*i);
    createReq23Hash(md, infohash);
    if(memcmp(md, rbufptr, sizeof(md)) == 0) {
      if(logger_->debug()) {
        logger_->debug("CUID#%s - info hash found: %s",
                       util::itos(cuid_).c_str(),
                       util::toHex(infohash, INFO_HASH_LENGTH).c_str());
      }
      downloadContext = *i;
      break;
    }
  }
  if(!downloadContext) {
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
    decryptor_->decrypt(cryptoProvide, sizeof(cryptoProvide),
                        rbufptr, sizeof(cryptoProvide));
    // TODO choose the crypto type based on the preference.
    // For now, choose ARC4.
    if(cryptoProvide[3]&CRYPTO_PLAIN_TEXT &&
       option_->get(PREF_BT_MIN_CRYPTO_LEVEL) == V_PLAIN) {
      if(logger_->debug()) {
        logger_->debug("CUID#%s - peer provides plaintext.",
                       util::itos(cuid_).c_str());
      }
      negotiatedCryptoType_ = CRYPTO_PLAIN_TEXT;
    } else if(cryptoProvide[3]&CRYPTO_ARC4) {
      if(logger_->debug()) {
        logger_->debug("CUID#%s - peer provides ARC4.",
                       util::itos(cuid_).c_str());
      }
      negotiatedCryptoType_ = CRYPTO_ARC4;
    }
    if(negotiatedCryptoType_ == CRYPTO_NONE) {
      throw DL_ABORT_EX
        (StringFormat("CUID#%s - No supported crypto type provided.",
                      util::itos(cuid_).c_str()).str());
    }
  }
  // decrypt PadC length
  rbufptr += CRYPTO_BITFIELD_LENGTH;
  padLength_ = verifyPadLength(rbufptr, "PadC");
  // reset rbufLength_
  rbufLength_ = 0;
  return true;
}

bool MSEHandshake::receiveReceiverIALength()
{
  size_t r = 2-rbufLength_;
  assert(r > 0);
  if(r > receiveNBytes(r)) {
    return false;
  }
  iaLength_ = decodeLength16(rbuf_);
  if(logger_->debug()) {
    logger_->debug("CUID#%s - len(IA)=%u.",
                   util::itos(cuid_).c_str(), iaLength_);
  }
  // reset rbufLength_
  rbufLength_ = 0;
  return true;
}

bool MSEHandshake::receiveReceiverIA()
{
  if(iaLength_ == 0) {
    return true;
  }
  size_t r = iaLength_-rbufLength_;
  if(r > receiveNBytes(r)) {
    return false;
  }
  delete [] ia_;
  ia_ = new unsigned char[iaLength_];
  decryptor_->decrypt(ia_, iaLength_, rbuf_, iaLength_);
  if(logger_->debug()) {
    logger_->debug("CUID#%s - IA received.", util::itos(cuid_).c_str());
  }
  // reset rbufLength_
  rbufLength_ = 0;
  return true;
}

bool MSEHandshake::sendReceiverStep2()
{
  if(socketBuffer_.sendBufferIsEmpty()) {
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
    cryptoSelect[3] = negotiatedCryptoType_;
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
  socketBuffer_.send();
  return socketBuffer_.sendBufferIsEmpty();
}

uint16_t MSEHandshake::verifyPadLength(const unsigned char* padlenbuf, const char* padName)
{
  if(logger_->debug()) {
    logger_->debug("CUID#%s - Verifying Pad length for %s",
                   util::itos(cuid_).c_str(), padName);
  }
  uint16_t padLength = decodeLength16(padlenbuf);
  if(logger_->debug()) {
    logger_->debug("CUID#%s - len(%s)=%u",
                   util::itos(cuid_).c_str(), padName, padLength);
  }
  if(padLength > 512) {
    throw DL_ABORT_EX
      (StringFormat("Too large %s length: %u", padName, padLength).str());
  }
  return padLength;
}

void MSEHandshake::verifyVC(const unsigned char* vcbuf)
{
  if(logger_->debug()) {
    logger_->debug("CUID#%s - Verifying VC.", util::itos(cuid_).c_str());
  }
  unsigned char vc[VC_LENGTH];
  decryptor_->decrypt(vc, sizeof(vc), vcbuf, sizeof(vc));
  if(memcmp(VC, vc, sizeof(VC)) != 0) {
    throw DL_ABORT_EX
      (StringFormat("Invalid VC: %s", util::toHex(vc, VC_LENGTH).c_str()).str());
  }
}

void MSEHandshake::verifyReq1Hash(const unsigned char* req1buf)
{
  if(logger_->debug()) {
    logger_->debug("CUID#%s - Verifying req hash.", util::itos(cuid_).c_str());
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
    if(!socket_->isReadable(0)) {
      return 0;
    }
    socket_->readData(rbuf_+rbufLength_, r);
    if(r == 0 && !socket_->wantRead() && !socket_->wantWrite()) {
      throw DL_ABORT_EX(EX_EOF_FROM_PEER);
    }
    rbufLength_ += r;
  }
  return r;
}

} // namespace aria2
