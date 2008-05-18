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
#include "DefaultBtMessageDispatcher.h"
#include "prefs.h"
#include "BtAbortOutstandingRequestEvent.h"
#include "BtCancelSendingPieceEvent.h"
#include "BtChokedEvent.h"
#include "BtChokingEvent.h"
#include "BtMessageFactory.h"
#include "message.h"
#include "BtContext.h"
#include "PeerStorage.h"
#include "PieceStorage.h"
#include "BtMessage.h"
#include "BtRegistry.h"
#include "Peer.h"
#include "Piece.h"
#include "LogFactory.h"
#include "Logger.h"
#include "a2functional.h"
#include <algorithm>

namespace aria2 {

DefaultBtMessageDispatcher::DefaultBtMessageDispatcher():
  cuid(0),
  maxUploadSpeedLimit(0),
  requestTimeout(0),
  logger(LogFactory::getInstance()) {}

DefaultBtMessageDispatcher::~DefaultBtMessageDispatcher()
{
  logger->debug("DefaultBtMessageDispatcher::deleted");
}

void DefaultBtMessageDispatcher::addMessageToQueue(const BtMessageHandle& btMessage)
{
  btMessage->onQueued();
  messageQueue.push_back(btMessage);
}

void DefaultBtMessageDispatcher::addMessageToQueue(const BtMessages& btMessages)
{
  for(BtMessages::const_iterator itr = btMessages.begin(); itr != btMessages.end(); itr++) {
    addMessageToQueue(*itr);
  }
}


void DefaultBtMessageDispatcher::sendMessages() {
  BtMessages tempQueue;
  while(!messageQueue.empty()) {
    BtMessageHandle msg = messageQueue.front();
    messageQueue.pop_front();
    if(maxUploadSpeedLimit > 0 &&
       msg->isUploading() && !msg->isSendingInProgress()) {
      TransferStat stat = peerStorage->calculateStat();
      if(maxUploadSpeedLimit < stat.getUploadSpeed()) {
	tempQueue.push_back(msg);
	continue;
      }
    }
    msg->send();
    if(msg->isSendingInProgress()) {
      messageQueue.push_front(msg);
      break;
    }
  }
  messageQueue.insert(messageQueue.end(), tempQueue.begin(), tempQueue.end());
}

class HandleEvent {
private:
  SharedHandle<BtEvent> _event;
public:
  HandleEvent(const SharedHandle<BtEvent>& event):_event(event) {}

  void operator()(const SharedHandle<BtMessage>& msg) const
  {
    msg->handleEvent(_event);
  }
};

// Cancel sending piece message to peer.
void DefaultBtMessageDispatcher::doCancelSendingPieceAction(size_t index, uint32_t begin, size_t length)
{
  BtCancelSendingPieceEventHandle event
    (new BtCancelSendingPieceEvent(index, begin, length));

  BtMessages tempQueue = messageQueue;
  std::for_each(tempQueue.begin(), tempQueue.end(), HandleEvent(event));
}

// Cancel sending piece message to peer.
// TODO Is this method really necessary?
void DefaultBtMessageDispatcher::doCancelSendingPieceAction(const PieceHandle& piece)
{
}

class AbortOutstandingRequest {
private:
  SharedHandle<Piece> _piece;
  int32_t _cuid;
  Logger* _logger;
public:
  AbortOutstandingRequest(const SharedHandle<Piece>& piece, int32_t cuid):
    _piece(piece),
    _cuid(cuid),
    _logger(LogFactory::getInstance()) {}

  void operator()(const RequestSlot& slot) const
  {
    _logger->debug(MSG_DELETING_REQUEST_SLOT,
		   _cuid,
		   slot.getIndex(),
		   slot.getBlockIndex());
    _logger->debug("index=%d, begin=%d", slot.getIndex(), slot.getBegin());
    _piece->cancelBlock(slot.getBlockIndex());
  }
};

// localhost cancels outstanding download requests to the peer.
void DefaultBtMessageDispatcher::doAbortOutstandingRequestAction(const PieceHandle& piece) {
  RequestSlot rs(piece->getIndex(), 0, 0, 0);
  std::deque<RequestSlot>::iterator first =
    std::lower_bound(requestSlots.begin(), requestSlots.end(), rs);

  rs.setIndex(piece->getIndex()+1);
  std::deque<RequestSlot>::iterator last =
    std::lower_bound(requestSlots.begin(), requestSlots.end(), rs);

  std::for_each(first, last, AbortOutstandingRequest(piece, cuid));
  requestSlots.erase(first, last);

  BtAbortOutstandingRequestEventHandle event
    (new BtAbortOutstandingRequestEvent(piece));

  BtMessages tempQueue = messageQueue;
  std::for_each(tempQueue.begin(), tempQueue.end(), HandleEvent(event));
}

class ProcessChokedRequestSlot {
private:
  int32_t _cuid;
  SharedHandle<Peer> _peer;
  SharedHandle<PieceStorage> _pieceStorage;
  Logger* _logger;
public:
  ProcessChokedRequestSlot(int32_t cuid,
			   const SharedHandle<Peer>& peer,
			   const SharedHandle<PieceStorage>& pieceStorage):
    _cuid(cuid),
    _peer(peer),
    _pieceStorage(pieceStorage),
    _logger(LogFactory::getInstance()) {}
  
