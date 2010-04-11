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
#include "StringFormat.h"
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
  :
  _downloadContext(downloadContext),
  peer(peer),
  _metadataGetMode(false),
  logger(LogFactory::getInstance()),
  allowedFastSetSize(10),
  keepAliveInterval(120),
  _utPexEnabled(false),
  _dhtEnabled(false),
  _numReceivedMessage(0),
  _maxOutstandingRequest(DEFAULT_MAX_OUTSTANDING_REQUEST)
{}

DefaultBtInteractive::~DefaultBtInteractive() {}

void DefaultBtInteractive::initiateHandshake() {
  SharedHandle<BtMessage> message =
    messageFactory->createHandshakeMessage
    (bittorrent::getInfoHash(_downloadContext), bittorrent::getStaticPeerId());
  dispatcher->addMessageToQueue(message);
  dispatcher->sendMessages();
}

BtMessageHandle DefaultBtInteractive::receiveHandshake(bool quickReply) {
  SharedHandle<BtHandshakeMessage> message =
    btMessageReceiver->receiveHandshake(quickReply);
  if(message.isNull()) {
    return SharedHandle<BtMessage>();
  }
  if(memcmp(message->getPeerId(), bittorrent::getStaticPeerId(),
            PEER_ID_LENGTH) == 0) {
    throw DL_ABORT_EX
      (StringFormat
       ("CUID#%s - Drop connection from the same Peer ID",
        util::itos(cuid).c_str()).str());
  }
  std::vector<SharedHandle<Peer> > activePeers;
  _peerStorage->getActivePeers(activePeers);
  for(std::vector<SharedHandle<Peer> >::const_iterator i = activePeers.begin(),
        eoi = activePeers.end(); i != eoi; ++i) {
    if(memcmp((*i)->getPeerId(), message->getPeerId(), PEER_ID_LENGTH) == 0) {
      throw DL_ABORT_EX
        (StringFormat
         ("CUID#%s - Same Peer ID has been already seen.",
          util::itos(cuid).c_str()).str());
    }
  }

  peer->setPeerId(message->getPeerId());
    
  if(message->isFastExtensionSupported()) {
    peer->setFastExtensionEnabled(true);
    if(logger->info()) {
      logger->info(MSG_FAST_EXTENSION_ENABLED, util::itos(cuid).c_str());
    }
  }
  if(message->isExtendedMessagingEnabled()) {
    peer->setExtendedMessagingEnabled(true);
    if(!_utPexEnabled) {
      _extensionMessageRegistry->removeExtension("ut_pex");
    }
    if(logger->info()) {
      logger->info(MSG_EXTENDED_MESSAGING_ENABLED, util::itos(cuid).c_str());
    }
  }
  if(message->isDHTEnabled()) {
    peer->setDHTEnabled(true);
    if(logger->info()) {
      logger->info(MSG_DHT_ENABLED_PEER, util::itos(cuid).c_str());
    }
  }
  if(logger->info()) {
    logger->info(MSG_RECEIVE_PEER_MESSAGE, util::itos(cuid).c_str(),
                 peer->ipaddr.c_str(), peer->port,
                 message->toString().c_str());
  }
  return message;
}

BtMessageHandle DefaultBtInteractive::receiveAndSendHandshake() {
  return receiveHandshake(true);
}

void DefaultBtInteractive::doPostHandshakeProcessing() {
  // Set time 0 to haveTimer to cache http/ftp download piece completion
  haveTimer.reset(0);
  keepAliveTimer = global::wallclock;
  floodingTimer = global::wallclock;
  _pexTimer.reset(0);
  if(peer->isExtendedMessagingEnabled()) {
    addHandshakeExtendedMessageToQueue();
  }
  if(!_metadataGetMode) {
    addBitfieldMessageToQueue();
  }
  if(peer->isDHTEnabled() && _dhtEnabled) {
    addPortMessageToQueue();
  }
  if(!_metadataGetMode) {
    addAllowedFastMessageToQueue();
  }
  sendPendingMessage();
}

void DefaultBtInteractive::addPortMessageToQueue()
{
  dispatcher->addMessageToQueue
    (messageFactory->createPortMessage(_localNode->getPort()));
}

