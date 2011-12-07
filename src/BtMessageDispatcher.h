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
#ifndef D_BT_MESSAGE_DISPATCHER_H
#define D_BT_MESSAGE_DISPATCHER_H

#include "common.h"

#include <vector>

#include "SharedHandle.h"
#include "RequestSlot.h"

namespace aria2 {

class Piece;
class BtMessage;

class BtMessageDispatcher {
public:
  virtual ~BtMessageDispatcher() {}

  virtual void addMessageToQueue(const SharedHandle<BtMessage>& btMessage) = 0;

  virtual void
  addMessageToQueue(const std::vector<SharedHandle<BtMessage> >& btMessages) =0;

  virtual void sendMessages() = 0;

  virtual void doCancelSendingPieceAction
  (size_t index, int32_t begin, int32_t length) = 0;

  virtual void doCancelSendingPieceAction(const SharedHandle<Piece>& piece) = 0;

  virtual void doAbortOutstandingRequestAction(const SharedHandle<Piece>& piece) = 0;

  virtual void doChokedAction() = 0;

  virtual void doChokingAction() = 0;

  virtual void checkRequestSlotAndDoNecessaryThing() = 0;

  virtual bool isSendingInProgress() = 0;

  virtual size_t countMessageInQueue() = 0;

  virtual size_t countOutstandingRequest() = 0;

  virtual bool isOutstandingRequest(size_t index, size_t blockIndex) = 0;

  virtual RequestSlot getOutstandingRequest
  (size_t index, int32_t begin, int32_t length) = 0;

  virtual void removeOutstandingRequest(const RequestSlot& slot) = 0;

  virtual void addOutstandingRequest(const RequestSlot& slot) = 0;

  virtual size_t countOutstandingUpload() = 0;
};

typedef SharedHandle<BtMessageDispatcher> BtMessageDispatcherHandle;

} // namespace aria2

#endif // D_BT_MESSAGE_DISPATCHER_H
