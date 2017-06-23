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
#include "DefaultBtInteractive.h"

#include <cstring>
#include <vector>

#include "prefs.h"
#include "message.h"
#include "BtHandshakeMessage.h"
#include "util.h"
#include "BtKeepAliveMessage.h"
#include "BtChokeMessage.h"
#include "BtUnchokeMessage.h"
#include "BtRequestMessage.h"
#include "BtPieceMessage.h"
#include "BtPortMessage.h"
#include "BtInterestedMessage.h"
#include "BtNotInterestedMessage.h"
#include "BtHaveMessage.h"
#include "BtHaveAllMessage.h"
#include "BtBitfieldMessage.h"
#include "BtHaveNoneMessage.h"
#include "BtAllowedFastMessage.h"
#include "DlAbortEx.h"
#include "BtExtendedMessage.h"
#include "HandshakeExtensionMessage.h"
#include "UTPexExtensionMessage.h"
#include "DefaultExtensionMessageFactory.h"
#include "ExtensionMessageRegistry.h"
#include "DHTNode.h"
#include "Peer.h"
#include "Piece.h"
#include "DownloadContext.h"
#include "PieceStorage.h"
#include "PeerStorage.h"
#include "BtRuntime.h"
#include "BtMessageReceiver.h"
#include "BtMessageDispatcher.h"
#include "BtMessageFactory.h"
#include "BtRequestFactory.h"
#include "PeerConnection.h"
#include "Logger.h"
#include "LogFactory.h"
#include "fmt.h"
#include "RequestGroup.h"
#include "RequestGroupMan.h"
#include "bittorrent_helper.h"
#include "UTMetadataRequestFactory.h"
#include "UTMetadataRequestTracker.h"
#include "wallclock.h"

