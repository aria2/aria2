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

#include <array>
#include <cstring>
#include <cassert>

#include "message.h"
#include "DlAbortEx.h"
#include "LogFactory.h"
#include "Logger.h"
#include "BtHandshakeMessage.h"
#include "SocketCore.h"
#include "a2netcompat.h"
#include "DHKeyExchange.h"
#include "ARC4Encryptor.h"
#include "MessageDigest.h"
#include "message_digest_helper.h"
#include "SimpleRandomizer.h"
#include "util.h"
#include "DownloadContext.h"
#include "prefs.h"
#include "Option.h"
#include "fmt.h"
#include "bittorrent_helper.h"
#include "array_fun.h"

namespace aria2 {

namespace {

const size_t MAX_PAD_LENGTH = 512;
const size_t CRYPTO_BITFIELD_LENGTH = 4;
const unsigned char VC[] = {0, 0, 0, 0, 0, 0, 0, 0};

const unsigned char* PRIME = reinterpret_cast<const unsigned char*>(
    "FFFFFFFFFFFFFFFFC90FDAA22168C234C4C6628B80DC1CD129024E088A67CC74020BBEA63B"
    "139B22514A08798E3404DDEF9519B3CD3A431B302B0A6DF25F14374FE1356D6D51C245E485"
    "B576625E7EC6F44C42E9A63A36210000000000090563");
const unsigned char* GENERATOR = reinterpret_cast<const unsigned char*>("2");

} // namespace

MSEHandshake::MSEHandshake(cuid_t cuid,
                           const std::shared_ptr<SocketCore>& socket,
                           const Option* op)
    : cuid_(cuid),
      socket_(socket),
      wantRead_(false),
      option_(op),
      rbufLength_(0),
      socketBuffer_(socket),
      negotiatedCryptoType_(CRYPTO_NONE),
      initiator_(true),
      markerIndex_(0),
      padLength_(0),
      iaLength_(0),
      sha1_(MessageDigest::sha1())
{
}

MSEHandshake::~MSEHandshake() {}

MSEHandshake::HANDSHAKE_TYPE MSEHandshake::identifyHandshakeType()
{
  if (rbufLength_ < 20) {
    wantRead_ = true;
    return HANDSHAKE_NOT_YET;
  }
  if (rbuf_[0] == BtHandshakeMessage::PSTR_LENGTH &&
      memcmp(BtHandshakeMessage::BT_PSTR, rbuf_ + 1, 19) == 0) {
    A2_LOG_DEBUG(
        fmt("CUID#%" PRId64 " - This is legacy BitTorrent handshake.", cuid_));
    return HANDSHAKE_LEGACY;
  }
  A2_LOG_DEBUG(fmt(
      "CUID#%" PRId64 " - This may be encrypted BitTorrent handshake.", cuid_));
  return HANDSHAKE_ENCRYPTED;
}

void MSEHandshake::initEncryptionFacility(bool initiator)
{
  dh_ = make_unique<DHKeyExchange>();
  dh_->init(PRIME, PRIME_BITS, GENERATOR, 160);
  dh_->generatePublicKey();
  A2_LOG_DEBUG(fmt("CUID#%" PRId64 " - DH initialized.", cuid_));
  initiator_ = initiator;
}

void MSEHandshake::sendPublicKey()
{
  A2_LOG_DEBUG(fmt("CUID#%" PRId64 " - Sending public key.", cuid_));
  auto buf = make_unique<unsigned char[]>(KEY_LENGTH + MAX_PAD_LENGTH);
  dh_->getPublicKey(buf.get(), KEY_LENGTH);

  size_t padLength =
      SimpleRandomizer::getInstance()->getRandomNumber(MAX_PAD_LENGTH + 1);
  dh_->generateNonce(buf.get() + KEY_LENGTH, padLength);
  socketBuffer_.pushBytes(buf.release(), KEY_LENGTH + padLength);
}

void MSEHandshake::read()
{
  if (rbufLength_ >= MAX_BUFFER_LENGTH) {
    assert(!wantRead_);
    return;
  }
  size_t len = MAX_BUFFER_LENGTH - rbufLength_;
  socket_->readData(rbuf_ + rbufLength_, len);
  if (len == 0 && !socket_->wantRead() && !socket_->wantWrite()) {
    // TODO Should we set graceful in peer?
    throw DL_ABORT_EX(EX_EOF_FROM_PEER);
  }
  rbufLength_ += len;
  wantRead_ = false;
}

bool MSEHandshake::send()
{
  socketBuffer_.send();
  return socketBuffer_.sendBufferIsEmpty();
}

void MSEHandshake::shiftBuffer(size_t offset)
{
  assert(rbufLength_ >= offset);
  memmove(rbuf_, rbuf_ + offset, rbufLength_ - offset);
  rbufLength_ -= offset;
}

bool MSEHandshake::receivePublicKey()
{
  if (rbufLength_ < KEY_LENGTH) {
    wantRead_ = true;
    return false;
  }
  A2_LOG_DEBUG(fmt("CUID#%" PRId64 " - public key received.", cuid_));
  // TODO handle exception. in catch, resbufLength = 0;
  dh_->computeSecret(secret_, sizeof(secret_), rbuf_, KEY_LENGTH);
  // shift buffer
  shiftBuffer(KEY_LENGTH);
  return true;
}

void MSEHandshake::initCipher(const unsigned char* infoHash)
{
  memcpy(infoHash_, infoHash, INFO_HASH_LENGTH);
  // Initialize cipher
  unsigned char s[4 + KEY_LENGTH + INFO_HASH_LENGTH];
  memcpy(s, initiator_ ? "keyA" : "keyB", 4);
  memcpy(s + 4, secret_, KEY_LENGTH);
  memcpy(s + 4 + KEY_LENGTH, infoHash, INFO_HASH_LENGTH);

  unsigned char localCipherKey[20];
  sha1_->reset();
  message_digest::digest(localCipherKey, sizeof(localCipherKey), sha1_.get(), s,
                         sizeof(s));
  encryptor_ = make_unique<ARC4Encryptor>();
  encryptor_->init(localCipherKey, sizeof(localCipherKey));

  unsigned char peerCipherKey[20];
  memcpy(s, initiator_ ? "keyB" : "keyA", 4);
  sha1_->reset();
  message_digest::digest(peerCipherKey, sizeof(peerCipherKey), sha1_.get(), s,
                         sizeof(s));
  decryptor_ = make_unique<ARC4Encryptor>();
  decryptor_->init(peerCipherKey, sizeof(peerCipherKey));

  // discard first 1024 bytes ARC4 output.
  std::array<unsigned char, 1_k> garbage;
  encryptor_->encrypt(garbage.size(), garbage.data(), garbage.data());
  decryptor_->encrypt(garbage.size(), garbage.data(), garbage.data());

  if (initiator_) {
    ARC4Encryptor enc;
    enc.init(peerCipherKey, sizeof(peerCipherKey));
    // discard first 1024 bytes ARC4 output.
    enc.encrypt(garbage.size(), garbage.data(), garbage.data());
    enc.encrypt(VC_LENGTH, initiatorVCMarker_, VC);
  }
}

// Given data is pushed to socketBuffer_ and data will be deleted by
// socketBuffer_.
void MSEHandshake::encryptAndSendData(unsigned char* data, size_t length)
{
  encryptor_->encrypt(length, data, data);
  socketBuffer_.pushBytes(data, length);
}

void MSEHandshake::createReq1Hash(unsigned char* md) const
{
  unsigned char buffer[100];
  memcpy(buffer, "req1", 4);
  memcpy(buffer + 4, secret_, KEY_LENGTH);
  sha1_->reset();
  message_digest::digest(md, 20, sha1_.get(), buffer, 4 + KEY_LENGTH);
}

void MSEHandshake::createReq23Hash(unsigned char* md,
                                   const unsigned char* infoHash) const
{
  unsigned char x[24];
  memcpy(x, "req2", 4);
  memcpy(x + 4, infoHash, INFO_HASH_LENGTH);
  unsigned char xh[20];
  sha1_->reset();
  message_digest::digest(xh, sizeof(xh), sha1_.get(), x, sizeof(x));

  unsigned char y[4 + 96];
  memcpy(y, "req3", 4);
  memcpy(y + 4, secret_, KEY_LENGTH);
  unsigned char yh[20];
  sha1_->reset();
  message_digest::digest(yh, sizeof(yh), sha1_.get(), y, sizeof(y));

  for (size_t i = 0; i < 20; ++i) {
    md[i] = xh[i] ^ yh[i];
  }
}

uint16_t MSEHandshake::decodeLength16(const unsigned char* buffer)
{
  uint16_t be;
  decryptor_->encrypt(sizeof(be), reinterpret_cast<unsigned char*>(&be),
                      buffer);
  return ntohs(be);
}

void MSEHandshake::sendInitiatorStep2()
{
  A2_LOG_DEBUG(fmt("CUID#%" PRId64 " - Sending negotiation step2.", cuid_));
  // Assuming no exception
  auto md = make_unique<unsigned char[]>((size_t)20);
  createReq1Hash(md.get());
  socketBuffer_.pushBytes(md.release(), 20);
  // Assuming no exception
  md = make_unique<unsigned char[]>((size_t)20);
  createReq23Hash(md.get(), infoHash_);
  socketBuffer_.pushBytes(md.release(), 20);
  // buffer is filled in this order:
  //   VC(VC_LENGTH bytes),
  //   crypto_provide(CRYPTO_BITFIELD_LENGTH bytes),
  //   len(padC)(2 bytes),
  //   padC(len(padC) bytes <= MAX_PAD_LENGTH),
  //   len(IA)(2 bytes)
  auto buffer = make_unique<unsigned char[]>(
      40 + VC_LENGTH + CRYPTO_BITFIELD_LENGTH + 2 + MAX_PAD_LENGTH + 2);
  unsigned char* ptr = buffer.get();
  // VC
  memcpy(ptr, VC, sizeof(VC));
  ptr += sizeof(VC);
  // crypto_provide
  memset(ptr, 0, CRYPTO_BITFIELD_LENGTH);
  if (!option_->getAsBool(PREF_BT_FORCE_ENCRYPTION) &&
      option_->get(PREF_BT_MIN_CRYPTO_LEVEL) == V_PLAIN) {
    ptr[3] = CRYPTO_PLAIN_TEXT;
  }
  ptr[3] |= CRYPTO_ARC4;
  ptr += CRYPTO_BITFIELD_LENGTH;
  // len(padC)
  uint16_t padCLength =
      SimpleRandomizer::getInstance()->getRandomNumber(MAX_PAD_LENGTH + 1);
  {
    uint16_t padCLengthBE = htons(padCLength);
    memcpy(ptr, &padCLengthBE, sizeof(padCLengthBE));
  }
  ptr += 2;
  // padC
  memset(ptr, 0, padCLength);
  ptr += padCLength;
  // len(IA)
  // currently, IA is zero-length.
  uint16_t iaLength = 0;
  {
    uint16_t iaLengthBE = htons(iaLength);
    memcpy(ptr, &iaLengthBE, sizeof(iaLengthBE));
  }
  ptr += 2;
  size_t buflen = ptr - buffer.get();
  encryptAndSendData(buffer.release(), buflen);
}

// This function reads exactly until the end of VC marker is reached.
bool MSEHandshake::findInitiatorVCMarker()
{
  // 616 is synchronization point of initiator
  // find vc
  unsigned char* ptr =
      std::search(&rbuf_[0], &rbuf_[rbufLength_], &initiatorVCMarker_[0],
                  &initiatorVCMarker_[VC_LENGTH]);
  if (ptr == &rbuf_[rbufLength_]) {
    if (616 - KEY_LENGTH <= rbufLength_) {
      throw DL_ABORT_EX("Failed to find VC marker.");
    }
    else {
      wantRead_ = true;
      return false;
    }
  }
  markerIndex_ = ptr - rbuf_;
  A2_LOG_DEBUG(fmt("CUID#%" PRId64 " - VC marker found at %lu", cuid_,
                   static_cast<unsigned long>(markerIndex_)));
  verifyVC(rbuf_ + markerIndex_);
  // shift rbuf
  shiftBuffer(markerIndex_ + VC_LENGTH);
  return true;
}

bool MSEHandshake::receiveInitiatorCryptoSelectAndPadDLength()
{
  if (CRYPTO_BITFIELD_LENGTH + 2 /* PadD length*/ > rbufLength_) {
    wantRead_ = true;
    return false;
  }
  // verifyCryptoSelect
  unsigned char* rbufptr = rbuf_;
  decryptor_->encrypt(CRYPTO_BITFIELD_LENGTH, rbufptr, rbufptr);
  if ((rbufptr[3] & CRYPTO_PLAIN_TEXT) &&
      !option_->getAsBool(PREF_BT_FORCE_ENCRYPTION) &&
      option_->get(PREF_BT_MIN_CRYPTO_LEVEL) == V_PLAIN) {
    A2_LOG_DEBUG(fmt("CUID#%" PRId64 " - peer prefers plaintext.", cuid_));
    negotiatedCryptoType_ = CRYPTO_PLAIN_TEXT;
  }
  if (rbufptr[3] & CRYPTO_ARC4) {
    A2_LOG_DEBUG(fmt("CUID#%" PRId64 " - peer prefers ARC4", cuid_));
    negotiatedCryptoType_ = CRYPTO_ARC4;
  }
  if (negotiatedCryptoType_ == CRYPTO_NONE) {
    throw DL_ABORT_EX(
        fmt("CUID#%" PRId64 " - No supported crypto type selected.", cuid_));
  }
  // padD length
  rbufptr += CRYPTO_BITFIELD_LENGTH;
  padLength_ = verifyPadLength(rbufptr, "PadD");
  // shift rbuf
  shiftBuffer(CRYPTO_BITFIELD_LENGTH + 2 /* PadD length*/);
  return true;
}

bool MSEHandshake::receivePad()
{
  if (padLength_ > rbufLength_) {
    wantRead_ = true;
    return false;
  }
  if (padLength_ == 0) {
    return true;
  }
  decryptor_->encrypt(padLength_, rbuf_, rbuf_);
  // shift rbuf_
  shiftBuffer(padLength_);
  return true;
}

bool MSEHandshake::findReceiverHashMarker()
{
  // 628 is synchronization limit of receiver.
  // find hash('req1', S), S is secret_.
  unsigned char md[20];
  createReq1Hash(md);
  unsigned char* ptr =
      std::search(&rbuf_[0], &rbuf_[rbufLength_], &md[0], &md[sizeof(md)]);
  if (ptr == &rbuf_[rbufLength_]) {
    if (628 - KEY_LENGTH <= rbufLength_) {
      throw DL_ABORT_EX("Failed to find hash marker.");
    }
    else {
      wantRead_ = true;
      return false;
    }
  }
  markerIndex_ = ptr - rbuf_;
  A2_LOG_DEBUG(fmt("CUID#%" PRId64 " - Hash marker found at %lu.", cuid_,
                   static_cast<unsigned long>(markerIndex_)));
  verifyReq1Hash(rbuf_ + markerIndex_);
  // shift rbuf_
  shiftBuffer(markerIndex_ + 20);
  return true;
}

bool MSEHandshake::receiveReceiverHashAndPadCLength(
    const std::vector<std::shared_ptr<DownloadContext>>& downloadContexts)
{
  if (20 + VC_LENGTH + CRYPTO_BITFIELD_LENGTH + 2 /*PadC length*/ >
      rbufLength_) {
    wantRead_ = true;
    return false;
  }
  // resolve info hash
  // pointing to the position of HASH('req2', SKEY) xor HASH('req3', S)
  unsigned char* rbufptr = rbuf_;
  std::shared_ptr<DownloadContext> downloadContext;
  for (auto& ctx : downloadContexts) {
    unsigned char md[20];
    const auto infohash = bittorrent::getInfoHash(ctx);
    createReq23Hash(md, infohash);
    if (memcmp(md, rbufptr, sizeof(md)) == 0) {
      A2_LOG_DEBUG(fmt("CUID#%" PRId64 " - info hash found: %s", cuid_,
                       util::toHex(infohash, INFO_HASH_LENGTH).c_str()));
      downloadContext = ctx;
      break;
    }
  }
  if (!downloadContext) {
    throw DL_ABORT_EX("Unknown info hash.");
  }
  initCipher(bittorrent::getInfoHash(downloadContext));
  // decrypt VC
  rbufptr += 20;
  verifyVC(rbufptr);
  // decrypt crypto_provide
  rbufptr += VC_LENGTH;
  decryptor_->encrypt(CRYPTO_BITFIELD_LENGTH, rbufptr, rbufptr);
  // TODO choose the crypto type based on the preference.
  // For now, choose ARC4.
  if ((rbufptr[3] & CRYPTO_PLAIN_TEXT) &&
      !option_->getAsBool(PREF_BT_FORCE_ENCRYPTION) &&
      option_->get(PREF_BT_MIN_CRYPTO_LEVEL) == V_PLAIN) {
    A2_LOG_DEBUG(fmt("CUID#%" PRId64 " - peer provides plaintext.", cuid_));
    negotiatedCryptoType_ = CRYPTO_PLAIN_TEXT;
  }
  else if (rbufptr[3] & CRYPTO_ARC4) {
    A2_LOG_DEBUG(fmt("CUID#%" PRId64 " - peer provides ARC4.", cuid_));
    negotiatedCryptoType_ = CRYPTO_ARC4;
  }
  if (negotiatedCryptoType_ == CRYPTO_NONE) {
    throw DL_ABORT_EX(
        fmt("CUID#%" PRId64 " - No supported crypto type provided.", cuid_));
  }
  // decrypt PadC length
  rbufptr += CRYPTO_BITFIELD_LENGTH;
  padLength_ = verifyPadLength(rbufptr, "PadC");
  // shift rbuf_
  shiftBuffer(20 + VC_LENGTH + CRYPTO_BITFIELD_LENGTH + 2 /*PadC length*/);
  return true;
}

bool MSEHandshake::receiveReceiverIALength()
{
  if (2 > rbufLength_) {
    wantRead_ = true;
    return false;
  }
  iaLength_ = decodeLength16(rbuf_);
  if (iaLength_ > BtHandshakeMessage::MESSAGE_LENGTH) {
    throw DL_ABORT_EX(fmt("Too large IA length length: %u", iaLength_));
  }
  A2_LOG_DEBUG(fmt("CUID#%" PRId64 " - len(IA)=%u.", cuid_, iaLength_));
  // shift rbuf_
  shiftBuffer(2);
  return true;
}

bool MSEHandshake::receiveReceiverIA()
{
  if (iaLength_ == 0) {
    return true;
  }
  if (iaLength_ > rbufLength_) {
    wantRead_ = true;
    return false;
  }
  ia_ = make_unique<unsigned char[]>(iaLength_);
  decryptor_->encrypt(iaLength_, ia_.get(), rbuf_);
  A2_LOG_DEBUG(fmt("CUID#%" PRId64 " - IA received.", cuid_));
  // shift rbuf_
  shiftBuffer(iaLength_);
  return true;
}

void MSEHandshake::sendReceiverStep2()
{
  // buffer is filled in this order:
  //   VC(VC_LENGTH bytes),
  //   cryptoSelect(CRYPTO_BITFIELD_LENGTH bytes),
  //   len(padD)(2bytes),
  //   padD(len(padD)bytes <= MAX_PAD_LENGTH)
  auto buffer = make_unique<unsigned char[]>(
      VC_LENGTH + CRYPTO_BITFIELD_LENGTH + 2 + MAX_PAD_LENGTH);
  unsigned char* ptr = buffer.get();
  // VC
  memcpy(ptr, VC, sizeof(VC));
  ptr += sizeof(VC);
  // crypto_select
  memset(ptr, 0, CRYPTO_BITFIELD_LENGTH);
  ptr[3] = negotiatedCryptoType_;
  ptr += CRYPTO_BITFIELD_LENGTH;
  // len(padD)
  uint16_t padDLength =
      SimpleRandomizer::getInstance()->getRandomNumber(MAX_PAD_LENGTH + 1);
  uint16_t padDLengthBE = htons(padDLength);
  memcpy(ptr, &padDLengthBE, sizeof(padDLengthBE));
  ptr += sizeof(padDLengthBE);
  // padD, all zeroed
  memset(ptr, 0, padDLength);
  ptr += padDLength;
  size_t buflen = ptr - buffer.get();
  encryptAndSendData(buffer.release(), buflen);
}

uint16_t MSEHandshake::verifyPadLength(const unsigned char* padlenbuf,
                                       const char* padName)
{
  A2_LOG_DEBUG(
      fmt("CUID#%" PRId64 " - Verifying Pad length for %s", cuid_, padName));
  uint16_t padLength = decodeLength16(padlenbuf);
  A2_LOG_DEBUG(fmt("CUID#%" PRId64 " - len(%s)=%u", cuid_, padName, padLength));
  if (padLength > 512) {
    throw DL_ABORT_EX(fmt("Too large %s length: %u", padName, padLength));
  }
  return padLength;
}

void MSEHandshake::verifyVC(unsigned char* vcbuf)
{
  A2_LOG_DEBUG(fmt("CUID#%" PRId64 " - Verifying VC.", cuid_));
  decryptor_->encrypt(VC_LENGTH, vcbuf, vcbuf);
  if (memcmp(VC, vcbuf, VC_LENGTH) != 0) {
    throw DL_ABORT_EX(
        fmt("Invalid VC: %s", util::toHex(vcbuf, VC_LENGTH).c_str()));
  }
}

void MSEHandshake::verifyReq1Hash(const unsigned char* req1buf)
{
  A2_LOG_DEBUG(fmt("CUID#%" PRId64 " - Verifying req hash.", cuid_));
  unsigned char md[20];
  createReq1Hash(md);
  if (memcmp(md, req1buf, sizeof(md)) != 0) {
    throw DL_ABORT_EX("Invalid req1 hash found.");
  }
}

bool MSEHandshake::getWantWrite() const
{
  return !socketBuffer_.sendBufferIsEmpty();
}

std::unique_ptr<ARC4Encryptor> MSEHandshake::popEncryptor()
{
  return std::move(encryptor_);
}

std::unique_ptr<ARC4Encryptor> MSEHandshake::popDecryptor()
{
  return std::move(decryptor_);
}

} // namespace aria2
