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
#ifndef _D_ABSTRACT_BT_MESSAGE_H_
#define _D_ABSTRACT_BT_MESSAGE_H_

#include "BtMessage.h"
#include "Command.h"

namespace aria2 {

class PieceStorage;
class Peer;
class BtMessageDispatcher;
class BtMessageFactory;
class BtRequestFactory;
class PeerConnection;
class BtMessageValidator;
class Logger;

class AbstractBtMessage : public BtMessage {
private:
  bool sendingInProgress_;
  bool invalidate_;
  bool uploading_;
  cuid_t cuid_;

  std::string name_;

  SharedHandle<PieceStorage> pieceStorage_;

  SharedHandle<Peer> peer_;

  WeakHandle<BtMessageDispatcher> dispatcher_;

  WeakHandle<BtMessageFactory> messageFactory_;

  WeakHandle<BtRequestFactory> requestFactory_;

  WeakHandle<PeerConnection> peerConnection_;

  SharedHandle<BtMessageValidator> validator_;

  bool metadataGetMode_;

  Logger* logger_;
protected:
  Logger* getLogger() const
  {
    return logger_;
  }

  const SharedHandle<PieceStorage>& getPieceStorage() const
  {
    return pieceStorage_;
  }

  const WeakHandle<PeerConnection>& getPeerConnection() const
  {
    return peerConnection_;
  }

  const WeakHandle<BtMessageDispatcher>& getBtMessageDispatcher() const
  {
    return dispatcher_;
  }

  const WeakHandle<BtRequestFactory>& getBtRequestFactory() const
  {
    return requestFactory_;
  }

  const WeakHandle<BtMessageFactory>& getBtMessageFactory() const
  {
    return messageFactory_;
  }

  bool isMetadataGetMode() const
  {
    return metadataGetMode_;
  }
public:
  AbstractBtMessage(uint8_t id, const std::string& name);

  virtual ~AbstractBtMessage();

  virtual bool isSendingInProgress() {
    return sendingInProgress_;
  }

  void setSendingInProgress(bool sendingInProgress) {
    sendingInProgress_ = sendingInProgress;
  }

  virtual bool isInvalidate() {
    return invalidate_;
  }

  void setInvalidate(bool invalidate) {
    invalidate_ = invalidate;
  }

  virtual bool isUploading() {
    return uploading_;
  }

  void setUploading(bool uploading) {
    uploading_ = uploading;
  }

  cuid_t getCuid() const {
    return cuid_;
  }

  void setCuid(cuid_t cuid) {
    cuid_ = cuid;
  }

  const SharedHandle<Peer>& getPeer() const
  {
    return peer_;
  }

  void setPeer(const SharedHandle<Peer>& peer);

  virtual void doReceivedAction() {}

  virtual void validate();
  
  virtual void onQueued() {}

  virtual void onAbortOutstandingRequestEvent
  (const BtAbortOutstandingRequestEvent& event) {}

  virtual void onCancelSendingPieceEvent
  (const BtCancelSendingPieceEvent& event) {}

  virtual void onChokingEvent(const BtChokingEvent& event) {}

  void setBtMessageValidator(const SharedHandle<BtMessageValidator>& validator);

  void setPieceStorage(const SharedHandle<PieceStorage>& pieceStorage);

  void setBtMessageDispatcher
  (const WeakHandle<BtMessageDispatcher>& dispatcher);

  void setPeerConnection(const WeakHandle<PeerConnection>& peerConnection);

  void setBtMessageFactory(const WeakHandle<BtMessageFactory>& factory);

  void setBtRequestFactory(const WeakHandle<BtRequestFactory>& factory);

  const std::string& getName() const
  {
    return name_;
  }

  void enableMetadataGetMode()
  {
    metadataGetMode_ = true;
  }
};

typedef SharedHandle<AbstractBtMessage> AbstractBtMessageHandle;

} // namespace aria2

#endif // _D_ABSTRACT_BT_MESSAGE_H_
