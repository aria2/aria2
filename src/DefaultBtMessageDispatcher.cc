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
  btContext(0),
  peerStorage(0),
  pieceStorage(0),
  peer(0),
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
  while(messageQueue.size() > 0) {
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
  std::copy(tempQueue.begin(), tempQueue.end(), std::back_inserter(messageQueue));
}

// Cancel sending piece message to peer.
void DefaultBtMessageDispatcher::doCancelSendingPieceAction(size_t index, uint32_t begin, size_t length)
{
  BtCancelSendingPieceEventHandle event =
    new BtCancelSendingPieceEvent(index, begin, length);

  BtMessages tempQueue = messageQueue;
  for(BtMessages::iterator itr = tempQueue.begin(); itr != tempQueue.end(); itr++) {
    (*itr)->handleEvent(event);
  }
}

// Cancel sending piece message to peer.
// TODO Is this method really necessary?
void DefaultBtMessageDispatcher::doCancelSendingPieceAction(const PieceHandle& piece)
{
}

// localhost cancels outstanding download requests to the peer.
void DefaultBtMessageDispatcher::doAbortOutstandingRequestAction(const PieceHandle& piece) {
  for(RequestSlots::iterator itr = requestSlots.begin();
      itr != requestSlots.end();) {
    RequestSlot& slot = *itr;
    if(slot.getIndex() == piece->getIndex()) {
      logger->debug(MSG_DELETING_REQUEST_SLOT,
		    cuid,
		    slot.getIndex(),
		    slot.getBlockIndex());
      piece->cancelBlock(slot.getBlockIndex());
      itr = requestSlots.erase(itr);
    } else {
      itr++;
    }
  }

  BtAbortOutstandingRequestEventHandle event =
    new BtAbortOutstandingRequestEvent(piece);

  BtMessages tempQueue = messageQueue;
  for(BtMessages::iterator itr = tempQueue.begin(); itr != tempQueue.end(); ++itr) {
    (*itr)->handleEvent(event);
  }  
}

// localhost received choke message from the peer.
void DefaultBtMessageDispatcher::doChokedAction()
{
  for(RequestSlots::iterator itr = requestSlots.begin();
      itr != requestSlots.end();) {
    RequestSlot& slot = *itr;
    if(peer->isInPeerAllowedIndexSet(slot.getIndex())) {
      itr++;
    } else {
      logger->debug(MSG_DELETING_REQUEST_SLOT_CHOKED,
		    cuid,
		    slot.getIndex(),
		    slot.getBlockIndex());
      PieceHandle piece = pieceStorage->getPiece(slot.getIndex());
      piece->cancelBlock(slot.getBlockIndex());
      itr = requestSlots.erase(itr);
    }
  }

  BtChokedEventHandle event = new BtChokedEvent();

  BtMessages tempQueue = messageQueue;
  for(BtMessages::iterator itr = tempQueue.begin(); itr != tempQueue.end(); ++itr) {
    (*itr)->handleEvent(event);
  }
}

// localhost dispatched choke message to the peer.
void DefaultBtMessageDispatcher::doChokingAction()
{
  BtChokingEventHandle event = new BtChokingEvent();

  BtMessages tempQueue = messageQueue;
  for(BtMessages::iterator itr = tempQueue.begin(); itr != tempQueue.end(); ++itr) {
    (*itr)->handleEvent(event);
  }
}

void DefaultBtMessageDispatcher::checkRequestSlotAndDoNecessaryThing()
{
  for(RequestSlots::iterator itr = requestSlots.begin();
      itr != requestSlots.end();) {
    RequestSlot& slot = *itr;
    PieceHandle piece = pieceStorage->getPiece(slot.getIndex());
    if(slot.isTimeout(requestTimeout)) {
      logger->debug(MSG_DELETING_REQUEST_SLOT_TIMEOUT,
		    cuid,
		    slot.getBlockIndex());
      piece->cancelBlock(slot.getBlockIndex());
      peer->snubbing(true);
      itr = requestSlots.erase(itr);
    } else if(piece->hasBlock(slot.getBlockIndex())) {
      logger->debug(MSG_DELETING_REQUEST_SLOT_ACQUIRED,
		    cuid,
		    slot.getBlockIndex());
      addMessageToQueue(messageFactory->createCancelMessage(slot.getIndex(),
							    slot.getBegin(),
							    slot.getLength()));
      itr = requestSlots.erase(itr);
    } else {
      itr++;
    }
  }
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

bool DefaultBtMessageDispatcher::isOutstandingRequest(size_t index, size_t blockIndex) {
  for(RequestSlots::const_iterator itr = requestSlots.begin();
      itr != requestSlots.end(); itr++) {
    const RequestSlot& slot = *itr;
    if(slot.getIndex() == index && slot.getBlockIndex() == blockIndex) {
      return true;
    }
  }
  return false;
}

RequestSlot
DefaultBtMessageDispatcher::getOutstandingRequest(size_t index, uint32_t begin, size_t length)
{
  for(RequestSlots::iterator itr = requestSlots.begin();
      itr != requestSlots.end(); itr++) {
    if(itr->getIndex() == index &&
       itr->getBegin() == begin &&
       itr->getLength() == length) {
      return *itr;
    }
  }
  return RequestSlot::nullSlot;
}

void DefaultBtMessageDispatcher::removeOutstandingRequest(const RequestSlot& slot)
{
  RequestSlots temp;
  std::remove_copy(requestSlots.begin(), requestSlots.end(), std::back_inserter(temp), slot);
  requestSlots = temp;
}

void DefaultBtMessageDispatcher::addOutstandingRequest(const RequestSlot& requestSlot)
{
  if(!isOutstandingRequest(requestSlot.getIndex(), requestSlot.getBlockIndex())) {
    requestSlots.push_back(requestSlot);
  }
}

size_t DefaultBtMessageDispatcher::countOutstandingUpload()
{
  return std::count_if(messageQueue.begin(), messageQueue.end(),
		       mem_fun_sh(&BtMessage::isUploading));
}

std::deque<SharedHandle<BtMessage> >&
DefaultBtMessageDispatcher::getMessageQueue()
{
  return messageQueue;
}

std::deque<RequestSlot>& DefaultBtMessageDispatcher::getRequestSlots()
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
