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

DefaultBtInteractive::DefaultBtInteractive
(const SharedHandle<DownloadContext>& downloadContext,
 const SharedHandle<Peer>& peer)
  : downloadContext_(downloadContext),
    peer_(peer),
    metadataGetMode_(false),
    localNode_(0),
    allowedFastSetSize_(10),
    haveTimer_(global::wallclock()),
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
    requestGroupMan_(0)
{}

DefaultBtInteractive::~DefaultBtInteractive() {}

void DefaultBtInteractive::initiateHandshake() {
  SharedHandle<BtMessage> message =
    messageFactory_->createHandshakeMessage
    (bittorrent::getInfoHash(downloadContext_), bittorrent::getStaticPeerId());
  dispatcher_->addMessageToQueue(message);
  dispatcher_->sendMessages();
}

BtMessageHandle DefaultBtInteractive::receiveHandshake(bool quickReply) {
  SharedHandle<BtHandshakeMessage> message =
    btMessageReceiver_->receiveHandshake(quickReply);
  if(!message) {
    return SharedHandle<BtMessage>();
  }
  if(memcmp(message->getPeerId(), bittorrent::getStaticPeerId(),
            PEER_ID_LENGTH) == 0) {
    throw DL_ABORT_EX
      (fmt("CUID#%" PRId64 " - Drop connection from the same Peer ID",
           cuid_));
  }
  std::vector<SharedHandle<Peer> > activePeers;
  peerStorage_->getActivePeers(activePeers);
  for(std::vector<SharedHandle<Peer> >::const_iterator i = activePeers.begin(),
        eoi = activePeers.end(); i != eoi; ++i) {
    if(memcmp((*i)->getPeerId(), message->getPeerId(), PEER_ID_LENGTH) == 0) {
      throw DL_ABORT_EX
        (fmt("CUID#%" PRId64 " - Same Peer ID has been already seen.",
             cuid_));
    }
  }

  peer_->setPeerId(message->getPeerId());
    
  if(message->isFastExtensionSupported()) {
    peer_->setFastExtensionEnabled(true);
    A2_LOG_INFO(fmt(MSG_FAST_EXTENSION_ENABLED, cuid_));
  }
  if(message->isExtendedMessagingEnabled()) {
    peer_->setExtendedMessagingEnabled(true);
    if(!utPexEnabled_) {
      extensionMessageRegistry_->removeExtension("ut_pex");
    }
    A2_LOG_INFO(fmt(MSG_EXTENDED_MESSAGING_ENABLED, cuid_));
  }
  if(message->isDHTEnabled()) {
    peer_->setDHTEnabled(true);
    A2_LOG_INFO(fmt(MSG_DHT_ENABLED_PEER, cuid_));
  }
  A2_LOG_INFO(fmt(MSG_RECEIVE_PEER_MESSAGE, cuid_,
                  peer_->getIPAddress().c_str(), peer_->getPort(),
                  message->toString().c_str()));
  return message;
}

BtMessageHandle DefaultBtInteractive::receiveAndSendHandshake() {
  return receiveHandshake(true);
}

void DefaultBtInteractive::doPostHandshakeProcessing() {
  // Set time 0 to haveTimer to cache http/ftp download piece completion
  haveTimer_.reset(0);
  keepAliveTimer_ = global::wallclock();
  floodingTimer_ = global::wallclock();
  pexTimer_.reset(0);
  if(peer_->isExtendedMessagingEnabled()) {
    addHandshakeExtendedMessageToQueue();
  }
  if(!metadataGetMode_) {
    addBitfieldMessageToQueue();
  }
  if(peer_->isDHTEnabled() && dhtEnabled_) {
    addPortMessageToQueue();
  }
  if(!metadataGetMode_) {
    addAllowedFastMessageToQueue();
  }
  sendPendingMessage();
}

void DefaultBtInteractive::addPortMessageToQueue()
{
  dispatcher_->addMessageToQueue
    (messageFactory_->createPortMessage(localNode_->getPort()));
}

