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
#include "Peer.h"
#include "Piece.h"
#include "LogFactory.h"
#include "Logger.h"
#include "BtEvent.h"
#include "BtEventListener.h"
#include "BtContext.h"
#include "BtRegistry.h"

class AbstractBtMessage : public BtMessage {
protected:
  bool sendingInProgress;
  bool invalidate;
  bool uploading;
  int32_t cuid;

  BtContextHandle btContext;

  PieceStorageHandle pieceStorage;

  PeerHandle peer;

  BtMessageValidatorHandle validator;
  BtEventListeners listeners;
  const Logger* logger;
public:
  AbstractBtMessage():sendingInProgress(false),
		      invalidate(false),
		      uploading(false),
		      cuid(0),
		      btContext(0),
		      pieceStorage(0),
		      peer(0),
		      validator(0),
		      logger(LogFactory::getInstance())
		      
  {}

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

  PeerHandle getPeer() const {
    return peer;
  }

  void setPeer(const PeerHandle& peer) {
    this->peer = peer;
  }

  virtual void doReceivedAction() {}

  virtual bool validate(Errors& errors) {
    if(validator.get()) {
      return validator->validate(errors);
    } else {
      return true;
    }
  }
  
  virtual void onQueued() {}

  virtual void handleEvent(const BtEventHandle& event) {
    for(BtEventListeners::iterator itr = listeners.begin();
	itr != listeners.end(); ++itr) {
      (*itr)->handleEvent(event);
    }
  }

  void addEventListener(const BtEventListenerHandle& listener) {
    listeners.push_back(listener);
  }

  void setBtMessageValidator(const BtMessageValidatorHandle& validator) {
    this->validator = validator;
  }

  BtMessageValidatorHandle getBtMessageValidator() const {
    return validator;
  }

  void setBtContext(const BtContextHandle& btContext) {
    this->btContext = btContext;
    this->pieceStorage = PIECE_STORAGE(btContext);
  }

  BtContextHandle getBtContext() const {
    return btContext;
  }

};

typedef SharedHandle<AbstractBtMessage> AbstractBtMessageHandle;

#endif // _D_ABSTRACT_BT_MESSAGE_H_
