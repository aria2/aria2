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
#ifndef D_BT_PIECE_MESSAGE_H
#define D_BT_PIECE_MESSAGE_H

#include "AbstractBtMessage.h"

namespace aria2 {

class Piece;
class BtPieceMessage;
class DownloadContext;
class PeerStorage;

typedef SharedHandle<BtPieceMessage> BtPieceMessageHandle;

class BtPieceMessage : public AbstractBtMessage {
private:
  size_t index_;
  int32_t begin_;
  int32_t blockLength_;
  const unsigned char* data_;
  SharedHandle<DownloadContext> downloadContext_;
  SharedHandle<PeerStorage> peerStorage_;

  static size_t MESSAGE_HEADER_LENGTH;

  bool checkPieceHash(const SharedHandle<Piece>& piece);

  void onNewPiece(const SharedHandle<Piece>& piece);

  void onWrongPiece(const SharedHandle<Piece>& piece);

  void erasePieceOnDisk(const SharedHandle<Piece>& piece);

  void pushPieceData(int64_t offset, int32_t length) const;
public:
  BtPieceMessage(size_t index = 0, int32_t begin = 0, int32_t blockLength = 0);

  virtual ~BtPieceMessage();

  static const uint8_t ID = 7;

  static const std::string NAME;

  size_t getIndex() const { return index_; }

  void setIndex(size_t index) { index_ = index; }

  int32_t getBegin() const { return begin_; }

  void setBegin(int32_t begin) { begin_ = begin; }

  const unsigned char* getBlock() const { return data_+9; }

  int32_t getBlockLength() const { return blockLength_; }

  // Sets message payload data. Caller must not change or free data
  // before doReceivedAction().
  void setMsgPayload(const unsigned char* data);

  void setBlockLength(int32_t blockLength) { blockLength_ = blockLength; }

  void setDownloadContext(const SharedHandle<DownloadContext>& downloadContext);

  void setPeerStorage(const SharedHandle<PeerStorage>& peerStorage);

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

#endif // D_BT_PIECE_MESSAGE_H
