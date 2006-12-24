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
#include "BtRegistry.h"
#include "prefs.h"
#include "BtAbortOutstandingRequestEvent.h"
#include "BtCancelSendingPieceEvent.h"
#include "BtChokedEvent.h"
#include "BtChokingEvent.h"
#include "BtRegistry.h"
#include "BtMessageFactory.h"

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
  int32_t uploadLimit = option->getAsInt(PREF_MAX_UPLOAD_LIMIT);
  while(messageQueue.size() > 0) {
    BtMessageHandle msg = messageQueue.front();
    messageQueue.pop_front();
    if(uploadLimit > 0) {
      TransferStat stat = peerStorage->calculateStat();
      if(uploadLimit < stat.getUploadSpeed() &&
	 msg->isUploading() && !msg->isSendingInProgress()) {
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
  copy(tempQueue.begin(), tempQueue.end(), back_inserter(messageQueue));
}

// Cancel sending piece message to peer.
void DefaultBtMessageDispatcher::doCancelSendingPieceAction(uint32_t index, uint32_t begin, uint32_t blockLength)
{
  BtCancelSendingPieceEventHandle event =
    new BtCancelSendingPieceEvent(index, begin, blockLength);

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
      logger->debug("CUID#%d - Deleting request slot index=%d, blockIndex=%d",
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
    logger->debug("CUID#%d - Deleting request slot index=%d, blockIndex=%d"
		  " because localhost got choked.",
		  cuid,
		  slot.getIndex(),
		  slot.getBlockIndex());
    PieceHandle piece = pieceStorage->getPiece(slot.getIndex());
    piece->cancelBlock(slot.getBlockIndex());
    itr = requestSlots.erase(itr);
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
    if(slot.isTimeout(option->getAsInt(PREF_BT_REQUEST_TIMEOUT))) {
      logger->debug("CUID#%d - Deleting request slot blockIndex=%d"
		    " because of time out",
		    cuid,
		    slot.getBlockIndex());
      piece->cancelBlock(slot.getBlockIndex());
      peer->snubbing = true;
      itr = requestSlots.erase(itr);
    } else if(piece->hasBlock(slot.getBlockIndex())) {
      logger->debug("CUID#%d - Deleting request slot blockIndex=%d because"
		    " the block has been acquired.",
		    cuid,
		    slot.getBlockIndex());
      addMessageToQueue(BT_MESSAGE_FACTORY(btContext, peer)->
			createCancelMessage(slot.getIndex(),
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

uint32_t DefaultBtMessageDispatcher::countOutstandingRequest()
{
  return requestSlots.size();
}

bool DefaultBtMessageDispatcher::isOutstandingRequest(uint32_t index, uint32_t blockIndex) {
  for(RequestSlots::const_iterator itr = requestSlots.begin();
      itr != requestSlots.end(); itr++) {
    const RequestSlot& slot = *itr;
    if(slot.getIndex() == index && slot.getBlockIndex() == blockIndex) {
      return true;
    }
  }
  return false;
}