namespace aria2 {

DefaultBtInteractive::DefaultBtInteractive(
    const std::shared_ptr<DownloadContext>& downloadContext,
    const std::shared_ptr<Peer>& peer)
    : cuid_(0),
      downloadContext_(downloadContext),
      peer_(peer),
      metadataGetMode_(false),
      localNode_(nullptr),
      lastHaveIndex_(0),
      allowedFastSetSize_(10),
      keepAliveTimer_(global::wallclock()),
      floodingTimer_(global::wallclock()),
      inactiveTimer_(global::wallclock()),
      pexTimer_(global::wallclock()),
      perSecTimer_(global::wallclock()),
      keepAliveInterval_(120),
      utPexEnabled_(false),
      dhtEnabled_(false),
      numReceivedMessage_(0),
      maxOutstandingRequest_(DEFAULT_MAX_OUTSTANDING_REQUEST),
      requestGroupMan_(nullptr),
      tcpPort_(0)
{
}

DefaultBtInteractive::~DefaultBtInteractive() = default;

void DefaultBtInteractive::initiateHandshake()
{
  dispatcher_->addMessageToQueue(messageFactory_->createHandshakeMessage(
      bittorrent::getInfoHash(downloadContext_),
      bittorrent::getStaticPeerId()));
  dispatcher_->sendMessages();
}

std::unique_ptr<BtHandshakeMessage>
DefaultBtInteractive::receiveHandshake(bool quickReply)
{
  auto message = btMessageReceiver_->receiveHandshake(quickReply);
  if (!message) {
    return nullptr;
  }
  if (memcmp(message->getPeerId(), bittorrent::getStaticPeerId(),
             PEER_ID_LENGTH) == 0) {
    throw DL_ABORT_EX(
        fmt("CUID#%" PRId64 " - Drop connection from the same Peer ID", cuid_));
  }
  for (auto& peer : peerStorage_->getUsedPeers()) {
    if (peer->isActive() &&
        memcmp(peer->getPeerId(), message->getPeerId(), PEER_ID_LENGTH) == 0) {
      throw DL_ABORT_EX(
          fmt("CUID#%" PRId64 " - Same Peer ID has been already seen.", cuid_));
    }
  }

  peer_->setPeerId(message->getPeerId());

  if (message->isFastExtensionSupported()) {
    peer_->setFastExtensionEnabled(true);
    A2_LOG_INFO(fmt(MSG_FAST_EXTENSION_ENABLED, cuid_));
  }
  if (message->isExtendedMessagingEnabled()) {
    peer_->setExtendedMessagingEnabled(true);
    if (!utPexEnabled_) {
      extensionMessageRegistry_->removeExtension(
          ExtensionMessageRegistry::UT_PEX);
    }
    A2_LOG_INFO(fmt(MSG_EXTENDED_MESSAGING_ENABLED, cuid_));
  }
  if (message->isDHTEnabled()) {
    peer_->setDHTEnabled(true);
    A2_LOG_INFO(fmt(MSG_DHT_ENABLED_PEER, cuid_));
  }
  A2_LOG_INFO(fmt(MSG_RECEIVE_PEER_MESSAGE, cuid_,
                  peer_->getIPAddress().c_str(), peer_->getPort(),
                  message->toString().c_str()));
  return message;
}

std::unique_ptr<BtHandshakeMessage>
DefaultBtInteractive::receiveAndSendHandshake()
{
  return receiveHandshake(true);
}

void DefaultBtInteractive::doPostHandshakeProcessing()
{
  // Set time 0 to haveTimer to cache http/ftp download piece completion
  keepAliveTimer_ = global::wallclock();
  floodingTimer_ = global::wallclock();
  pexTimer_ = Timer::zero();
  if (peer_->isExtendedMessagingEnabled()) {
    addHandshakeExtendedMessageToQueue();
  }
  if (!metadataGetMode_) {
    addBitfieldMessageToQueue();
  }
  if (peer_->isDHTEnabled() && dhtEnabled_) {
    addPortMessageToQueue();
  }
  if (!metadataGetMode_) {
    addAllowedFastMessageToQueue();
  }
  sendPendingMessage();
}

void DefaultBtInteractive::addPortMessageToQueue()
{
  dispatcher_->addMessageToQueue(
      messageFactory_->createPortMessage(localNode_->getPort()));
}

void DefaultBtInteractive::addHandshakeExtendedMessageToQueue()
{
  auto m = make_unique<HandshakeExtensionMessage>();
  m->setClientVersion(bittorrent::getStaticPeerAgent());
  m->setTCPPort(tcpPort_);
  m->setExtensions(extensionMessageRegistry_->getExtensions());
  auto attrs = bittorrent::getTorrentAttrs(downloadContext_);
  if (!attrs->metadata.empty()) {
    m->setMetadataSize(attrs->metadataSize);
  }
  dispatcher_->addMessageToQueue(
      messageFactory_->createBtExtendedMessage(std::move(m)));
}

void DefaultBtInteractive::addBitfieldMessageToQueue()
{
  if (peer_->isFastExtensionEnabled()) {
    if (pieceStorage_->allDownloadFinished()) {
      dispatcher_->addMessageToQueue(messageFactory_->createHaveAllMessage());
    }
    else if (pieceStorage_->getCompletedLength() > 0) {
      dispatcher_->addMessageToQueue(messageFactory_->createBitfieldMessage());
    }
    else {
      dispatcher_->addMessageToQueue(messageFactory_->createHaveNoneMessage());
    }
  }
  else {
    if (pieceStorage_->getCompletedLength() > 0) {
      dispatcher_->addMessageToQueue(messageFactory_->createBitfieldMessage());
    }
  }
}

void DefaultBtInteractive::addAllowedFastMessageToQueue()
{
  if (peer_->isFastExtensionEnabled()) {
    auto fastSet = bittorrent::computeFastSet(
        peer_->getIPAddress(), downloadContext_->getNumPieces(),
        bittorrent::getInfoHash(downloadContext_), allowedFastSetSize_);
    for (std::vector<size_t>::const_iterator itr = fastSet.begin(),
                                             eoi = fastSet.end();
         itr != eoi; ++itr) {
      dispatcher_->addMessageToQueue(
          messageFactory_->createAllowedFastMessage(*itr));
    }
  }
}

void DefaultBtInteractive::decideChoking()
{
  if (peer_->shouldBeChoking()) {
    if (!peer_->amChoking()) {
      peer_->amChoking(true);
      dispatcher_->doChokingAction();
      dispatcher_->addMessageToQueue(messageFactory_->createChokeMessage());
    }
  }
  else {
    if (peer_->amChoking()) {
      peer_->amChoking(false);
      dispatcher_->addMessageToQueue(messageFactory_->createUnchokeMessage());
    }
  }
}

void DefaultBtInteractive::checkHave()
{
  std::vector<size_t> haveIndexes;

  lastHaveIndex_ = pieceStorage_->getAdvertisedPieceIndexes(haveIndexes, cuid_,
                                                            lastHaveIndex_);

  // Use bitfield message if it is equal to or less than the total
  // size of have messages.
  if (5 + pieceStorage_->getBitfieldLength() <= haveIndexes.size() * 9) {
    if (peer_->isFastExtensionEnabled() &&
        pieceStorage_->allDownloadFinished()) {
      dispatcher_->addMessageToQueue(messageFactory_->createHaveAllMessage());

      return;
    }

    dispatcher_->addMessageToQueue(messageFactory_->createBitfieldMessage());

    return;
  }

  for (auto idx : haveIndexes) {
    dispatcher_->addMessageToQueue(messageFactory_->createHaveMessage(idx));
  }
}

void DefaultBtInteractive::sendKeepAlive()
{
  if (keepAliveTimer_.difference(global::wallclock()) >= keepAliveInterval_) {
    dispatcher_->addMessageToQueue(messageFactory_->createKeepAliveMessage());
    dispatcher_->sendMessages();
    keepAliveTimer_ = global::wallclock();
  }
}

size_t DefaultBtInteractive::receiveMessages()
{
  size_t countOldOutstandingRequest = dispatcher_->countOutstandingRequest();
  size_t msgcount = 0;
  while (1) {
    if (requestGroupMan_->doesOverallDownloadSpeedExceed() ||
        downloadContext_->getOwnerRequestGroup()->doesDownloadSpeedExceed()) {
      break;
    }
    auto message = btMessageReceiver_->receiveMessage();
    if (!message) {
      break;
    }
    ++msgcount;
    A2_LOG_INFO(fmt(MSG_RECEIVE_PEER_MESSAGE, cuid_,
                    peer_->getIPAddress().c_str(), peer_->getPort(),
                    message->toString().c_str()));
    message->doReceivedAction();

    switch (message->getId()) {
    case BtChokeMessage::ID:
      if (!peer_->peerChoking()) {
        floodingStat_.incChokeUnchokeCount();
      }
      break;
    case BtUnchokeMessage::ID:
      if (peer_->peerChoking()) {
        floodingStat_.incChokeUnchokeCount();
      }
      break;
    case BtRequestMessage::ID:
    case BtPieceMessage::ID:
      inactiveTimer_ = global::wallclock();
      break;
    case BtKeepAliveMessage::ID:
      floodingStat_.incKeepAliveCount();
      break;
    }
  }

  if (!pieceStorage_->isEndGame() &&
      countOldOutstandingRequest > dispatcher_->countOutstandingRequest() &&
      (countOldOutstandingRequest - dispatcher_->countOutstandingRequest()) *
              4 >=
          maxOutstandingRequest_) {
    maxOutstandingRequest_ = std::min((size_t)UB_MAX_OUTSTANDING_REQUEST,
                                      maxOutstandingRequest_ * 2);
  }
  return msgcount;
}

void DefaultBtInteractive::decideInterest()
{
  if (pieceStorage_->hasMissingPiece(peer_)) {
    if (!peer_->amInterested()) {
      A2_LOG_DEBUG(fmt(MSG_PEER_INTERESTED, cuid_));
      peer_->amInterested(true);
      dispatcher_->addMessageToQueue(
          messageFactory_->createInterestedMessage());
    }
  }
  else {
    if (peer_->amInterested()) {
      A2_LOG_DEBUG(fmt(MSG_PEER_NOT_INTERESTED, cuid_));
      peer_->amInterested(false);
      dispatcher_->addMessageToQueue(
          messageFactory_->createNotInterestedMessage());
    }
  }
}

void DefaultBtInteractive::fillPiece(size_t maxMissingBlock)
{
  if (pieceStorage_->hasMissingPiece(peer_)) {
    size_t numMissingBlock = btRequestFactory_->countMissingBlock();
    if (numMissingBlock >= maxMissingBlock) {
      return;
    }
    size_t diffMissingBlock = maxMissingBlock - numMissingBlock;
    std::vector<std::shared_ptr<Piece>> pieces;
    if (peer_->peerChoking()) {
      if (peer_->isFastExtensionEnabled()) {
        if (pieceStorage_->isEndGame()) {
          pieceStorage_->getMissingFastPiece(
              pieces, diffMissingBlock, peer_,
              btRequestFactory_->getTargetPieceIndexes(), cuid_);
        }
        else {
          pieces.reserve(diffMissingBlock);
          pieceStorage_->getMissingFastPiece(pieces, diffMissingBlock, peer_,
                                             cuid_);
        }
      }
    }
    else {
      if (pieceStorage_->isEndGame()) {
        pieceStorage_->getMissingPiece(
            pieces, diffMissingBlock, peer_,
            btRequestFactory_->getTargetPieceIndexes(), cuid_);
      }
      else {
        pieces.reserve(diffMissingBlock);
        pieceStorage_->getMissingPiece(pieces, diffMissingBlock, peer_, cuid_);
      }
    }
    for (std::vector<std::shared_ptr<Piece>>::const_iterator i = pieces.begin(),
                                                             eoi = pieces.end();
         i != eoi; ++i) {
      btRequestFactory_->addTargetPiece(*i);
    }
  }
}

void DefaultBtInteractive::addRequests()
{
  if (!pieceStorage_->isEndGame() && !pieceStorage_->hasMissingUnusedPiece()) {
    pieceStorage_->enterEndGame();
  }
  fillPiece(maxOutstandingRequest_);
  size_t reqNumToCreate =
      maxOutstandingRequest_ <= dispatcher_->countOutstandingRequest()
          ? 0
          : maxOutstandingRequest_ - dispatcher_->countOutstandingRequest();

  if (reqNumToCreate > 0) {
    auto requests = btRequestFactory_->createRequestMessages(
        reqNumToCreate, pieceStorage_->isEndGame());
    for (auto& i : requests) {
      dispatcher_->addMessageToQueue(std::move(i));
    }
  }
}

void DefaultBtInteractive::cancelAllPiece()
{
  btRequestFactory_->removeAllTargetPiece();
  if (metadataGetMode_ && downloadContext_->getTotalLength() > 0) {
    std::vector<size_t> metadataRequests =
        utMetadataRequestTracker_->getAllTrackedIndex();
    for (std::vector<size_t>::const_iterator i = metadataRequests.begin(),
                                             eoi = metadataRequests.end();
         i != eoi; ++i) {
      A2_LOG_DEBUG(
          fmt("Cancel metadata: piece=%lu", static_cast<unsigned long>(*i)));
      pieceStorage_->cancelPiece(pieceStorage_->getPiece(*i), cuid_);
    }
  }
}

void DefaultBtInteractive::sendPendingMessage() { dispatcher_->sendMessages(); }

namespace {
constexpr auto FLOODING_CHECK_INTERVAL = 5_s;
} // namespace

void DefaultBtInteractive::detectMessageFlooding()
{
  if (floodingTimer_.difference(global::wallclock()) >=
      FLOODING_CHECK_INTERVAL) {
    if (floodingStat_.getChokeUnchokeCount() >= 2 ||
        floodingStat_.getKeepAliveCount() >= 2) {
      throw DL_ABORT_EX(EX_FLOODING_DETECTED);
    }
    else {
      floodingStat_.reset();
    }
    floodingTimer_ = global::wallclock();
  }
}

void DefaultBtInteractive::checkActiveInteraction()
{
  auto inactiveTime = inactiveTimer_.difference(global::wallclock());
  // To allow aria2 to accept mutially interested peer, disconnect uninterested
  // peer.
  {
    const time_t interval = 30;
    if (!peer_->amInterested() && !peer_->peerInterested() &&
        inactiveTime >= std::chrono::seconds(interval)) {
      peer_->setDisconnectedGracefully(true);
      // TODO change the message
      throw DL_ABORT_EX(
          fmt("Disconnect peer because we are not interested each other"
              " after %ld second(s).",
              static_cast<long int>(interval)));
    }
  }
  // Since the peers which are *just* connected and do nothing to improve
  // mutual download progress are completely waste of resources, those peers
  // are disconnected in a certain time period.
  {
    const time_t interval = 60;
    if (inactiveTime >= std::chrono::seconds(interval)) {
      peer_->setDisconnectedGracefully(true);
      throw DL_ABORT_EX(
          fmt(EX_DROP_INACTIVE_CONNECTION, static_cast<long int>(interval)));
    }
  }
  // If both of us are seeders, drop connection.
  if (peer_->isSeeder() && pieceStorage_->downloadFinished()) {
    throw DL_ABORT_EX(MSG_GOOD_BYE_SEEDER);
  }
}

void DefaultBtInteractive::addPeerExchangeMessage()
{
  if (pexTimer_.difference(global::wallclock()) >=
      UTPexExtensionMessage::DEFAULT_INTERVAL) {
    auto m = make_unique<UTPexExtensionMessage>(
        peer_->getExtensionMessageID(ExtensionMessageRegistry::UT_PEX));
    auto& usedPeers = peerStorage_->getUsedPeers();
    for (auto i = std::begin(usedPeers), eoi = std::end(usedPeers);
         i != eoi && !m->freshPeersAreFull(); ++i) {
      if ((*i)->isActive() && peer_->getIPAddress() != (*i)->getIPAddress()) {
        m->addFreshPeer(*i);
      }
    }
    auto& droppedPeers = peerStorage_->getDroppedPeers();
    for (auto i = std::begin(droppedPeers), eoi = std::end(droppedPeers);
         i != eoi && !m->droppedPeersAreFull(); ++i) {
      if (peer_->getIPAddress() != (*i)->getIPAddress()) {
        m->addDroppedPeer(*i);
      }
    }
    dispatcher_->addMessageToQueue(
        messageFactory_->createBtExtendedMessage(std::move(m)));
    pexTimer_ = global::wallclock();
  }
}

void DefaultBtInteractive::doInteractionProcessing()
{
  if (metadataGetMode_) {
    sendKeepAlive();
    numReceivedMessage_ = receiveMessages();
    // PieceStorage is re-initialized with metadata_size in
    // HandshakeExtensionMessage::doReceivedAction().
    pieceStorage_ = downloadContext_->getOwnerRequestGroup()->getPieceStorage();
    if (peer_->getExtensionMessageID(ExtensionMessageRegistry::UT_METADATA) &&
        downloadContext_->getTotalLength() > 0) {
      size_t num = utMetadataRequestTracker_->avail();
      if (num > 0) {
        auto requests =
            utMetadataRequestFactory_->create(num, pieceStorage_.get());
        for (auto& i : requests) {
          dispatcher_->addMessageToQueue(std::move(i));
        }
      }
      if (perSecTimer_.difference(global::wallclock()) >= 1_s) {
        perSecTimer_ = global::wallclock();
        // Drop timeout request after queuing message to give a chance
        // to other connection to request piece.
        auto indexes = utMetadataRequestTracker_->removeTimeoutEntry();
        for (auto idx : indexes) {
          pieceStorage_->cancelPiece(pieceStorage_->getPiece(idx), cuid_);
        }
      }
      if (pieceStorage_->downloadFinished()) {
        downloadContext_->getOwnerRequestGroup()->setForceHaltRequested(
            true, RequestGroup::NONE);
      }
    }
  }
  else {
    checkActiveInteraction();
    if (perSecTimer_.difference(global::wallclock()) >= 1_s) {
      perSecTimer_ = global::wallclock();
      dispatcher_->checkRequestSlotAndDoNecessaryThing();
    }
    numReceivedMessage_ = receiveMessages();
    detectMessageFlooding();
    decideChoking();
    decideInterest();
    checkHave();
    sendKeepAlive();
    btRequestFactory_->removeCompletedPiece();
    if (!pieceStorage_->downloadFinished()) {
      addRequests();
    }
  }
  if (peer_->getExtensionMessageID(ExtensionMessageRegistry::UT_PEX) &&
      utPexEnabled_) {
    addPeerExchangeMessage();
  }

  sendPendingMessage();
}

void DefaultBtInteractive::setLocalNode(DHTNode* node) { localNode_ = node; }

size_t DefaultBtInteractive::countPendingMessage()
{
  return dispatcher_->countMessageInQueue();
}

bool DefaultBtInteractive::isSendingMessageInProgress()
{
  return dispatcher_->isSendingInProgress();
}

size_t DefaultBtInteractive::countReceivedMessageInIteration() const
{
  return numReceivedMessage_;
}

size_t DefaultBtInteractive::countOutstandingRequest()
{
  if (metadataGetMode_) {
    return utMetadataRequestTracker_->count();
  }
  else {
    return dispatcher_->countOutstandingRequest();
  }
}

void DefaultBtInteractive::setBtRuntime(
    const std::shared_ptr<BtRuntime>& btRuntime)
{
  btRuntime_ = btRuntime;
}

void DefaultBtInteractive::setPieceStorage(
    const std::shared_ptr<PieceStorage>& pieceStorage)
{
  pieceStorage_ = pieceStorage;
}

void DefaultBtInteractive::setPeerStorage(
    const std::shared_ptr<PeerStorage>& peerStorage)
{
  peerStorage_ = peerStorage;
}

void DefaultBtInteractive::setPeer(const std::shared_ptr<Peer>& peer)
{
  peer_ = peer;
}

void DefaultBtInteractive::setBtMessageReceiver(
    std::unique_ptr<BtMessageReceiver> receiver)
{
  btMessageReceiver_ = std::move(receiver);
}

void DefaultBtInteractive::setDispatcher(
    std::unique_ptr<BtMessageDispatcher> dispatcher)
{
  dispatcher_ = std::move(dispatcher);
}

void DefaultBtInteractive::setBtRequestFactory(
    std::unique_ptr<BtRequestFactory> factory)
{
  btRequestFactory_ = std::move(factory);
}

void DefaultBtInteractive::setPeerConnection(
    std::unique_ptr<PeerConnection> peerConnection)
{
  peerConnection_ = std::move(peerConnection);
}

void DefaultBtInteractive::setExtensionMessageFactory(
    std::unique_ptr<ExtensionMessageFactory> factory)
{
  extensionMessageFactory_ = std::move(factory);
}

void DefaultBtInteractive::setBtMessageFactory(
    std::unique_ptr<BtMessageFactory> factory)
{
  messageFactory_ = std::move(factory);
}

void DefaultBtInteractive::setRequestGroupMan(RequestGroupMan* rgman)
{
  requestGroupMan_ = rgman;
}

void DefaultBtInteractive::setExtensionMessageRegistry(
    std::unique_ptr<ExtensionMessageRegistry> registry)
{
  extensionMessageRegistry_ = std::move(registry);
}

void DefaultBtInteractive::setUTMetadataRequestTracker(
    std::unique_ptr<UTMetadataRequestTracker> tracker)
{
  utMetadataRequestTracker_ = std::move(tracker);
}

void DefaultBtInteractive::setUTMetadataRequestFactory(
    std::unique_ptr<UTMetadataRequestFactory> factory)
{
  utMetadataRequestFactory_ = std::move(factory);
}

} // namespace aria2
