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
#ifndef _D_DEFAULT_BT_MESSAGE_DISPATCHER_H_
#define _D_DEFAULT_BT_MESSAGE_DISPATCHER_H_

#include "BtMessageDispatcher.h"
#include "BtContext.h"
#include "PeerStorage.h"
#include "PieceStorage.h"
#include "RequestSlot.h"
#include "BtMessage.h"
#include "Peer.h"
#include "Option.h"
#include "LogFactory.h"
#include "Logger.h"
#include "BtRegistry.h"

class DefaultBtMessageDispatcher : public BtMessageDispatcher {
private:
  int32_t cuid;
  BtMessages messageQueue;
  RequestSlots requestSlots;
  BtContextHandle btContext;
  PeerStorageHandle peerStorage;
  PieceStorageHandle pieceStorage;
  PeerHandle peer;
  const Option* option;
  const Logger* logger;
public:
  DefaultBtMessageDispatcher():
    cuid(0),
    btContext(0),
    peerStorage(0),
    pieceStorage(0),
    peer(0),
    option(0),
    logger(LogFactory::getInstance())
  {
    logger->debug("DefaultBtMessageDispatcher::instantiated");
  }

  virtual ~DefaultBtMessageDispatcher()
  {
    logger->debug("DefaultBtMessageDispatcher::deleted");
  }

  virtual void addMessageToQueue(const BtMessageHandle& btMessage);

  virtual void addMessageToQueue(const BtMessages& btMessages);

  virtual void sendMessages();

  virtual void doCancelSendingPieceAction(int32_t index, int32_t begin, uint32_t length);

  virtual void doCancelSendingPieceAction(const PieceHandle& piece);

  virtual void doAbortOutstandingRequestAction(const PieceHandle& piece);

  virtual void doChokedAction();

  virtual void doChokingAction();

  virtual void checkRequestSlotAndDoNecessaryThing();

  virtual bool isSendingInProgress();

  virtual uint32_t countMessageInQueue() {
    return messageQueue.size();
  }

  virtual uint32_t countOutstandingRequest();

  virtual bool isOutstandingRequest(int32_t index, int32_t blockIndex);

  virtual RequestSlot getOutstandingRequest(int32_t index, int32_t begin, uint32_t length) {
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

  virtual void removeOutstandingRequest(const RequestSlot& slot) {
    RequestSlots temp;
    remove_copy(requestSlots.begin(), requestSlots.end(), back_inserter(temp), slot);
    requestSlots = temp;
  }

  virtual void addOutstandingRequest(const RequestSlot& requestSlot) {
    if(!isOutstandingRequest(requestSlot.getIndex(), requestSlot.getBlockIndex())) {
      requestSlots.push_back(requestSlot);
    }
  }

  BtMessages& getMessageQueue() {
    return messageQueue;
  }

  void setOption(const Option* option) {
    this->option = option;
  }

  const Option* getOption() const {
    return option;
  }

  RequestSlots& getRequestSlots() {
    return requestSlots;
  }

  void setPeer(const PeerHandle& peer) {
    this->peer = peer;
  }

  void setBtContext(const BtContextHandle& btContext) {
    this->btContext = btContext;
    this->pieceStorage = PIECE_STORAGE(btContext);
    this->peerStorage = PEER_STORAGE(btContext);
  }

  void setCuid(int32_t cuid) {
    this->cuid = cuid;
  }
};

typedef SharedHandle<DefaultBtMessageDispatcher> DefaultBtMessageDispatcherHandle;
#endif // _D_DEFAULT_BT_MESSAGE_DISPATCHER_H_