void DefaultBtInteractive::addHandshakeExtendedMessageToQueue()
{
  static const std::string CLIENT_ARIA2("aria2/"PACKAGE_VERSION);
  HandshakeExtensionMessageHandle m(new HandshakeExtensionMessage());
  m->setClientVersion(CLIENT_ARIA2);
  m->setTCPPort(_btRuntime->getListenPort());
  m->setExtensions(_extensionMessageRegistry->getExtensions());
  const BDE& attrs = _downloadContext->getAttribute(bittorrent::BITTORRENT);
  if(attrs.containsKey(bittorrent::METADATA)) {
    m->setMetadataSize(attrs[bittorrent::METADATA_SIZE].i());
  }
  SharedHandle<BtMessage> msg = messageFactory->createBtExtendedMessage(m);
  dispatcher->addMessageToQueue(msg);
}

void DefaultBtInteractive::addBitfieldMessageToQueue() {
  if(peer->isFastExtensionEnabled()) {
    if(_pieceStorage->allDownloadFinished()) {
      dispatcher->addMessageToQueue(messageFactory->createHaveAllMessage());
    } else if(_pieceStorage->getCompletedLength() > 0) {
      dispatcher->addMessageToQueue(messageFactory->createBitfieldMessage());
    } else {
      dispatcher->addMessageToQueue(messageFactory->createHaveNoneMessage());
    }
  } else {
    if(_pieceStorage->getCompletedLength() > 0) {
      dispatcher->addMessageToQueue(messageFactory->createBitfieldMessage());
    }
  }
}

void DefaultBtInteractive::addAllowedFastMessageToQueue() {
  if(peer->isFastExtensionEnabled()) {
    std::vector<size_t> fastSet;
    bittorrent::computeFastSet(fastSet, peer->ipaddr,
                               _downloadContext->getNumPieces(),
                               bittorrent::getInfoHash(_downloadContext),
                               allowedFastSetSize);
    for(std::vector<size_t>::const_iterator itr = fastSet.begin(),
          eoi = fastSet.end(); itr != eoi; ++itr) {
      dispatcher->addMessageToQueue
        (messageFactory->createAllowedFastMessage(*itr));
    }
  }
}

void DefaultBtInteractive::decideChoking() {
  if(peer->shouldBeChoking()) {
    if(!peer->amChoking()) {
      dispatcher->addMessageToQueue(messageFactory->createChokeMessage());
    }
  } else {
    if(peer->amChoking()) {
      dispatcher->addMessageToQueue(messageFactory->createUnchokeMessage());
    }
  }
}

void DefaultBtInteractive::checkHave() {
  std::vector<size_t> indexes;
  _pieceStorage->getAdvertisedPieceIndexes(indexes, cuid, haveTimer);
  haveTimer = global::wallclock;
  if(indexes.size() >= 20) {
    if(peer->isFastExtensionEnabled() && _pieceStorage->allDownloadFinished()) {
      dispatcher->addMessageToQueue(messageFactory->createHaveAllMessage());
    } else {
      dispatcher->addMessageToQueue(messageFactory->createBitfieldMessage());
    }
  } else {
    for(std::vector<size_t>::const_iterator itr = indexes.begin(),
          eoi = indexes.end(); itr != eoi; ++itr) {
      dispatcher->addMessageToQueue(messageFactory->createHaveMessage(*itr));
    }
  }
}

void DefaultBtInteractive::sendKeepAlive() {
  if(keepAliveTimer.difference(global::wallclock) >= keepAliveInterval) {
    dispatcher->addMessageToQueue(messageFactory->createKeepAliveMessage());
    dispatcher->sendMessages();
    keepAliveTimer = global::wallclock;
  }
}