  void operator()(const RequestSlot& slot) const
  {
    if(!_peer->isInPeerAllowedIndexSet(slot.getIndex())) {
      _logger->debug(MSG_DELETING_REQUEST_SLOT_CHOKED,
		     _cuid,
		     slot.getIndex(),
		     slot.getBlockIndex());
      _logger->debug("index=%d, begin=%d", slot.getIndex(), slot.getBegin());
      SharedHandle<Piece> piece = _pieceStorage->getPiece(slot.getIndex());
      piece->cancelBlock(slot.getBlockIndex());
    }
  }

};

class FindChokedRequestSlot {
private:
  SharedHandle<Peer> _peer;
public:
  FindChokedRequestSlot(const SharedHandle<Peer>& peer):
    _peer(peer) {}
  
  bool operator()(const RequestSlot& slot) const
  {
    return !_peer->isInPeerAllowedIndexSet(slot.getIndex());
  }
};

// localhost received choke message from the peer.
void DefaultBtMessageDispatcher::doChokedAction()
{
  std::for_each(requestSlots.begin(), requestSlots.end(),
		ProcessChokedRequestSlot(cuid, peer, pieceStorage));

  requestSlots.erase(std::remove_if(requestSlots.begin(), requestSlots.end(),
				    FindChokedRequestSlot(peer)),
		     requestSlots.end());

  BtChokedEventHandle event(new BtChokedEvent());

  BtMessages tempQueue = messageQueue;
  std::for_each(tempQueue.begin(), tempQueue.end(), HandleEvent(event));
}

// localhost dispatched choke message to the peer.
void DefaultBtMessageDispatcher::doChokingAction()
{
  BtChokingEventHandle event(new BtChokingEvent());

  BtMessages tempQueue = messageQueue;
  std::for_each(tempQueue.begin(), tempQueue.end(), HandleEvent(event));
}

class ProcessStaleRequestSlot {
private:
  int32_t _cuid;
  SharedHandle<Peer> _peer;
  SharedHandle<PieceStorage> _pieceStorage;
  BtMessageDispatcher* _messageDispatcher;
  WeakHandle<BtMessageFactory> _messageFactory;
  time_t _requestTimeout;
  Logger* _logger;
public:
  ProcessStaleRequestSlot(int32_t cuid, const SharedHandle<Peer>& peer,
			  const SharedHandle<PieceStorage>& pieceStorage,
			  BtMessageDispatcher* dispatcher,
			  const WeakHandle<BtMessageFactory>& factory,
			  time_t requestTimeout):
    _cuid(cuid),
    _peer(peer),
    _pieceStorage(pieceStorage),
    _messageDispatcher(dispatcher),
    _messageFactory(factory),
    _requestTimeout(requestTimeout),
    _logger(LogFactory::getInstance()) {}

  void operator()(const RequestSlot& slot)
  {
    SharedHandle<Piece> piece = _pieceStorage->getPiece(slot.getIndex());
    if(slot.isTimeout(_requestTimeout)) {
      _logger->debug(MSG_DELETING_REQUEST_SLOT_TIMEOUT,
		     _cuid,
		     slot.getBlockIndex());
      _logger->debug("index=%d, begin=%d", slot.getIndex(), slot.getBegin());
      piece->cancelBlock(slot.getBlockIndex());
      _peer->snubbing(true);
    } else if(piece->hasBlock(slot.getBlockIndex())) {
      _logger->debug(MSG_DELETING_REQUEST_SLOT_ACQUIRED,
		     _cuid,
		     slot.getBlockIndex());
      _logger->debug("index=%d, begin=%d", slot.getIndex(), slot.getBegin());
      _messageDispatcher->addMessageToQueue
	(_messageFactory->createCancelMessage(slot.getIndex(),
					      slot.getBegin(),
					      slot.getLength()));
    }
  }
};

class FindStaleRequestSlot {
private:
  SharedHandle<PieceStorage> _pieceStorage;
  time_t _requestTimeout;
public:
  FindStaleRequestSlot(const SharedHandle<PieceStorage>& pieceStorage,
		       time_t requestTimeout):
    _pieceStorage(pieceStorage),
    _requestTimeout(requestTimeout) {}

