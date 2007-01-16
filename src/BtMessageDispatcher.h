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
#ifndef _D_BT_MESSAGE_DISPATCHER_H_
#define _D_BT_MESSAGE_DISPATCHER_H_

#include "common.h"
#include "Piece.h"
#include "BtMessage.h"
#include "RequestSlot.h"

class BtMessageDispatcher {
public:
  virtual ~BtMessageDispatcher() {}

  virtual void addMessageToQueue(const BtMessageHandle& btMessage) = 0;

  virtual void addMessageToQueue(const BtMessages& btMessages) = 0;

  virtual void sendMessages() = 0;

  virtual void doCancelSendingPieceAction(int32_t index, int32_t begin, uint32_t length) = 0;

  virtual void doCancelSendingPieceAction(const PieceHandle& piece) = 0;

  virtual void doAbortOutstandingRequestAction(const PieceHandle& piece) = 0;

  virtual void doChokedAction() = 0;

  virtual void doChokingAction() = 0;

  virtual void checkRequestSlotAndDoNecessaryThing() = 0;

  virtual bool isSendingInProgress() = 0;

  virtual uint32_t countMessageInQueue() = 0;

  virtual uint32_t countOutstandingRequest() = 0;

  virtual bool isOutstandingRequest(int32_t index, int32_t blockIndex) = 0;

  virtual RequestSlot getOutstandingRequest(int32_t index, int32_t begin, uint32_t length) = 0;

  virtual void removeOutstandingRequest(const RequestSlot& slot) = 0;

  virtual void addOutstandingRequest(const RequestSlot& slot) = 0;
};

typedef SharedHandle<BtMessageDispatcher> BtMessageDispatcherHandle;
typedef WeakHandle<BtMessageDispatcher> BtMessageDispatcherWeakHandle;
#endif // _D_BT_MESSAGE_DISPATCHER_H_
