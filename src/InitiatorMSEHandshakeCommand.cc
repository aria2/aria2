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
#include "InitiatorMSEHandshakeCommand.h"
#include "PeerInitiateConnectionCommand.h"
#include "PeerInteractionCommand.h"
#include "DownloadEngine.h"
#include "DlAbortEx.h"
#include "message.h"
#include "prefs.h"
#include "SocketCore.h"
#include "Logger.h"
#include "LogFactory.h"
#include "Peer.h"
#include "PeerConnection.h"
#include "BtRuntime.h"
#include "PeerStorage.h"
#include "PieceStorage.h"
#include "Option.h"
#include "MSEHandshake.h"
#include "ARC4Encryptor.h"
#include "RequestGroup.h"
#include "DownloadContext.h"
#include "bittorrent_helper.h"
#include "util.h"
#include "fmt.h"
#include "array_fun.h"

namespace aria2 {

InitiatorMSEHandshakeCommand::InitiatorMSEHandshakeCommand(
    cuid_t cuid, RequestGroup* requestGroup, const std::shared_ptr<Peer>& p,
    DownloadEngine* e, const std::shared_ptr<BtRuntime>& btRuntime,
    const std::shared_ptr<SocketCore>& s)
    : PeerAbstractCommand(cuid, p, e, s),
      requestGroup_(requestGroup),
      btRuntime_(btRuntime),
      sequence_(INITIATOR_SEND_KEY),
      mseHandshake_(make_unique<MSEHandshake>(cuid, s, getOption().get()))
{
  disableReadCheckSocket();
  setWriteCheckSocket(getSocket());
  setTimeout(std::chrono::seconds(
      getOption()->getAsInt(PREF_PEER_CONNECTION_TIMEOUT)));

  btRuntime_->increaseConnections();
  requestGroup_->increaseNumCommand();
}

InitiatorMSEHandshakeCommand::~InitiatorMSEHandshakeCommand()
{
  requestGroup_->decreaseNumCommand();
  btRuntime_->decreaseConnections();
}

bool InitiatorMSEHandshakeCommand::executeInternal()
{
  if (mseHandshake_->getWantRead()) {
    mseHandshake_->read();
  }
  bool done = false;
  while (!done) {
    switch (sequence_) {
    case INITIATOR_SEND_KEY: {
      if (!getSocket()->isWritable(0)) {
        addCommandSelf();
        return false;
      }
      setTimeout(std::chrono::seconds(getOption()->getAsInt(PREF_BT_TIMEOUT)));
      mseHandshake_->initEncryptionFacility(true);
      mseHandshake_->sendPublicKey();
      sequence_ = INITIATOR_SEND_KEY_PENDING;
      break;
    }
    case INITIATOR_SEND_KEY_PENDING:
      if (mseHandshake_->send()) {
        sequence_ = INITIATOR_WAIT_KEY;
      }
      else {
        done = true;
      }
      break;
    case INITIATOR_WAIT_KEY: {
      if (mseHandshake_->receivePublicKey()) {
        mseHandshake_->initCipher(
            bittorrent::getInfoHash(requestGroup_->getDownloadContext()));
        ;
        mseHandshake_->sendInitiatorStep2();
        sequence_ = INITIATOR_SEND_STEP2_PENDING;
      }
      else {
        done = true;
      }
      break;
    }
    case INITIATOR_SEND_STEP2_PENDING:
      if (mseHandshake_->send()) {
        sequence_ = INITIATOR_FIND_VC_MARKER;
      }
      else {
        done = true;
      }
      break;
    case INITIATOR_FIND_VC_MARKER: {
      if (mseHandshake_->findInitiatorVCMarker()) {
        sequence_ = INITIATOR_RECEIVE_PAD_D_LENGTH;
      }
      else {
        done = true;
      }
      break;
    }
    case INITIATOR_RECEIVE_PAD_D_LENGTH: {
      if (mseHandshake_->receiveInitiatorCryptoSelectAndPadDLength()) {
        sequence_ = INITIATOR_RECEIVE_PAD_D;
      }
      else {
        done = true;
      }
      break;
    }
    case INITIATOR_RECEIVE_PAD_D: {
      if (mseHandshake_->receivePad()) {
        auto peerConnection =
            make_unique<PeerConnection>(getCuid(), getPeer(), getSocket());
        if (mseHandshake_->getNegotiatedCryptoType() ==
            MSEHandshake::CRYPTO_ARC4) {
          size_t buflen = mseHandshake_->getBufferLength();
          mseHandshake_->getDecryptor()->encrypt(
              buflen, mseHandshake_->getBuffer(), mseHandshake_->getBuffer());
          peerConnection->presetBuffer(mseHandshake_->getBuffer(), buflen);
          peerConnection->enableEncryption(mseHandshake_->popEncryptor(),
                                           mseHandshake_->popDecryptor());
        }
        else {
          peerConnection->presetBuffer(mseHandshake_->getBuffer(),
                                       mseHandshake_->getBufferLength());
        }
        getDownloadEngine()->addCommand(make_unique<PeerInteractionCommand>(
            getCuid(), requestGroup_, getPeer(), getDownloadEngine(),
            btRuntime_, pieceStorage_, peerStorage_, getSocket(),
            PeerInteractionCommand::INITIATOR_SEND_HANDSHAKE,
            std::move(peerConnection)));
        return true;
      }
      else {
        done = true;
      }
      break;
    }
    }
  }
  if (mseHandshake_->getWantRead()) {
    setReadCheckSocket(getSocket());
  }
  else {
    disableReadCheckSocket();
  }
  if (mseHandshake_->getWantWrite()) {
    setWriteCheckSocket(getSocket());
  }
  else {
    disableWriteCheckSocket();
  }
  addCommandSelf();
  return false;
}

void InitiatorMSEHandshakeCommand::tryNewPeer()
{
  if (peerStorage_->isPeerAvailable() && btRuntime_->lessThanEqMinPeers()) {
    cuid_t ncuid = getDownloadEngine()->newCUID();
    std::shared_ptr<Peer> peer = peerStorage_->checkoutPeer(ncuid);
    // sanity check
    if (peer) {
      auto command = make_unique<PeerInitiateConnectionCommand>(
          ncuid, requestGroup_, peer, getDownloadEngine(), btRuntime_);
      command->setPeerStorage(peerStorage_);
      command->setPieceStorage(pieceStorage_);
      getDownloadEngine()->addCommand(std::move(command));
    }
  }
}

bool InitiatorMSEHandshakeCommand::prepareForNextPeer(time_t wait)
{
  if (sequence_ == INITIATOR_SEND_KEY) {
    // We don't try legacy handshake when connection did not
    // established.
    tryNewPeer();
    return true;
  }
  else if (getOption()->getAsBool(PREF_BT_FORCE_ENCRYPTION) ||
           getOption()->getAsBool(PREF_BT_REQUIRE_CRYPTO)) {
    A2_LOG_INFO(fmt("CUID#%" PRId64 " - Establishing connection using legacy"
                    " BitTorrent handshake is disabled by preference.",
                    getCuid()));
    tryNewPeer();
    return true;
  }
  else {
    // try legacy BitTorrent handshake
    A2_LOG_INFO(fmt("CUID#%" PRId64
                    " - Retry using legacy BitTorrent handshake.",
                    getCuid()));
    auto command = make_unique<PeerInitiateConnectionCommand>(
        getCuid(), requestGroup_, getPeer(), getDownloadEngine(), btRuntime_,
        false);
    command->setPeerStorage(peerStorage_);
    command->setPieceStorage(pieceStorage_);
    getDownloadEngine()->addCommand(std::move(command));
    return true;
  }
}

void InitiatorMSEHandshakeCommand::onAbort()
{
  if (sequence_ == INITIATOR_SEND_KEY ||
      getOption()->getAsBool(PREF_BT_FORCE_ENCRYPTION) ||
      getOption()->getAsBool(PREF_BT_REQUIRE_CRYPTO)) {
    peerStorage_->returnPeer(getPeer());
  }
}

bool InitiatorMSEHandshakeCommand::exitBeforeExecute()
{
  return btRuntime_->isHalt();
}

void InitiatorMSEHandshakeCommand::setPeerStorage(
    const std::shared_ptr<PeerStorage>& peerStorage)
{
  peerStorage_ = peerStorage;
}

void InitiatorMSEHandshakeCommand::setPieceStorage(
    const std::shared_ptr<PieceStorage>& pieceStorage)
{
  pieceStorage_ = pieceStorage;
}

const std::shared_ptr<Option>& InitiatorMSEHandshakeCommand::getOption() const
{
  return requestGroup_->getOption();
}

} // namespace aria2