  bool operator()(const RequestSlot& slot)
  {
    if(slot.isTimeout(_requestTimeout)) {
      return true;
    } else {
      SharedHandle<Piece> piece = _pieceStorage->getPiece(slot.getIndex());
      if(piece->hasBlock(slot.getBlockIndex())) {
	return true;
      } else {
	return false;
      }
    }
  }
};

void DefaultBtMessageDispatcher::checkRequestSlotAndDoNecessaryThing()
{
  std::for_each(requestSlots.begin(), requestSlots.end(),
		ProcessStaleRequestSlot(cuid,
					peer,
					pieceStorage,
					this,
					messageFactory,
					requestTimeout));
  requestSlots.erase(std::remove_if(requestSlots.begin(), requestSlots.end(),
				    FindStaleRequestSlot(pieceStorage,
							 requestTimeout)),
		     requestSlots.end());
}

bool DefaultBtMessageDispatcher::isSendingInProgress()
{
  if(messageQueue.size() > 0) {
    return messageQueue.front()->isSendingInProgress();
  } else {
    return false;
  }
}

size_t DefaultBtMessageDispatcher::countOutstandingRequest()
{
  return requestSlots.size();
}

class BlockIndexLess {
public:
  bool operator()(const RequestSlot& lhs, const RequestSlot& rhs) const
  {
    if(lhs.getIndex() == rhs.getIndex()) {
      return lhs.getBlockIndex() < rhs.getBlockIndex();
    } else {
      return lhs.getIndex() < rhs.getIndex();
    }
  }
};

bool DefaultBtMessageDispatcher::isOutstandingRequest(size_t index, size_t blockIndex) {
  RequestSlot rs(index, 0, 0, blockIndex);

  std::deque<RequestSlot>::iterator i =
    std::lower_bound(requestSlots.begin(), requestSlots.end(), rs, BlockIndexLess());
  return i != requestSlots.end() &&
    (*i).getIndex() == index && (*i).getBlockIndex() == blockIndex;
}

RequestSlot
DefaultBtMessageDispatcher::getOutstandingRequest(size_t index, uint32_t begin, size_t length)
{
  RequestSlot ret(0, 0, 0, 0);
  RequestSlot rs(index, begin, length, 0);
  std::deque<RequestSlot>::iterator i =
    std::lower_bound(requestSlots.begin(), requestSlots.end(), rs);
  if(i != requestSlots.end() && (*i) == rs) {
    ret = *i;
  } else {
    ret = RequestSlot::nullSlot;
  }
  return ret;
}

void DefaultBtMessageDispatcher::removeOutstandingRequest(const RequestSlot& slot)
{
  std::deque<RequestSlot>::iterator i =
    std::lower_bound(requestSlots.begin(), requestSlots.end(), slot);
  if(i != requestSlots.end() && (*i) == slot) {
    requestSlots.erase(i);
  }
}

void DefaultBtMessageDispatcher::addOutstandingRequest(const RequestSlot& slot)
{
  std::deque<RequestSlot>::iterator i =
    std::lower_bound(requestSlots.begin(), requestSlots.end(), slot);
  if(i == requestSlots.end() || (*i) != slot) {
    requestSlots.insert(i, slot);
  }
}

size_t DefaultBtMessageDispatcher::countOutstandingUpload()
{
  return std::count_if(messageQueue.begin(), messageQueue.end(),
		       mem_fun_sh(&BtMessage::isUploading));
}

const std::deque<SharedHandle<BtMessage> >&
DefaultBtMessageDispatcher::getMessageQueue()
{
  return messageQueue;
}

const std::deque<RequestSlot>& DefaultBtMessageDispatcher::getRequestSlots()
{
  return requestSlots;
}

void DefaultBtMessageDispatcher::setPeer(const SharedHandle<Peer>& peer)
{
  this->peer = peer;
}

void DefaultBtMessageDispatcher::setBtContext(const BtContextHandle& btContext)
{
  this->btContext = btContext;
  this->pieceStorage = PIECE_STORAGE(btContext);
  this->peerStorage = PEER_STORAGE(btContext);
}

void DefaultBtMessageDispatcher::setBtMessageFactory(const WeakHandle<BtMessageFactory>& factory)
{
  this->messageFactory = factory;
}

} // namespace aria2