size_t DefaultBtInteractive::receiveMessages() {
  size_t countOldOutstandingRequest = dispatcher->countOutstandingRequest();
  size_t msgcount = 0;
  for(int i = 0; i < 50; ++i) {
    if(_requestGroupMan->doesOverallDownloadSpeedExceed() ||
       _downloadContext->getOwnerRequestGroup()->doesDownloadSpeedExceed()) {
      break;
    }
    BtMessageHandle message = btMessageReceiver->receiveMessage();
    if(message.isNull()) {
      break;
    }
    ++msgcount;
    if(logger->info()) {
      logger->info(MSG_RECEIVE_PEER_MESSAGE, util::itos(cuid).c_str(),
                   peer->ipaddr.c_str(), peer->port,
                   message->toString().c_str());
    }
    message->doReceivedAction();

    switch(message->getId()) {
    case BtKeepAliveMessage::ID:
      floodingStat.incKeepAliveCount();
      break;
    case BtChokeMessage::ID:
      if(!peer->peerChoking()) {
        floodingStat.incChokeUnchokeCount();
      }
      break;
    case BtUnchokeMessage::ID:
      if(peer->peerChoking()) {
        floodingStat.incChokeUnchokeCount();
      }
      break;
    case BtPieceMessage::ID:
      _peerStorage->updateTransferStatFor(peer);
      // pass through
    case BtRequestMessage::ID:
      inactiveTimer = global::wallclock;
      break;
    }
  }
  if(countOldOutstandingRequest > 0 &&
     dispatcher->countOutstandingRequest() == 0){
    _maxOutstandingRequest = std::min((size_t)UB_MAX_OUTSTANDING_REQUEST,
                                      _maxOutstandingRequest*2);
  }
  return msgcount;
}

void DefaultBtInteractive::decideInterest() {
  if(_pieceStorage->hasMissingPiece(peer)) {
    if(!peer->amInterested()) {
      if(logger->debug()) {
        logger->debug(MSG_PEER_INTERESTED, util::itos(cuid).c_str());
      }
      dispatcher->
        addMessageToQueue(messageFactory->createInterestedMessage());
    }
  } else {
    if(peer->amInterested()) {
      if(logger->debug()) {
        logger->debug(MSG_PEER_NOT_INTERESTED, util::itos(cuid).c_str());
      }
      dispatcher->
        addMessageToQueue(messageFactory->createNotInterestedMessage());
    }
  }
}

void DefaultBtInteractive::fillPiece(size_t maxMissingBlock) {
  if(_pieceStorage->hasMissingPiece(peer)) {

    size_t numMissingBlock = btRequestFactory->countMissingBlock();

    if(peer->peerChoking()) {
      if(peer->isFastExtensionEnabled()) {
        std::vector<size_t> excludedIndexes;
        excludedIndexes.reserve(btRequestFactory->countTargetPiece());
        btRequestFactory->getTargetPieceIndexes(excludedIndexes);
        while(numMissingBlock < maxMissingBlock) {
          SharedHandle<Piece> piece =
            _pieceStorage->getMissingFastPiece(peer, excludedIndexes);
          if(piece.isNull()) {
            break;
          } else {
            btRequestFactory->addTargetPiece(piece);
            numMissingBlock += piece->countMissingBlock();
            excludedIndexes.push_back(piece->getIndex());
          }
        }
      }
    } else {
      std::vector<size_t> excludedIndexes;
      excludedIndexes.reserve(btRequestFactory->countTargetPiece());
      btRequestFactory->getTargetPieceIndexes(excludedIndexes);
      while(numMissingBlock < maxMissingBlock) {
        SharedHandle<Piece> piece =
          _pieceStorage->getMissingPiece(peer, excludedIndexes);
        if(piece.isNull()) {
          break;
        } else {
          btRequestFactory->addTargetPiece(piece);
          numMissingBlock += piece->countMissingBlock();
          excludedIndexes.push_back(piece->getIndex());
        }
      }
    }
  }
}

void DefaultBtInteractive::addRequests() {
  fillPiece(_maxOutstandingRequest);
  size_t reqNumToCreate =
    _maxOutstandingRequest <= dispatcher->countOutstandingRequest() ?
    0 : _maxOutstandingRequest-dispatcher->countOutstandingRequest();
  if(reqNumToCreate > 0) {
    std::vector<SharedHandle<BtMessage> > requests;
    requests.reserve(reqNumToCreate);
    if(_pieceStorage->isEndGame()) {
      btRequestFactory->createRequestMessagesOnEndGame(requests,reqNumToCreate);
    } else {
      btRequestFactory->createRequestMessages(requests, reqNumToCreate);
    }
    dispatcher->addMessageToQueue(requests);
  }
}

