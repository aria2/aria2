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
#ifndef _D_BT_REQUEST_MESSAGE_H_
#define _D_BT_REQUEST_MESSAGE_H_

#include "SimpleBtMessage.h"
#include "BtContext.h"
#include "PieceStorage.h"
#include "AbstractBtEventListener.h"

class BtRequestMessage;

typedef SharedHandle<BtRequestMessage> BtRequestMessageHandle;

class BtRequestMessage : public SimpleBtMessage {
private:
  uint32_t index;
  uint32_t begin;
  uint32_t length;
  uint32_t blockIndex;
  char* msg;

  static uint32_t MESSAGE_LENGTH;

  class BtAbortOutstandingRequestEventListener : public AbstractBtEventListener {
  private:
    BtRequestMessage* message;
  public:
    BtAbortOutstandingRequestEventListener(BtRequestMessage* message):message(message) {}

    virtual bool canHandle(const BtEventHandle& event);

    virtual void handleEventInternal(const BtEventHandle& event);
  };

  typedef SharedHandle<BtAbortOutstandingRequestEventListener> BtAbortOutstandingRequestEventListenerHandle;
public:
  BtRequestMessage(uint32_t index = 0,
		   uint32_t begin = 0,
		   uint32_t length = 0,
		   uint32_t blockIndex = 0)
    :index(index),
     begin(begin),
     length(length),
     blockIndex(blockIndex),
     msg(0)
  {
    addEventListener(new BtAbortOutstandingRequestEventListener(this));
  }

  virtual ~BtRequestMessage() {
    delete [] msg;
  }

  enum ID_t {
    ID = 6
  };

  uint32_t getIndex() const { return index; }
  void setIndex(uint32_t index) { this->index = index; }
  uint32_t getBegin() const { return begin; }
  void setBegin(uint32_t begin) { this->begin = begin; }
  uint32_t getLength() const { return length; }
  void setLength(uint32_t length) { this->length = length; }
  uint32_t getBlockIndex() const { return blockIndex; }
  void setBlockIndex(uint32_t blockIndex) { this->blockIndex = blockIndex; }

  static BtRequestMessageHandle create(const unsigned char* data, uint32_t dataLength);

  virtual int32_t getId() const { return ID; }

  virtual void doReceivedAction();

  virtual const char* getMessage();

  virtual uint32_t getMessageLength();

  virtual string toString() const;

  virtual void onQueued();

  virtual void handleAbortOutstandingRequestEvent(const BtEventHandle& event);
};

#endif // _D_BT_REQUEST_MESSAGE_H_
