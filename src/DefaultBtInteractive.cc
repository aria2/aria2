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
#include "DefaultBtInteractive.h"
#include "prefs.h"
#include "message.h"
#include "BtHandshakeMessage.h"
#include "Util.h"
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
#include "BtRegistry.h"
#include "DHTNode.h"
#include "PeerObject.h"
#include "Peer.h"
#include "Piece.h"
#include "BtContext.h"
#include "PieceStorage.h"
#include "PeerStorage.h"
#include "BtRuntime.h"
#include "BtMessageReceiver.h"
#include "BtMessageDispatcher.h"
#include "BtMessageFactory.h"
#include "BtRequestFactory.h"
#include "PeerConnection.h"
#include "BtRegistry.h"
#include "Logger.h"
#include "LogFactory.h"
#include "StringFormat.h"
#include <cstring>

namespace aria2 {

DefaultBtInteractive::DefaultBtInteractive(const SharedHandle<BtContext>& btContext, const SharedHandle<Peer>& peer):
  BtContextAwareCommand(btContext),
  peer(peer),
  logger(LogFactory::getInstance()),
  allowedFastSetSize(10),
  keepAliveInterval(120),
  maxDownloadSpeedLimit(0),
  _utPexEnabled(false),
  _dhtEnabled(false),
  _numReceivedMessage(0)
{}

DefaultBtInteractive::~DefaultBtInteractive() {}


void DefaultBtInteractive::initiateHandshake() {
  SharedHandle<BtMessage> message =
    messageFactory->createHandshakeMessage(btContext->getInfoHash(),
					   btContext->getPeerId());
  dispatcher->addMessageToQueue(message);
  dispatcher->sendMessages();
}

BtMessageHandle DefaultBtInteractive::receiveHandshake(bool quickReply) {
  SharedHandle<BtHandshakeMessage> message =
    btMessageReceiver->receiveHandshake(quickReply);
  if(message.isNull()) {
    return SharedHandle<BtMessage>();
  }
  if(memcmp(message->getPeerId(), btContext->getPeerId(), PEER_ID_LENGTH) == 0) {
    throw DlAbortEx(StringFormat("CUID#%d - Drop connection from the same Peer ID", cuid).str());
  }

  peer->setPeerId(message->getPeerId());
    
  if(message->isFastExtensionSupported()) {
    peer->setFastExtensionEnabled(true);
    logger->info(MSG_FAST_EXTENSION_ENABLED, cuid);
  }
  if(message->isExtendedMessagingEnabled()) {
    peer->setExtendedMessagingEnabled(true);
    DefaultExtensionMessageFactoryHandle factory
      (new DefaultExtensionMessageFactory(btContext, peer));
    if(!_utPexEnabled) {
      factory->removeExtension("ut_pex");
    }
    PEER_OBJECT(btContext, peer)->extensionMessageFactory = factory;
    logger->info(MSG_EXTENDED_MESSAGING_ENABLED, cuid);
  }
  if(message->isDHTEnabled()) {
    peer->setDHTEnabled(true);
    logger->info(MSG_DHT_ENABLED_PEER, cuid);
  }
  logger->info(MSG_RECEIVE_PEER_MESSAGE, cuid,
	       peer->ipaddr.c_str(), peer->port,
	       message->toString().c_str());
  return message;
}

BtMessageHandle DefaultBtInteractive::receiveAndSendHandshake() {
  return receiveHandshake(true);
}

void DefaultBtInteractive::doPostHandshakeProcessing() {
  // Set time 0 to haveCheckPoint to cache http/ftp download piece completion
  haveCheckPoint.setTimeInSec(0);
  keepAliveCheckPoint.reset();
  floodingCheckPoint.reset();
  _pexCheckPoint.setTimeInSec(0);
  if(peer->isExtendedMessagingEnabled()) {
    addHandshakeExtendedMessageToQueue();
  }
  addBitfieldMessageToQueue();
  if(peer->isDHTEnabled() && _dhtEnabled) {
    addPortMessageToQueue();
  }
  addAllowedFastMessageToQueue();  
  sendPendingMessage();
}

void DefaultBtInteractive::addPortMessageToQueue()
{
  dispatcher->addMessageToQueue(messageFactory->createPortMessage(_localNode->getPort()));
}

void DefaultBtInteractive::addHandshakeExtendedMessageToQueue()
{
  static const std::string CLIENT_ARIA2("aria2");
  HandshakeExtensionMessageHandle m(new HandshakeExtensionMessage());
  m->setClientVersion(CLIENT_ARIA2);
  m->setTCPPort(btRuntime->getListenPort());
  m->setExtensions(EXTENSION_MESSAGE_FACTORY(btContext, peer)->getExtensions());
  
  SharedHandle<BtMessage> msg = messageFactory->createBtExtendedMessage(m);
  dispatcher->addMessageToQueue(msg);
}

void DefaultBtInteractive::addBitfieldMessageToQueue() {
  if(peer->isFastExtensionEnabled()) {
    if(pieceStorage->allDownloadFinished()) {
      dispatcher->addMessageToQueue(messageFactory->createHaveAllMessage());
    } else if(pieceStorage->getCompletedLength() > 0) {
      dispatcher->addMessageToQueue(messageFactory->createBitfieldMessage());
    } else {
      dispatcher->addMessageToQueue(messageFactory->createHaveNoneMessage());
    }
  } else {
    if(pieceStorage->getCompletedLength() > 0) {
      dispatcher->addMessageToQueue(messageFactory->createBitfieldMessage());
    }
  }
}

void DefaultBtInteractive::addAllowedFastMessageToQueue() {
  if(peer->isFastExtensionEnabled()) {
    std::deque<size_t> fastSet;
    btContext->computeFastSet(fastSet, peer->ipaddr, allowedFastSetSize);
    for(std::deque<size_t>::const_iterator itr = fastSet.begin();
	itr != fastSet.end(); itr++) {
      dispatcher->addMessageToQueue(messageFactory->createAllowedFastMessage(*itr));
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
  std::deque<size_t> indexes;
  pieceStorage->getAdvertisedPieceIndexes(indexes, cuid, haveCheckPoint);
  haveCheckPoint.reset();
  if(indexes.size() >= 20) {
    if(peer->isFastExtensionEnabled() && pieceStorage->allDownloadFinished()) {
      dispatcher->addMessageToQueue(messageFactory->createHaveAllMessage());
    } else {
      dispatcher->addMessageToQueue(messageFactory->createBitfieldMessage());
    }
  } else {
    for(std::deque<size_t>::iterator itr = indexes.begin(); itr != indexes.end(); itr++) {
      dispatcher->addMessageToQueue(messageFactory->createHaveMessage(*itr));
    }
  }
}

void DefaultBtInteractive::sendKeepAlive() {
  if(keepAliveCheckPoint.elapsed(keepAliveInterval)) {
    dispatcher->addMessageToQueue(messageFactory->createKeepAliveMessage());
    dispatcher->sendMessages();
    keepAliveCheckPoint.reset();
  }
}

size_t DefaultBtInteractive::receiveMessages() {
  size_t msgcount = 0;
  for(int i = 0; i < 50; i++) {
    if(maxDownloadSpeedLimit > 0) {
      TransferStat stat = peerStorage->calculateStat();
      if(maxDownloadSpeedLimit < stat.downloadSpeed) {
	break;
      }
    }
    BtMessageHandle message = btMessageReceiver->receiveMessage();
    if(message.isNull()) {
      break;
    }
    ++msgcount;
    logger->info(MSG_RECEIVE_PEER_MESSAGE, cuid,
		 peer->ipaddr.c_str(), peer->port,
		 message->toString().c_str());
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
    case BtRequestMessage::ID:
    case BtPieceMessage::ID:
      inactiveCheckPoint.reset();
      break;
    }
  }
  return msgcount;
}

void DefaultBtInteractive::decideInterest() {
  if(pieceStorage->hasMissingPiece(peer)) {
    if(!peer->amInterested()) {
      logger->debug(MSG_PEER_INTERESTED, cuid);
      dispatcher->
	addMessageToQueue(messageFactory->createInterestedMessage());
    }
  } else {
    if(peer->amInterested()) {
      logger->debug(MSG_PEER_NOT_INTERESTED, cuid);
      dispatcher->
	addMessageToQueue(messageFactory->createNotInterestedMessage());
    }
  }
}

void DefaultBtInteractive::fillPiece(size_t maxMissingBlock) {
  if(pieceStorage->hasMissingPiece(peer)) {

    size_t numMissingBlock = btRequestFactory->countMissingBlock();

    if(peer->peerChoking()) {
      if(peer->isFastExtensionEnabled()) {

	while(numMissingBlock < maxMissingBlock) {
	  PieceHandle piece = pieceStorage->getMissingFastPiece(peer);
	  if(piece.isNull()) {
	    break;
	  } else {
	    btRequestFactory->addTargetPiece(piece);
	    numMissingBlock += piece->countMissingBlock();
	  }
	}
      }
    } else {
      while(numMissingBlock < maxMissingBlock) {
	PieceHandle piece = pieceStorage->getMissingPiece(peer);
	if(piece.isNull()) {
	  break;
	} else {
	  btRequestFactory->addTargetPiece(piece);
	  numMissingBlock += piece->countMissingBlock();
	}
      }
    }
  }
}

void DefaultBtInteractive::addRequests() {
  size_t MAX_PENDING_REQUEST;
  if(peer->getLatency() < 500) {
    MAX_PENDING_REQUEST = 24;
  } else if(peer->getLatency() < 1500) {
    MAX_PENDING_REQUEST = 12;
  } else {
    MAX_PENDING_REQUEST = 6;
  }
  fillPiece(MAX_PENDING_REQUEST);

  size_t reqNumToCreate =
    MAX_PENDING_REQUEST <= dispatcher->countOutstandingRequest() ?
    0 : MAX_PENDING_REQUEST-dispatcher->countOutstandingRequest();
  if(reqNumToCreate > 0) {
    BtMessages requests;
    if(pieceStorage->isEndGame()) {
      btRequestFactory->createRequestMessagesOnEndGame(requests, reqNumToCreate);
    } else {
      btRequestFactory->createRequestMessages(requests, reqNumToCreate);
    }
    dispatcher->addMessageToQueue(requests);
  }
}

void DefaultBtInteractive::cancelAllPiece() {
  btRequestFactory->removeAllTargetPiece();
}

void DefaultBtInteractive::sendPendingMessage() {
  dispatcher->sendMessages();
}

void DefaultBtInteractive::detectMessageFlooding() {
  if(floodingCheckPoint.elapsed(FLOODING_CHECK_INTERVAL)) {
    if(floodingStat.getChokeUnchokeCount() >= 2 ||
       floodingStat.getKeepAliveCount() >= 2) {
      throw DlAbortEx(EX_FLOODING_DETECTED);
    } else {
      floodingStat.reset();
    }
    floodingCheckPoint.reset();
  }
}

void DefaultBtInteractive::checkActiveInteraction()
{
  // To allow aria2 to accept mutially interested peer, disconnect unintersted
  // peer.
  {
    time_t interval = 30;
    if(!peer->amInterested() && !peer->peerInterested() &&
       inactiveCheckPoint.elapsed(interval)) {
      // TODO change the message
      throw DlAbortEx
	(StringFormat("Disconnect peer because we are not interested each other after %u second(s).", interval).str());
    }
  }
  // Since the peers which are *just* connected and do nothing to improve
  // mutual download progress are completely waste of resources, those peers
  // are disconnected in a certain time period.
  {
    time_t interval = 2*60;
    if(inactiveCheckPoint.elapsed(interval)) {
      throw DlAbortEx
	(StringFormat(EX_DROP_INACTIVE_CONNECTION, interval).str());
    }
  }
}

void DefaultBtInteractive::addPeerExchangeMessage()
{
  time_t interval = 60;
  if(_pexCheckPoint.elapsed(interval)) {
    UTPexExtensionMessageHandle m
      (new UTPexExtensionMessage(peer->getExtensionMessageID("ut_pex")));
    const Peers& peers = peerStorage->getPeers();
    {
      size_t max = 30;
      for(Peers::const_iterator i = peers.begin();
	  i != peers.end() && max; ++i) {
	const PeerHandle& cpeer = *i;
	if(peer->ipaddr != cpeer->ipaddr && !cpeer->isIncomingPeer() &&
	   !cpeer->getFirstContactTime().elapsed(interval)) {
	  m->addFreshPeer(cpeer);
	  --max;
	}
      }
    }
    {
      size_t max = 10;
      for(Peers::const_reverse_iterator i = peers.rbegin();
	  i != peers.rend() && max; ++i) {
	const PeerHandle& cpeer = *i;
	if(peer->ipaddr != cpeer->ipaddr &&
	   !cpeer->getBadConditionStartTime().elapsed(interval)) {
	  m->addDroppedPeer(cpeer);
	  --max;
	}
      }
    }
    BtMessageHandle msg = messageFactory->createBtExtendedMessage(m);
    dispatcher->addMessageToQueue(msg);
    _pexCheckPoint.reset();
  }
}

void DefaultBtInteractive::doInteractionProcessing() {
  checkActiveInteraction();

  decideChoking();

  detectMessageFlooding();

  dispatcher->checkRequestSlotAndDoNecessaryThing();

  checkHave();

  sendKeepAlive();

  _numReceivedMessage = receiveMessages();
  
  btRequestFactory->removeCompletedPiece();

  decideInterest();
  if(!pieceStorage->downloadFinished()) {
    addRequests();
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
  return dispatcher->countOutstandingRequest();
}

void DefaultBtInteractive::setPeer(const SharedHandle<Peer>& peer)
{
  this->peer = peer;
}

void DefaultBtInteractive::setBtMessageReceiver(const BtMessageReceiverWeakHandle& receiver)
{
  this->btMessageReceiver = receiver;
}

void DefaultBtInteractive::setDispatcher(const BtMessageDispatcherWeakHandle& dispatcher)
{
  this->dispatcher = dispatcher;
}

void DefaultBtInteractive::setBtRequestFactory(const BtRequestFactoryWeakHandle& factory)
{
  this->btRequestFactory = factory;
}

void DefaultBtInteractive::setPeerConnection(const PeerConnectionWeakHandle& peerConnection)
{
  this->peerConnection  = peerConnection;
}

void DefaultBtInteractive::setBtMessageFactory(const BtMessageFactoryWeakHandle& factory)
{
  this->messageFactory = factory;
}

} // namespace aria2