void DefaultBtInteractive::cancelAllPiece() {
  btRequestFactory->removeAllTargetPiece();
  if(_metadataGetMode && _downloadContext->getTotalLength() > 0) {
    std::vector<size_t> metadataRequests =
      _utMetadataRequestTracker->getAllTrackedIndex();
    for(std::vector<size_t>::const_iterator i = metadataRequests.begin(),
          eoi = metadataRequests.end(); i != eoi; ++i) {
      if(logger->debug()) {
        logger->debug("Cancel metadata: piece=%lu",
                      static_cast<unsigned long>(*i));
      }
      _pieceStorage->cancelPiece(_pieceStorage->getPiece(*i));
    }
  }
}

void DefaultBtInteractive::sendPendingMessage() {
  dispatcher->sendMessages();
}

void DefaultBtInteractive::detectMessageFlooding() {
  if(floodingTimer.
     difference(global::wallclock) >= FLOODING_CHECK_INTERVAL) {
    if(floodingStat.getChokeUnchokeCount() >= 2 ||
       floodingStat.getKeepAliveCount() >= 2) {
      throw DL_ABORT_EX(EX_FLOODING_DETECTED);
    } else {
      floodingStat.reset();
    }
    floodingTimer = global::wallclock;
  }
}

void DefaultBtInteractive::checkActiveInteraction()
{
  time_t inactiveTime = inactiveTimer.difference(global::wallclock);
  // To allow aria2 to accept mutially interested peer, disconnect unintersted
  // peer.
  {
    const time_t interval = 30;
    if(!peer->amInterested() && !peer->peerInterested() &&
       inactiveTime >= interval) {
      // TODO change the message
      throw DL_ABORT_EX
        (StringFormat("Disconnect peer because we are not interested each other"
                      " after %u second(s).", interval).str());
    }
  }
  // Since the peers which are *just* connected and do nothing to improve
  // mutual download progress are completely waste of resources, those peers
  // are disconnected in a certain time period.
  {
    const time_t interval = 60;
    if(inactiveTime >= interval) {
      throw DL_ABORT_EX
        (StringFormat(EX_DROP_INACTIVE_CONNECTION, interval).str());
    }
  }
}

void DefaultBtInteractive::addPeerExchangeMessage()
{
  if(_pexTimer.
     difference(global::wallclock) >= UTPexExtensionMessage::DEFAULT_INTERVAL) {
    UTPexExtensionMessageHandle m
      (new UTPexExtensionMessage(peer->getExtensionMessageID("ut_pex")));
    const std::deque<SharedHandle<Peer> >& peers = _peerStorage->getPeers();
    {
      for(std::deque<SharedHandle<Peer> >::const_iterator i =
            peers.begin(), eoi = peers.end();
          i != eoi && !m->freshPeersAreFull(); ++i) {
        if(peer->ipaddr != (*i)->ipaddr) {
          m->addFreshPeer(*i);
        }
      }
    }
    {
      for(std::deque<SharedHandle<Peer> >::const_reverse_iterator i =
            peers.rbegin(), eoi = peers.rend();
          i != eoi && !m->droppedPeersAreFull();
          ++i) {
        if(peer->ipaddr != (*i)->ipaddr) {
          m->addDroppedPeer(*i);
        }
      }
    }
    BtMessageHandle msg = messageFactory->createBtExtendedMessage(m);
    dispatcher->addMessageToQueue(msg);
    _pexTimer = global::wallclock;
  }
}