void DefaultBtInteractive::addHandshakeExtendedMessageToQueue()
{
  static const std::string CLIENT_ARIA2("aria2/"PACKAGE_VERSION);
  HandshakeExtensionMessageHandle m(new HandshakeExtensionMessage());
  m->setClientVersion(CLIENT_ARIA2);
  m->setTCPPort(tcpPort_);
  m->setExtensions(extensionMessageRegistry_->getExtensions());
  SharedHandle<TorrentAttribute> attrs =
    bittorrent::getTorrentAttrs(downloadContext_);
  if(!attrs->metadata.empty()) {
    m->setMetadataSize(attrs->metadataSize);
  }
  SharedHandle<BtMessage> msg = messageFactory_->createBtExtendedMessage(m);
  dispatcher_->addMessageToQueue(msg);
}

void DefaultBtInteractive::addBitfieldMessageToQueue() {
  if(peer_->isFastExtensionEnabled()) {
    if(pieceStorage_->allDownloadFinished()) {
      dispatcher_->addMessageToQueue(messageFactory_->createHaveAllMessage());
    } else if(pieceStorage_->getCompletedLength() > 0) {
      dispatcher_->addMessageToQueue(messageFactory_->createBitfieldMessage());
    } else {
      dispatcher_->addMessageToQueue(messageFactory_->createHaveNoneMessage());
    }
  } else {
    if(pieceStorage_->getCompletedLength() > 0) {
      dispatcher_->addMessageToQueue(messageFactory_->createBitfieldMessage());
    }
  }
}

void DefaultBtInteractive::addAllowedFastMessageToQueue() {
  if(peer_->isFastExtensionEnabled()) {
    std::vector<size_t> fastSet;
    bittorrent::computeFastSet(fastSet, peer_->getIPAddress(),
                               downloadContext_->getNumPieces(),
                               bittorrent::getInfoHash(downloadContext_),
                               allowedFastSetSize_);
    for(std::vector<size_t>::const_iterator itr = fastSet.begin(),
          eoi = fastSet.end(); itr != eoi; ++itr) {
      dispatcher_->addMessageToQueue
        (messageFactory_->createAllowedFastMessage(*itr));
    }
  }
}

void DefaultBtInteractive::decideChoking() {
  if(peer_->shouldBeChoking()) {
    if(!peer_->amChoking()) {
      dispatcher_->addMessageToQueue(messageFactory_->createChokeMessage());
    }
  } else {
    if(peer_->amChoking()) {
      dispatcher_->addMessageToQueue(messageFactory_->createUnchokeMessage());
    }
  }
}

void DefaultBtInteractive::checkHave() {
  std::vector<size_t> indexes;
  pieceStorage_->getAdvertisedPieceIndexes(indexes, cuid_, haveTimer_);
  haveTimer_ = global::wallclock();
  if(indexes.size() >= 20) {
    if(peer_->isFastExtensionEnabled() &&
       pieceStorage_->allDownloadFinished()) {
      dispatcher_->addMessageToQueue(messageFactory_->createHaveAllMessage());
    } else {
      dispatcher_->addMessageToQueue(messageFactory_->createBitfieldMessage());
    }
  } else {
    for(std::vector<size_t>::const_iterator itr = indexes.begin(),
          eoi = indexes.end(); itr != eoi; ++itr) {
      dispatcher_->addMessageToQueue(messageFactory_->createHaveMessage(*itr));
    }
  }
}

void DefaultBtInteractive::sendKeepAlive() {
  if(keepAliveTimer_.difference(global::wallclock()) >= keepAliveInterval_) {
    dispatcher_->addMessageToQueue(messageFactory_->createKeepAliveMessage());
    dispatcher_->sendMessages();
    keepAliveTimer_ = global::wallclock();
  }
}

