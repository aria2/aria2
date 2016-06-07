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
#include "PeerInteractionCommand.h"

#include <algorithm>

#include "DownloadEngine.h"
#include "PeerInitiateConnectionCommand.h"
#include "DefaultBtInteractive.h"
#include "DlAbortEx.h"
#include "message.h"
#include "prefs.h"
#include "SocketCore.h"
#include "Option.h"
#include "DownloadContext.h"
#include "Peer.h"
#include "BtMessage.h"
#include "BtHandshakeMessage.h"
#include "BtRuntime.h"
#include "PeerStorage.h"
#include "DefaultBtMessageDispatcher.h"
#include "DefaultBtMessageReceiver.h"
#include "DefaultBtRequestFactory.h"
#include "DefaultBtMessageFactory.h"
#include "DefaultBtInteractive.h"
#include "PeerConnection.h"
#include "ExtensionMessageFactory.h"
#include "DHTRoutingTable.h"
#include "DHTTaskQueue.h"
#include "DHTTaskFactory.h"
#include "DHTNode.h"
#include "DHTRegistry.h"
#include "DHTPeerAnnounceStorage.h"
#include "DHTTokenTracker.h"
#include "DHTMessageDispatcher.h"
#include "DHTMessageReceiver.h"
#include "DHTMessageFactory.h"
#include "DHTMessageCallback.h"
#include "PieceStorage.h"
#include "RequestGroup.h"
#include "DefaultExtensionMessageFactory.h"
#include "RequestGroupMan.h"
#include "ExtensionMessageRegistry.h"
#include "bittorrent_helper.h"
#include "UTMetadataRequestFactory.h"
#include "UTMetadataRequestTracker.h"
#include "BtRegistry.h"

