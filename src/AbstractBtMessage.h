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
#ifndef _D_ABSTRACT_BT_MESSAGE_H_
#define _D_ABSTRACT_BT_MESSAGE_H_

#include "BtMessage.h"
#include <deque>

namespace aria2 {

class BtContext;
class PieceStorage;
class Peer;
class BtMessageDispatcher;
class BtMessageFactory;
class BtRequestFactory;
class PeerConnection;
class BtMessageValidator;
class BtEventListener;
class Logger;

class AbstractBtMessage : public BtMessage {
protected:
  bool sendingInProgress;
  bool invalidate;
  bool uploading;
  int32_t cuid;

  SharedHandle<BtContext> btContext;

  SharedHandle<PieceStorage> pieceStorage;

  SharedHandle<Peer> peer;

  WeakHandle<BtMessageDispatcher> dispatcher;

  WeakHandle<BtMessageFactory> messageFactory;

  WeakHandle<BtRequestFactory> requestFactory;

  WeakHandle<PeerConnection> peerConnection;

  SharedHandle<BtMessageValidator> validator;

  std::deque<SharedHandle<BtEventListener> > listeners;

  Logger* logger;
public:
  AbstractBtMessage();

  virtual ~AbstractBtMessage();

  virtual bool isSendingInProgress() {
    return sendingInProgress;
  }

  void setSendingInProgress(bool sendingInProgress) {
    this->sendingInProgress = sendingInProgress;
  }

  virtual bool isInvalidate() {
    return invalidate;
  }

  void setInvalidate(bool invalidate) {
    this->invalidate = invalidate;
  }

  virtual bool isUploading() {
    return uploading;
  }

  void setUploading(bool uploading) {
    this->uploading = uploading;
  }

  int32_t getCuid() const {
    return cuid;
  }

  void setCuid(int32_t cuid) {
    this->cuid = cuid;
  }

  SharedHandle<Peer> getPeer() const;

  void setPeer(const SharedHandle<Peer>& peer);

  virtual void doReceivedAction() {}

  virtual bool validate(std::deque<std::string>& errors);
  
  virtual void onQueued() {}

  virtual void handleEvent(const SharedHandle<BtEvent>& event);

  void addEventListener(const SharedHandle<BtEventListener>& listener);

  void setBtMessageValidator(const SharedHandle<BtMessageValidator>& validator);

  SharedHandle<BtMessageValidator> getBtMessageValidator() const;

  void setBtContext(const SharedHandle<BtContext>& btContext);

  SharedHandle<BtContext> getBtContext() const;

  void setPieceStorage(const SharedHandle<PieceStorage>& pieceStorage);

  void setBtMessageDispatcher(const WeakHandle<BtMessageDispatcher>& dispatcher);

  void setPeerConnection(const WeakHandle<PeerConnection>& peerConnection);

  void setBtMessageFactory(const WeakHandle<BtMessageFactory>& factory);

  void setBtRequestFactory(const WeakHandle<BtRequestFactory>& factory);
};

typedef SharedHandle<AbstractBtMessage> AbstractBtMessageHandle;

} // namespace aria2

#endif // _D_ABSTRACT_BT_MESSAGE_H_
