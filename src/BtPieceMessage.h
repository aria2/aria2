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
#ifndef _D_BT_PIECE_MESSAGE_H_
#define _D_BT_PIECE_MESSAGE_H_

#include "AbstractBtMessage.h"
#include "AbstractBtEventListener.h"

namespace aria2 {

class BtEvent;
class Piece;
class BtPieceMessage;

typedef SharedHandle<BtPieceMessage> BtPieceMessageHandle;

class BtPieceMessage : public AbstractBtMessage {
private:
  size_t index;
  uint32_t begin;
  uint32_t blockLength;
  unsigned char* block;
  size_t leftDataLength;
  bool headerSent;
  unsigned char* msgHeader;

  static size_t MESSAGE_HEADER_LENGTH;

  bool checkPieceHash(const SharedHandle<Piece>& piece);

  void onNewPiece(const SharedHandle<Piece>& piece);

  void onWrongPiece(const SharedHandle<Piece>& piece);

  void erasePieceOnDisk(const SharedHandle<Piece>& piece);

  size_t sendPieceData(off_t offset, size_t length) const;

  class BtChokingEventListener : public AbstractBtEventListener {
  private:
    BtPieceMessage* message;
  public:
    BtChokingEventListener(BtPieceMessage* message):message(message) {}

    virtual bool canHandle(const SharedHandle<BtEvent>& btEvent);

    virtual void handleEventInternal(const SharedHandle<BtEvent>& btEvent);
  };

  typedef SharedHandle<BtChokingEventListener> BtChokingEventListenerHandle;

  class BtCancelSendingPieceEventListener : public AbstractBtEventListener {
  private:
    BtPieceMessage* message;
  public:
    BtCancelSendingPieceEventListener(BtPieceMessage* message):message(message) {}

    virtual bool canHandle(const SharedHandle<BtEvent>& btEvent);

    virtual void handleEventInternal(const SharedHandle<BtEvent>& btEvent);
  };

  typedef SharedHandle<BtCancelSendingPieceEventListener> BtCancelSendingPieceEventListenerHandle;
public:
  BtPieceMessage(size_t index = 0, uint32_t begin = 0, size_t blockLength = 0)
    :index(index),
     begin(begin),
     blockLength(blockLength),
     block(0),
     leftDataLength(0),
     headerSent(false),
     msgHeader(0)
  {
    uploading = true;
    addEventListener(new BtChokingEventListener(this));
    addEventListener(new BtCancelSendingPieceEventListener(this));
  }

  virtual ~BtPieceMessage() {
    delete [] msgHeader;
    delete []  block;
  }

  static const uint8_t ID = 7;

  size_t getIndex() const { return index; }

  void setIndex(size_t index) { this->index = index; }

  uint32_t getBegin() const { return begin; }

  void setBegin(uint32_t begin) { this->begin = begin; }

  const unsigned char* getBlock() const { return block; }

  void setBlock(const unsigned char* block, size_t blockLength);

  size_t getBlockLength() const { return blockLength; }

  void setBlockLength(size_t blockLength) { this->blockLength = blockLength; }

  static BtPieceMessageHandle create(const unsigned char* data, size_t dataLength);

  virtual uint8_t getId() { return ID; }

  virtual void doReceivedAction();

  const unsigned char* getMessageHeader();

  size_t getMessageHeaderLength();

  virtual void send();

  virtual std::string toString() const;

  void handleChokingEvent(const SharedHandle<BtEvent>& event);
  
  void handleCancelSendingPieceEvent(const SharedHandle<BtEvent>& event);
};

} // namespace aria2

#endif // _D_BT_PIECE_MESSAGE_H_