namespace aria2 {

PeerInteractionCommand::PeerInteractionCommand(
    cuid_t cuid, RequestGroup* requestGroup, const std::shared_ptr<Peer>& p,
    DownloadEngine* e, const std::shared_ptr<BtRuntime>& btRuntime,
    const std::shared_ptr<PieceStorage>& pieceStorage,
    const std::shared_ptr<PeerStorage>& peerStorage,
    const std::shared_ptr<SocketCore>& s, Seq sequence,
    std::unique_ptr<PeerConnection> peerConnection)
    : PeerAbstractCommand{cuid, p, e, s},
      requestGroup_{requestGroup},
      btRuntime_{btRuntime},
      pieceStorage_{pieceStorage},
      peerStorage_{peerStorage},
      sequence_{sequence}
{
  // TODO move following bunch of processing to separate method, like init()
  if (sequence_ == INITIATOR_SEND_HANDSHAKE) {
    disableReadCheckSocket();
    setWriteCheckSocket(getSocket());
    setTimeout(std::chrono::seconds(
        getOption()->getAsInt(PREF_PEER_CONNECTION_TIMEOUT)));
  }

  int family;
  unsigned char compact[COMPACT_LEN_IPV6];
  int compactlen = bittorrent::packcompact(compact, getPeer()->getIPAddress(),
                                           getPeer()->getPort());
  if (compactlen == COMPACT_LEN_IPV6) {
    family = AF_INET6;
  }
  else {
    family = AF_INET;
  }

  auto torrentAttrs =
      bittorrent::getTorrentAttrs(requestGroup_->getDownloadContext());
  bool metadataGetMode = torrentAttrs->metadata.empty();

  auto exMsgRegistry = make_unique<ExtensionMessageRegistry>();
  exMsgRegistry->setExtensionMessageID(ExtensionMessageRegistry::UT_PEX, 8);
  // http://www.bittorrent.org/beps/bep_0009.html
  exMsgRegistry->setExtensionMessageID(ExtensionMessageRegistry::UT_METADATA,
                                       9);

  auto extensionMessageFactory = make_unique<DefaultExtensionMessageFactory>(
      getPeer(), exMsgRegistry.get());
  auto extensionMessageFactoryPtr = extensionMessageFactory.get();
  extensionMessageFactory->setPeerStorage(peerStorage.get());
  extensionMessageFactory->setDownloadContext(
      requestGroup_->getDownloadContext().get());
  // PieceStorage will be set later.

  auto factory = make_unique<DefaultBtMessageFactory>();
  auto factoryPtr = factory.get();
  factory->setCuid(cuid);
  factory->setDownloadContext(requestGroup_->getDownloadContext().get());
  factory->setPieceStorage(pieceStorage.get());
  factory->setPeerStorage(peerStorage.get());
  factory->setExtensionMessageFactory(extensionMessageFactory.get());
  factory->setPeer(getPeer());
  if (family == AF_INET) {
    factory->setLocalNode(DHTRegistry::getData().localNode.get());
    factory->setRoutingTable(DHTRegistry::getData().routingTable.get());
    factory->setTaskQueue(DHTRegistry::getData().taskQueue.get());
    factory->setTaskFactory(DHTRegistry::getData().taskFactory.get());
  }
  else {
    factory->setLocalNode(DHTRegistry::getData6().localNode.get());
    factory->setRoutingTable(DHTRegistry::getData6().routingTable.get());
    factory->setTaskQueue(DHTRegistry::getData6().taskQueue.get());
    factory->setTaskFactory(DHTRegistry::getData6().taskFactory.get());
  }
  if (metadataGetMode) {
    factory->enableMetadataGetMode();
  }

  if (!peerConnection) {
    peerConnection = make_unique<PeerConnection>(cuid, getPeer(), getSocket());
  }
  else {
    if (sequence_ == RECEIVER_WAIT_HANDSHAKE &&
        peerConnection->getBufferLength() > 0) {
      setStatus(Command::STATUS_ONESHOT_REALTIME);
      getDownloadEngine()->setNoWait(true);
    }
  }
  // If the number of pieces gets bigger, the length of Bitfield
  // message payload exceeds the initial buffer capacity of
  // PeerConnection, which is MAX_PAYLOAD_LEN.  We expand buffer as
  // necessary so that PeerConnection can receive the Bitfield
  // message.
  size_t bitfieldPayloadSize =
      1 + (requestGroup_->getDownloadContext()->getNumPieces() + 7) / 8;
  peerConnection->reserveBuffer(bitfieldPayloadSize);

  auto dispatcher = make_unique<DefaultBtMessageDispatcher>();
  auto dispatcherPtr = dispatcher.get();
  dispatcher->setCuid(cuid);
  dispatcher->setPeer(getPeer());
  dispatcher->setDownloadContext(requestGroup_->getDownloadContext().get());
  dispatcher->setRequestTimeout(
      std::chrono::seconds(getOption()->getAsInt(PREF_BT_REQUEST_TIMEOUT)));
  dispatcher->setBtMessageFactory(factory.get());
  dispatcher->setRequestGroupMan(
      getDownloadEngine()->getRequestGroupMan().get());
  dispatcher->setPeerConnection(peerConnection.get());

  auto receiver = make_unique<DefaultBtMessageReceiver>();
  receiver->setDownloadContext(requestGroup_->getDownloadContext().get());
  receiver->setPeerConnection(peerConnection.get());
  receiver->setDispatcher(dispatcher.get());
  receiver->setBtMessageFactory(factory.get());

  auto reqFactory = make_unique<DefaultBtRequestFactory>();
  reqFactory->setPeer(getPeer());
  reqFactory->setPieceStorage(pieceStorage.get());
  reqFactory->setBtMessageDispatcher(dispatcher.get());
  reqFactory->setBtMessageFactory(factory.get());
  reqFactory->setCuid(cuid);

  // reverse depends
  factory->setBtMessageDispatcher(dispatcher.get());
  factory->setBtRequestFactory(reqFactory.get());
  factory->setPeerConnection(peerConnection.get());

  extensionMessageFactory->setBtMessageDispatcher(dispatcher.get());
  extensionMessageFactory->setBtMessageFactory(factory.get());

  getPeer()->allocateSessionResource(
      requestGroup_->getDownloadContext()->getPieceLength(),
      requestGroup_->getDownloadContext()->getTotalLength());
  getPeer()->setBtMessageDispatcher(dispatcher.get());

  auto btInteractive = make_unique<DefaultBtInteractive>(
      requestGroup_->getDownloadContext(), getPeer());
  btInteractive->setBtRuntime(btRuntime_);
  btInteractive->setPieceStorage(pieceStorage_);
  btInteractive->setPeerStorage(peerStorage); // Note: Not a member variable.
  btInteractive->setCuid(cuid);
  btInteractive->setBtMessageReceiver(std::move(receiver));
  btInteractive->setDispatcher(std::move(dispatcher));
  btInteractive->setBtRequestFactory(std::move(reqFactory));
  btInteractive->setPeerConnection(std::move(peerConnection));
  btInteractive->setExtensionMessageFactory(std::move(extensionMessageFactory));
  btInteractive->setExtensionMessageRegistry(std::move(exMsgRegistry));
  btInteractive->setKeepAliveInterval(
      std::chrono::seconds(getOption()->getAsInt(PREF_BT_KEEP_ALIVE_INTERVAL)));
  btInteractive->setRequestGroupMan(
      getDownloadEngine()->getRequestGroupMan().get());
  btInteractive->setBtMessageFactory(std::move(factory));
  if ((metadataGetMode || !torrentAttrs->privateTorrent) &&
      !getPeer()->isLocalPeer()) {
    if (getOption()->getAsBool(PREF_ENABLE_PEER_EXCHANGE)) {
      btInteractive->setUTPexEnabled(true);
    }
    if (family == AF_INET) {
      if (DHTRegistry::isInitialized()) {
        btInteractive->setDHTEnabled(true);
        factoryPtr->setDHTEnabled(true);
        btInteractive->setLocalNode(DHTRegistry::getData().localNode.get());
      }
    }
    else {
      if (DHTRegistry::isInitialized6()) {
        btInteractive->setDHTEnabled(true);
        factoryPtr->setDHTEnabled(true);
        btInteractive->setLocalNode(DHTRegistry::getData6().localNode.get());
      }
    }
  }

  if (metadataGetMode) {
    auto utMetadataRequestFactory = make_unique<UTMetadataRequestFactory>();
    auto utMetadataRequestTracker = make_unique<UTMetadataRequestTracker>();

    utMetadataRequestFactory->setCuid(cuid);
    utMetadataRequestFactory->setDownloadContext(
        requestGroup_->getDownloadContext().get());
    utMetadataRequestFactory->setBtMessageDispatcher(dispatcherPtr);
    utMetadataRequestFactory->setBtMessageFactory(factoryPtr);
    utMetadataRequestFactory->setPeer(getPeer());
    utMetadataRequestFactory->setUTMetadataRequestTracker(
        utMetadataRequestTracker.get());

    extensionMessageFactoryPtr->setUTMetadataRequestTracker(
        utMetadataRequestTracker.get());

    btInteractive->setUTMetadataRequestFactory(
        std::move(utMetadataRequestFactory));
    btInteractive->setUTMetadataRequestTracker(
        std::move(utMetadataRequestTracker));
  }

  btInteractive->setTcpPort(e->getBtRegistry()->getTcpPort());
  if (metadataGetMode) {
    btInteractive->enableMetadataGetMode();
  }
  btInteractive_ = std::move(btInteractive);

  btRuntime_->increaseConnections();
  requestGroup_->increaseNumCommand();
}

PeerInteractionCommand::~PeerInteractionCommand()
{
  if (getPeer()->getCompletedLength() > 0) {
    pieceStorage_->subtractPieceStats(getPeer()->getBitfield(),
                                      getPeer()->getBitfieldLength());
  }
  getPeer()->releaseSessionResource();

  requestGroup_->decreaseNumCommand();
  btRuntime_->decreaseConnections();
}

bool PeerInteractionCommand::executeInternal()
{
  setNoCheck(false);
  bool done = false;
  while (!done) {
    switch (sequence_) {
    case INITIATOR_SEND_HANDSHAKE:
      if (!getSocket()->isWritable(0)) {
        done = true;
        break;
      }
      disableWriteCheckSocket();
      setReadCheckSocket(getSocket());
      // socket->setBlockingMode();
      setTimeout(std::chrono::seconds(getOption()->getAsInt(PREF_BT_TIMEOUT)));
      btInteractive_->initiateHandshake();
      sequence_ = INITIATOR_WAIT_HANDSHAKE;
      break;
    case INITIATOR_WAIT_HANDSHAKE: {
      if (btInteractive_->countPendingMessage() > 0) {
        btInteractive_->sendPendingMessage();
        if (btInteractive_->countPendingMessage() > 0) {
          done = true;
          break;
        }
      }
      auto handshakeMessage = btInteractive_->receiveHandshake();
      if (!handshakeMessage) {
        done = true;
        break;
      }
      btInteractive_->doPostHandshakeProcessing();
      sequence_ = WIRED;
      break;
    }
    case RECEIVER_WAIT_HANDSHAKE: {
      auto handshakeMessage = btInteractive_->receiveAndSendHandshake();
      if (!handshakeMessage) {
        done = true;
        break;
      }
      btInteractive_->doPostHandshakeProcessing();
      sequence_ = WIRED;
      break;
    }
    case WIRED:
      btInteractive_->doInteractionProcessing();
      if (btInteractive_->countReceivedMessageInIteration() > 0) {
        updateKeepAlive();
      }

      if (getDownloadEngine()
              ->getRequestGroupMan()
              ->doesOverallDownloadSpeedExceed() ||
          requestGroup_->doesDownloadSpeedExceed()) {
        disableReadCheckSocket();
        setNoCheck(true);
      }
      else {
        setReadCheckSocket(getSocket());
      }

      done = true;
      break;
    }
  }
  if ((btInteractive_->countPendingMessage() > 0 ||
       btInteractive_->isSendingMessageInProgress()) &&
      !getDownloadEngine()
           ->getRequestGroupMan()
           ->doesOverallUploadSpeedExceed() &&
      !requestGroup_->doesUploadSpeedExceed()) {
    setWriteCheckSocket(getSocket());
  }
  else {
    disableWriteCheckSocket();
  }

  addCommandSelf();
  return false;
}

// TODO this method removed when PeerBalancerCommand is implemented
bool PeerInteractionCommand::prepareForNextPeer(time_t wait)
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
  return true;
}

void PeerInteractionCommand::onAbort()
{
  btInteractive_->cancelAllPiece();
  peerStorage_->returnPeer(getPeer());
}

void PeerInteractionCommand::onFailure(const Exception& err)
{
  requestGroup_->setLastErrorCode(err.getErrorCode(), err.what());
  requestGroup_->setHaltRequested(true);
  getDownloadEngine()->setRefreshInterval(std::chrono::milliseconds(0));
}

bool PeerInteractionCommand::exitBeforeExecute()
{
  return btRuntime_->isHalt();
}

const std::shared_ptr<Option>& PeerInteractionCommand::getOption() const
{
  return requestGroup_->getOption();
}

} // namespace aria2
