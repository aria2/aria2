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
#ifndef _D_BT_PIECE_MESSAGE_H_
#define _D_BT_PIECE_MESSAGE_H_

#include "AbstractBtMessage.h"

namespace aria2 {

class Piece;
class BtPieceMessage;
class DownloadContext;

typedef SharedHandle<BtPieceMessage> BtPieceMessageHandle;

class BtPieceMessage : public AbstractBtMessage {
private:
  size_t _index;
  uint32_t _begin;
  uint32_t _blockLength;
  unsigned char* _block;
  unsigned char* _rawData;
  SharedHandle<DownloadContext> _downloadContext;

  static size_t MESSAGE_HEADER_LENGTH;

  bool checkPieceHash(const SharedHandle<Piece>& piece);

  void onNewPiece(const SharedHandle<Piece>& piece);

  void onWrongPiece(const SharedHandle<Piece>& piece);

  void erasePieceOnDisk(const SharedHandle<Piece>& piece);

  size_t sendPieceData(off_t offset, size_t length) const;
public:
  BtPieceMessage(size_t index = 0, uint32_t begin = 0, size_t blockLength = 0);

  virtual ~BtPieceMessage();

  static const uint8_t ID = 7;

  static const std::string NAME;

  size_t getIndex() const { return _index; }

  void setIndex(size_t index) { _index = index; }

  uint32_t getBegin() const { return _begin; }

  void setBegin(uint32_t begin) { _begin = begin; }

  const unsigned char* getBlock() const { return _block; }

  size_t getBlockLength() const { return _blockLength; }

  // Stores raw message data. After this function call, this object
  // has ownership of data. Caller must not be free or alter data.
  // Member block is pointed to block starting position in data.
  void setRawMessage(unsigned char* data);

  void setBlockLength(size_t blockLength) { _blockLength = blockLength; }

  void setDownloadContext(const SharedHandle<DownloadContext>& downloadContext);

  static BtPieceMessageHandle create
  (const unsigned char* data, size_t dataLength);

  virtual void doReceivedAction();

  unsigned char* createMessageHeader();

  size_t getMessageHeaderLength();

  virtual void send();

  virtual std::string toString() const;

  virtual void onChokingEvent(const BtChokingEvent& event);
  
  virtual void onCancelSendingPieceEvent
  (const BtCancelSendingPieceEvent& event);
};

} // namespace aria2

#endif // _D_BT_PIECE_MESSAGE_H_