size_t DefaultBtInteractive::receiveMessages() {
  size_t countOldOutstandingRequest = dispatcher_->countOutstandingRequest();
  size_t msgcount = 0;
  while(1) {
    if(requestGroupMan_->doesOverallDownloadSpeedExceed() ||
       downloadContext_->getOwnerRequestGroup()->doesDownloadSpeedExceed()) {
      break;
    }
    BtMessageHandle message = btMessageReceiver_->receiveMessage();
    if(!message) {
      break;
    }
    ++msgcount;
    A2_LOG_INFO(fmt(MSG_RECEIVE_PEER_MESSAGE,
                    cuid_,
                    peer_->getIPAddress().c_str(), peer_->getPort(),
                    message->toString().c_str()));
    message->doReceivedAction();

    switch(message->getId()) {
    case BtKeepAliveMessage::ID:
      floodingStat_.incKeepAliveCount();
      break;
    case BtChokeMessage::ID:
      if(!peer_->peerChoking()) {
        floodingStat_.incChokeUnchokeCount();
      }
      break;
    case BtUnchokeMessage::ID:
      if(peer_->peerChoking()) {
        floodingStat_.incChokeUnchokeCount();
      }
      break;
    case BtPieceMessage::ID:
      peerStorage_->updateTransferStatFor(peer_);
      // pass through
    case BtRequestMessage::ID:
      inactiveTimer_ = global::wallclock();
      break;
    }
  }
  if(!pieceStorage_->isEndGame() &&
     countOldOutstandingRequest >= maxOutstandingRequest_ &&
     dispatcher_->countOutstandingRequest()*2 <= maxOutstandingRequest_){
    maxOutstandingRequest_ =
      std::min((size_t)UB_MAX_OUTSTANDING_REQUEST,
               maxOutstandingRequest_+OUTSTANDING_REQUEST_STEP);
  }
  return msgcount;
}

void DefaultBtInteractive::decideInterest() {
  if(pieceStorage_->hasMissingPiece(peer_)) {
    if(!peer_->amInterested()) {
      A2_LOG_DEBUG(fmt(MSG_PEER_INTERESTED, cuid_));
      dispatcher_->
        addMessageToQueue(messageFactory_->createInterestedMessage());
    }
  } else {
    if(peer_->amInterested()) {
      A2_LOG_DEBUG(fmt(MSG_PEER_NOT_INTERESTED, cuid_));
      dispatcher_->
        addMessageToQueue(messageFactory_->createNotInterestedMessage());
    }
  }
}

void DefaultBtInteractive::fillPiece(size_t maxMissingBlock) {
  if(pieceStorage_->hasMissingPiece(peer_)) {
    size_t numMissingBlock = btRequestFactory_->countMissingBlock();
    if(numMissingBlock >= maxMissingBlock) {
      return;
    }
    size_t diffMissingBlock = maxMissingBlock-numMissingBlock;
    std::vector<SharedHandle<Piece> > pieces;
    if(peer_->peerChoking()) {
      if(peer_->isFastExtensionEnabled()) {
        if(pieceStorage_->isEndGame()) {
          std::vector<size_t> excludedIndexes;
          excludedIndexes.reserve(btRequestFactory_->countTargetPiece());
          btRequestFactory_->getTargetPieceIndexes(excludedIndexes);
          pieceStorage_->getMissingFastPiece
            (pieces, diffMissingBlock, peer_, excludedIndexes, cuid_);
        } else {
          pieces.reserve(diffMissingBlock);
          pieceStorage_->getMissingFastPiece
            (pieces, diffMissingBlock, peer_, cuid_);
        }
      }
    } else {
      if(pieceStorage_->isEndGame()) {
        std::vector<size_t> excludedIndexes;
        excludedIndexes.reserve(btRequestFactory_->countTargetPiece());
        btRequestFactory_->getTargetPieceIndexes(excludedIndexes);
        pieceStorage_->getMissingPiece
          (pieces, diffMissingBlock, peer_, excludedIndexes, cuid_);
      } else {        
        pieces.reserve(diffMissingBlock);
        pieceStorage_->getMissingPiece(pieces, diffMissingBlock, peer_, cuid_);
      }
    }
    for(std::vector<SharedHandle<Piece> >::const_iterator i =
          pieces.begin(), eoi = pieces.end(); i != eoi; ++i) {
      btRequestFactory_->addTargetPiece(*i);
    }
  }
}

void DefaultBtInteractive::addRequests() {
  if(!pieceStorage_->isEndGame() && !pieceStorage_->hasMissingUnusedPiece()) {
    pieceStorage_->enterEndGame();
  }
  if(pieceStorage_->isEndGame()) {
    maxOutstandingRequest_ = 2;
  }
  fillPiece(maxOutstandingRequest_);
  size_t reqNumToCreate =
    maxOutstandingRequest_ <= dispatcher_->countOutstandingRequest() ?
    0 : maxOutstandingRequest_-dispatcher_->countOutstandingRequest();
  if(reqNumToCreate > 0) {
    std::vector<SharedHandle<BtMessage> > requests;
    requests.reserve(reqNumToCreate);
    if(pieceStorage_->isEndGame()) {
      btRequestFactory_->createRequestMessagesOnEndGame(requests,reqNumToCreate);
    } else {
      btRequestFactory_->createRequestMessages(requests, reqNumToCreate);
    }
    dispatcher_->addMessageToQueue(requests);
  }
}

