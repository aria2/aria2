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
#ifndef D_DEFAULT_BT_MESSAGE_DISPATCHER_H
#define D_DEFAULT_BT_MESSAGE_DISPATCHER_H

#include "BtMessageDispatcher.h"

#include <deque>

#include "a2time.h"
#include "Command.h"

namespace aria2 {

class DownloadContext;
class BtMessage;
class BtMessageFactory;
class Peer;
class Piece;
class RequestGroupMan;
class PeerConnection;

class DefaultBtMessageDispatcher : public BtMessageDispatcher {
private:
  cuid_t cuid_;
  std::deque<std::unique_ptr<BtMessage>> messageQueue_;
  std::deque<std::unique_ptr<RequestSlot>> requestSlots_;
  DownloadContext* downloadContext_;
  PeerConnection* peerConnection_;
  BtMessageFactory* messageFactory_;
  std::shared_ptr<Peer> peer_;
  RequestGroupMan* requestGroupMan_;
  std::chrono::seconds requestTimeout_;

public:
  DefaultBtMessageDispatcher();

  virtual ~DefaultBtMessageDispatcher();

  virtual void
  addMessageToQueue(std::unique_ptr<BtMessage> btMessage) CXX11_OVERRIDE;

  virtual void sendMessages() CXX11_OVERRIDE;

  // For unit tests without PeerConnection
  void sendMessagesInternal();

  virtual void doCancelSendingPieceAction(size_t index, int32_t begin,
                                          int32_t length) CXX11_OVERRIDE;

  virtual void doCancelSendingPieceAction(const std::shared_ptr<Piece>& piece)
      CXX11_OVERRIDE;

  virtual void doAbortOutstandingRequestAction(
      const std::shared_ptr<Piece>& piece) CXX11_OVERRIDE;

  virtual void doChokedAction() CXX11_OVERRIDE;

  virtual void doChokingAction() CXX11_OVERRIDE;

  virtual void checkRequestSlotAndDoNecessaryThing() CXX11_OVERRIDE;

  virtual bool isSendingInProgress() CXX11_OVERRIDE;

  virtual size_t countMessageInQueue() CXX11_OVERRIDE
  {
    return messageQueue_.size();
  }

  virtual size_t countOutstandingRequest() CXX11_OVERRIDE
  {
    return requestSlots_.size();
  }

  virtual bool isOutstandingRequest(size_t index,
                                    size_t blockIndex) CXX11_OVERRIDE;

  virtual const RequestSlot*
  getOutstandingRequest(size_t index, int32_t begin,
                        int32_t length) CXX11_OVERRIDE;

  virtual void removeOutstandingRequest(const RequestSlot* slot) CXX11_OVERRIDE;

  virtual void addOutstandingRequest(std::unique_ptr<RequestSlot> requestSlot)
      CXX11_OVERRIDE;

  virtual size_t countOutstandingUpload() CXX11_OVERRIDE;

  const std::deque<std::unique_ptr<BtMessage>>& getMessageQueue() const
  {
    return messageQueue_;
  }

  const std::deque<std::unique_ptr<RequestSlot>>& getRequestSlots() const
  {
    return requestSlots_;
  }

  void setPeer(const std::shared_ptr<Peer>& peer);

  void setDownloadContext(DownloadContext* downloadContext);

  void setBtMessageFactory(BtMessageFactory* factory);

  void setRequestGroupMan(RequestGroupMan* rgman);

  void setCuid(cuid_t cuid) { cuid_ = cuid; }

  void setRequestTimeout(std::chrono::seconds requestTimeout)
  {
    requestTimeout_ = std::move(requestTimeout);
  }

  void setPeerConnection(PeerConnection* peerConnection)
  {
    peerConnection_ = peerConnection;
  }
};

} // namespace aria2

#endif // D_DEFAULT_BT_MESSAGE_DISPATCHER_H