void DefaultBtInteractive::doInteractionProcessing() {
  if(_metadataGetMode) {
    sendKeepAlive();
    _numReceivedMessage = receiveMessages();
    // PieceStorage is re-initialized with metadata_size in
    // HandshakeExtensionMessage::doReceivedAction().
    _pieceStorage =
      _downloadContext->getOwnerRequestGroup()->getPieceStorage();

    if(peer->getExtensionMessageID("ut_metadata") &&
       _downloadContext->getTotalLength() > 0) {
      size_t num = _utMetadataRequestTracker->avail();
      if(num > 0) {
        std::vector<SharedHandle<BtMessage> > requests;
        _utMetadataRequestFactory->create(requests, num, _pieceStorage);
        dispatcher->addMessageToQueue(requests);
      }
      if(_perSecTimer.difference(global::wallclock) >= 1) {
        _perSecTimer = global::wallclock;
        // Drop timeout request after queuing message to give a chance
        // to other connection to request piece.
        std::vector<size_t> indexes =
          _utMetadataRequestTracker->removeTimeoutEntry();
        for(std::vector<size_t>::const_iterator i = indexes.begin(),
              eoi = indexes.end(); i != eoi; ++i) {
          _pieceStorage->cancelPiece(_pieceStorage->getPiece(*i));
        }
      }
      if(_pieceStorage->downloadFinished()) {
        _downloadContext->getOwnerRequestGroup()->setForceHaltRequested
          (true, RequestGroup::NONE);
      }
    }
  } else {
    checkActiveInteraction();

    decideChoking();

    detectMessageFlooding();

    if(_perSecTimer.difference(global::wallclock) >= 1) {
      _perSecTimer = global::wallclock;
      dispatcher->checkRequestSlotAndDoNecessaryThing();
    }
    checkHave();

    sendKeepAlive();

    _numReceivedMessage = receiveMessages();
  
    btRequestFactory->removeCompletedPiece();

    decideInterest();
    if(!_pieceStorage->downloadFinished()) {
      addRequests();
    }
  }
  if(peer->getExtensionMessageID("ut_pex") && _utPexEnabled) {
    addPeerExchangeMessage();
  }

  sendPendingMessage();
}

void DefaultBtInteractive::setLocalNode(const WeakHandle<DHTNode>& node)
{
  _localNode = node;
}

size_t DefaultBtInteractive::countPendingMessage()
{
  return dispatcher->countMessageInQueue();
}
  
bool DefaultBtInteractive::isSendingMessageInProgress()
{
  return dispatcher->isSendingInProgress();
}

size_t DefaultBtInteractive::countReceivedMessageInIteration() const
{
  return _numReceivedMessage;
}

size_t DefaultBtInteractive::countOutstandingRequest()
{
  if(_metadataGetMode) {
    return _utMetadataRequestTracker->count();
  } else {
    return dispatcher->countOutstandingRequest();
  }
}

void DefaultBtInteractive::setBtRuntime
(const SharedHandle<BtRuntime>& btRuntime)
{
  _btRuntime = btRuntime;
}

void DefaultBtInteractive::setPieceStorage
(const SharedHandle<PieceStorage>& pieceStorage)
{
  _pieceStorage = pieceStorage;
}

void DefaultBtInteractive::setPeerStorage
(const SharedHandle<PeerStorage>& peerStorage)
{
  _peerStorage = peerStorage;
}

void DefaultBtInteractive::setPeer(const SharedHandle<Peer>& peer)
{
  this->peer = peer;
}

void DefaultBtInteractive::setBtMessageReceiver
(const SharedHandle<BtMessageReceiver>& receiver)
{
  this->btMessageReceiver = receiver;
}

void DefaultBtInteractive::setDispatcher
(const SharedHandle<BtMessageDispatcher>& dispatcher)
{
  this->dispatcher = dispatcher;
}

void DefaultBtInteractive::setBtRequestFactory
(const SharedHandle<BtRequestFactory>& factory)
{
  this->btRequestFactory = factory;
}

void DefaultBtInteractive::setPeerConnection
(const SharedHandle<PeerConnection>& peerConnection)
{
  this->peerConnection  = peerConnection;
}

void DefaultBtInteractive::setExtensionMessageFactory
(const SharedHandle<ExtensionMessageFactory>& factory)
{
  _extensionMessageFactory = factory;
}

void DefaultBtInteractive::setBtMessageFactory
(const SharedHandle<BtMessageFactory>& factory)
{
  this->messageFactory = factory;
}

void DefaultBtInteractive::setRequestGroupMan
(const WeakHandle<RequestGroupMan>& rgman)
{
  _requestGroupMan = rgman;
}

} // namespace aria2