void DefaultBtInteractive::cancelAllPiece() {
  btRequestFactory_->removeAllTargetPiece();
  if(metadataGetMode_ && downloadContext_->getTotalLength() > 0) {
    std::vector<size_t> metadataRequests =
      utMetadataRequestTracker_->getAllTrackedIndex();
    for(std::vector<size_t>::const_iterator i = metadataRequests.begin(),
          eoi = metadataRequests.end(); i != eoi; ++i) {
      A2_LOG_DEBUG(fmt("Cancel metadata: piece=%lu",
                       static_cast<unsigned long>(*i)));
      pieceStorage_->cancelPiece(pieceStorage_->getPiece(*i), cuid_);
    }
  }
}

void DefaultBtInteractive::sendPendingMessage() {
  dispatcher_->sendMessages();
}

void DefaultBtInteractive::detectMessageFlooding() {
  if(floodingTimer_.
     difference(global::wallclock()) >= FLOODING_CHECK_INTERVAL) {
    if(floodingStat_.getChokeUnchokeCount() >= 2 ||
       floodingStat_.getKeepAliveCount() >= 2) {
      throw DL_ABORT_EX(EX_FLOODING_DETECTED);
    } else {
      floodingStat_.reset();
    }
    floodingTimer_ = global::wallclock();
  }
}

void DefaultBtInteractive::checkActiveInteraction()
{
  time_t inactiveTime = inactiveTimer_.difference(global::wallclock());
  // To allow aria2 to accept mutially interested peer, disconnect unintersted
  // peer.
  {
    const time_t interval = 30;
    if(!peer_->amInterested() && !peer_->peerInterested() &&
       inactiveTime >= interval) {
      peer_->setDisconnectedGracefully(true);
      // TODO change the message
      throw DL_ABORT_EX
        (fmt("Disconnect peer because we are not interested each other"
             " after %ld second(s).",
             static_cast<long int>(interval)));
    }
  }
  // Since the peers which are *just* connected and do nothing to improve
  // mutual download progress are completely waste of resources, those peers
  // are disconnected in a certain time period.
  {
    const time_t interval = 60;
    if(inactiveTime >= interval) {
      peer_->setDisconnectedGracefully(true);
      throw DL_ABORT_EX
        (fmt(EX_DROP_INACTIVE_CONNECTION,
             static_cast<long int>(interval)));
    }
  }
  // If both of us are seeders, drop connection.
  if(peer_->isSeeder() && pieceStorage_->downloadFinished()) {
    throw DL_ABORT_EX(MSG_GOOD_BYE_SEEDER);
  }
}

void DefaultBtInteractive::addPeerExchangeMessage()
{
  if(pexTimer_.
     difference(global::wallclock()) >= UTPexExtensionMessage::DEFAULT_INTERVAL) {
    UTPexExtensionMessageHandle m
      (new UTPexExtensionMessage(peer_->getExtensionMessageID("ut_pex")));

    std::vector<SharedHandle<Peer> > activePeers;
    peerStorage_->getActivePeers(activePeers);
    for(std::vector<SharedHandle<Peer> >::const_iterator i =
          activePeers.begin(), eoi = activePeers.end();
        i != eoi && !m->freshPeersAreFull(); ++i) {
      if(peer_->getIPAddress() != (*i)->getIPAddress()) {
        m->addFreshPeer(*i);
      }
    }
    const std::deque<SharedHandle<Peer> >& droppedPeers =
      peerStorage_->getDroppedPeers();
    for(std::deque<SharedHandle<Peer> >::const_iterator i =
          droppedPeers.begin(), eoi = droppedPeers.end();
        i != eoi && !m->droppedPeersAreFull();
        ++i) {
      if(peer_->getIPAddress() != (*i)->getIPAddress()) {
        m->addDroppedPeer(*i);
      }
    }

    BtMessageHandle msg = messageFactory_->createBtExtendedMessage(m);
    dispatcher_->addMessageToQueue(msg);
    pexTimer_ = global::wallclock();
  }
}

void DefaultBtInteractive::doInteractionProcessing() {
  if(metadataGetMode_) {
    sendKeepAlive();
    numReceivedMessage_ = receiveMessages();
    // PieceStorage is re-initialized with metadata_size in
    // HandshakeExtensionMessage::doReceivedAction().
    pieceStorage_ =
      downloadContext_->getOwnerRequestGroup()->getPieceStorage();
    if(peer_->getExtensionMessageID("ut_metadata") &&
       downloadContext_->getTotalLength() > 0) {
      size_t num = utMetadataRequestTracker_->avail();
      if(num > 0) {
        std::vector<SharedHandle<BtMessage> > requests;
        utMetadataRequestFactory_->create(requests, num, pieceStorage_);
        dispatcher_->addMessageToQueue(requests);
      }
      if(perSecTimer_.difference(global::wallclock()) >= 1) {
        perSecTimer_ = global::wallclock();
        // Drop timeout request after queuing message to give a chance
        // to other connection to request piece.
        std::vector<size_t> indexes =
          utMetadataRequestTracker_->removeTimeoutEntry();
        for(std::vector<size_t>::const_iterator i = indexes.begin(),
              eoi = indexes.end(); i != eoi; ++i) {
          pieceStorage_->cancelPiece(pieceStorage_->getPiece(*i), cuid_);
        }
      }
      if(pieceStorage_->downloadFinished()) {
        downloadContext_->getOwnerRequestGroup()->setForceHaltRequested
          (true, RequestGroup::NONE);
      }
    }
  } else {
    checkActiveInteraction();
    decideChoking();
    detectMessageFlooding();
    if(perSecTimer_.difference(global::wallclock()) >= 1) {
      perSecTimer_ = global::wallclock();
      dispatcher_->checkRequestSlotAndDoNecessaryThing();
    }
    checkHave();
    sendKeepAlive();
    numReceivedMessage_ = receiveMessages();
    btRequestFactory_->removeCompletedPiece();
    decideInterest();
    if(!pieceStorage_->downloadFinished()) {
      addRequests();
    }
  }
  if(peer_->getExtensionMessageID("ut_pex") && utPexEnabled_) {
    addPeerExchangeMessage();
  }

  sendPendingMessage();
}

void DefaultBtInteractive::setLocalNode(DHTNode* node)
{
  localNode_ = node;
}

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
  if(metadataGetMode_) {
    return utMetadataRequestTracker_->count();
  } else {
    return dispatcher_->countOutstandingRequest();
  }
}

void DefaultBtInteractive::setBtRuntime
(const SharedHandle<BtRuntime>& btRuntime)
{
  btRuntime_ = btRuntime;
}

void DefaultBtInteractive::setPieceStorage
(const SharedHandle<PieceStorage>& pieceStorage)
{
  pieceStorage_ = pieceStorage;
}

void DefaultBtInteractive::setPeerStorage
(const SharedHandle<PeerStorage>& peerStorage)
{
  peerStorage_ = peerStorage;
}

void DefaultBtInteractive::setPeer(const SharedHandle<Peer>& peer)
{
  peer_ = peer;
}

void DefaultBtInteractive::setBtMessageReceiver
(const SharedHandle<BtMessageReceiver>& receiver)
{
  btMessageReceiver_ = receiver;
}

void DefaultBtInteractive::setDispatcher
(const SharedHandle<BtMessageDispatcher>& dispatcher)
{
  dispatcher_ = dispatcher;
}

void DefaultBtInteractive::setBtRequestFactory
(const SharedHandle<BtRequestFactory>& factory)
{
  btRequestFactory_ = factory;
}

void DefaultBtInteractive::setPeerConnection
(const SharedHandle<PeerConnection>& peerConnection)
{
  peerConnection_ = peerConnection;
}

void DefaultBtInteractive::setExtensionMessageFactory
(const SharedHandle<ExtensionMessageFactory>& factory)
{
  extensionMessageFactory_ = factory;
}

void DefaultBtInteractive::setBtMessageFactory
(const SharedHandle<BtMessageFactory>& factory)
{
  messageFactory_ = factory;
}

void DefaultBtInteractive::setRequestGroupMan(RequestGroupMan* rgman)
{
  requestGroupMan_ = rgman;
}

} // namespace aria2
