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
#include "BtPieceMessage.h"

#include <cstring>
#include <cstdlib>
#include <cassert>

#include "bittorrent_helper.h"
#include "util.h"
#include "message.h"
#include "DlAbortEx.h"
#include "message_digest_helper.h"
#include "DiskAdaptor.h"
#include "Logger.h"
#include "LogFactory.h"
#include "Peer.h"
#include "Piece.h"
#include "PieceStorage.h"
#include "BtMessageDispatcher.h"
#include "BtMessageFactory.h"
#include "BtRequestFactory.h"
#include "PeerConnection.h"
#include "fmt.h"
#include "DownloadContext.h"
#include "PeerStorage.h"
#include "array_fun.h"
#include "WrDiskCache.h"
#include "WrDiskCacheEntry.h"
#include "DownloadFailureException.h"

namespace aria2 {

const char BtPieceMessage::NAME[] = "piece";

BtPieceMessage::BtPieceMessage
(size_t index, int32_t begin, int32_t blockLength)
  : AbstractBtMessage(ID, NAME),
    index_(index),
    begin_(begin),
    blockLength_(blockLength),
    data_(0)
{
  setUploading(true);
}

BtPieceMessage::~BtPieceMessage()
{}

void BtPieceMessage::setMsgPayload(const unsigned char* data)
{
  data_ = data;
}

BtPieceMessage* BtPieceMessage::create
(const unsigned char* data, size_t dataLength)
{
  bittorrent::assertPayloadLengthGreater(9, dataLength, NAME);
  bittorrent::assertID(ID, data, NAME);
  BtPieceMessage* message(new BtPieceMessage());
  message->setIndex(bittorrent::getIntParam(data, 1));
  message->setBegin(bittorrent::getIntParam(data, 5));
  message->setBlockLength(dataLength-9);
  return message;
}

void BtPieceMessage::doReceivedAction()
{
  if(isMetadataGetMode()) {
    return;
  }
  RequestSlot slot = getBtMessageDispatcher()->getOutstandingRequest
    (index_, begin_, blockLength_);
  getPeer()->updateDownloadLength(blockLength_);
  downloadContext_->updateDownloadLength(blockLength_);
  if(!RequestSlot::isNull(slot)) {
    getPeer()->snubbing(false);
    SharedHandle<Piece> piece = getPieceStorage()->getPiece(index_);
    int64_t offset =
      static_cast<int64_t>(index_)*downloadContext_->getPieceLength()+begin_;
    A2_LOG_DEBUG(fmt(MSG_PIECE_RECEIVED,
                     getCuid(),
                     static_cast<unsigned long>(index_),
                     begin_,
                     blockLength_,
                     static_cast<int64_t>(offset),
                     static_cast<unsigned long>(slot.getBlockIndex())));
    if(piece->hasBlock(slot.getBlockIndex())) {
      A2_LOG_DEBUG("Already have this block.");
      return;
    }
    if(piece->getWrDiskCacheEntry()) {
      // Write Disk Cache enabled. Unfortunately, it incurs extra data
      // copy.
      unsigned char* dataCopy = new unsigned char[blockLength_];
      memcpy(dataCopy, data_+9, blockLength_);
      piece->updateWrCache(getPieceStorage()->getWrDiskCache(),
                           dataCopy, 0, blockLength_, blockLength_, offset);
    } else {
      getPieceStorage()->getDiskAdaptor()->writeData(data_+9, blockLength_,
                                                     offset);
    }
    piece->completeBlock(slot.getBlockIndex());
    A2_LOG_DEBUG(fmt(MSG_PIECE_BITFIELD, getCuid(),
                     util::toHex(piece->getBitfield(),
                                 piece->getBitfieldLength()).c_str()));
    piece->updateHash(begin_, data_+9, blockLength_);
    getBtMessageDispatcher()->removeOutstandingRequest(slot);
    if(piece->pieceComplete()) {
      if(checkPieceHash(piece)) {
        onNewPiece(piece);
      } else {
        onWrongPiece(piece);
        peerStorage_->addBadPeer(getPeer()->getIPAddress());
        throw DL_ABORT_EX("Bad piece hash.");
      }
    }
  } else {
    A2_LOG_DEBUG(fmt("CUID#%" PRId64 " - RequestSlot not found, index=%lu, begin=%d",
                     getCuid(),
                     static_cast<unsigned long>(index_),
                     begin_));
  }
}

size_t BtPieceMessage::MESSAGE_HEADER_LENGTH = 13;

void BtPieceMessage::createMessageHeader(unsigned char* msgHeader) const
{
  /**
   * len --- 9+blockLength, 4bytes
   * id --- 7, 1byte
   * index --- index, 4bytes
   * begin --- begin, 4bytes
   * total: 13bytes
   */
  bittorrent::createPeerMessageString(msgHeader, MESSAGE_HEADER_LENGTH,
                                      9+blockLength_, ID);
  bittorrent::setIntParam(&msgHeader[5], index_);
  bittorrent::setIntParam(&msgHeader[9], begin_);
}

size_t BtPieceMessage::getMessageHeaderLength()
{
  return MESSAGE_HEADER_LENGTH;
}

namespace {
struct PieceSendUpdate : public ProgressUpdate {
  PieceSendUpdate(const SharedHandle<Peer>& peer, size_t headerLength)
    : peer(peer), headerLength(headerLength) {}
  virtual void update(size_t length, bool complete)
  {
    if(headerLength > 0) {
      size_t m = std::min(headerLength, length);
      headerLength -= m;
      length -= m;
    }
    peer->updateUploadLength(length);
  }
  SharedHandle<Peer> peer;
  size_t headerLength;
};
} // namespace

void BtPieceMessage::send()
{
  if(isInvalidate()) {
    return;
  }
  A2_LOG_INFO(fmt(MSG_SEND_PEER_MESSAGE,
                  getCuid(),
                  getPeer()->getIPAddress().c_str(),
                  getPeer()->getPort(),
                  toString().c_str()));
  int64_t pieceDataOffset =
    static_cast<int64_t>(index_)*downloadContext_->getPieceLength()+begin_;
  pushPieceData(pieceDataOffset, blockLength_);
}

void BtPieceMessage::pushPieceData(int64_t offset, int32_t length) const
{
  assert(length <= 16*1024);
  array_ptr<unsigned char> buf
    (new unsigned char[length+MESSAGE_HEADER_LENGTH]);
  createMessageHeader(buf);
  ssize_t r;
  r = getPieceStorage()->getDiskAdaptor()->readData(buf+MESSAGE_HEADER_LENGTH,
                                                    length, offset);
  if(r == length) {
    unsigned char* dbuf = buf;
    buf.reset(0);
    getPeerConnection()->pushBytes(dbuf, length+MESSAGE_HEADER_LENGTH,
                                   new PieceSendUpdate(getPeer(),
                                                       MESSAGE_HEADER_LENGTH));
    // To avoid upload rate overflow, we update the length here at
    // once.
    downloadContext_->updateUploadLength(length);
  } else {
    throw DL_ABORT_EX(EX_DATA_READ);
  }
}

std::string BtPieceMessage::toString() const
{
  return fmt("%s index=%lu, begin=%d, length=%d",
             NAME,
             static_cast<unsigned long>(index_),
             begin_, blockLength_);
}

bool BtPieceMessage::checkPieceHash(const SharedHandle<Piece>& piece)
{
  if(!getPieceStorage()->isEndGame() && piece->isHashCalculated()) {
    A2_LOG_DEBUG(fmt("Hash is available!! index=%lu",
                     static_cast<unsigned long>(piece->getIndex())));
    return
      piece->getDigest() == downloadContext_->getPieceHash(piece->getIndex());
  } else {
    A2_LOG_DEBUG(fmt("Calculating hash index=%lu",
                     static_cast<unsigned long>(piece->getIndex())));
    try {
      return piece->getDigestWithWrCache(downloadContext_->getPieceLength(),
                                         getPieceStorage()->getDiskAdaptor())
        == downloadContext_->getPieceHash(piece->getIndex());
    } catch(RecoverableException& e) {
      piece->clearAllBlock(getPieceStorage()->getWrDiskCache());
      throw;
    }
  }
}

void BtPieceMessage::onNewPiece(const SharedHandle<Piece>& piece)
{
  if(piece->getWrDiskCacheEntry()) {
    // We flush cached data whenever an whole piece is retrieved.
     piece->flushWrCache(getPieceStorage()->getWrDiskCache());
     if(piece->getWrDiskCacheEntry()->getError() !=
        WrDiskCacheEntry::CACHE_ERR_SUCCESS) {
       piece->clearAllBlock(getPieceStorage()->getWrDiskCache());
       throw DOWNLOAD_FAILURE_EXCEPTION2
         (fmt("Write disk cache flush failure index=%lu",
              static_cast<unsigned long>(piece->getIndex())),
          piece->getWrDiskCacheEntry()->getErrorCode());
     }
  }
  A2_LOG_INFO(fmt(MSG_GOT_NEW_PIECE,
                  getCuid(),
                  static_cast<unsigned long>(piece->getIndex())));
  getPieceStorage()->completePiece(piece);
  getPieceStorage()->advertisePiece(getCuid(), piece->getIndex());
}

void BtPieceMessage::onWrongPiece(const SharedHandle<Piece>& piece)
{
  A2_LOG_INFO(fmt(MSG_GOT_WRONG_PIECE,
                  getCuid(),
                  static_cast<unsigned long>(piece->getIndex())));
  piece->clearAllBlock(getPieceStorage()->getWrDiskCache());
  piece->destroyHashContext();
  getBtRequestFactory()->removeTargetPiece(piece);
}

void BtPieceMessage::onChokingEvent(const BtChokingEvent& event)
{
  if(!isInvalidate() &&
     !getPeer()->isInAmAllowedIndexSet(index_)) {
    A2_LOG_DEBUG(fmt(MSG_REJECT_PIECE_CHOKED,
                     getCuid(),
                     static_cast<unsigned long>(index_),
                     begin_,
                     blockLength_));
    if(getPeer()->isFastExtensionEnabled()) {
      SharedHandle<BtMessage> rej =
        getBtMessageFactory()->createRejectMessage
        (index_, begin_, blockLength_);
      getBtMessageDispatcher()->addMessageToQueue(rej);
    }
    setInvalidate(true);
  }
}

void BtPieceMessage::onCancelSendingPieceEvent
(const BtCancelSendingPieceEvent& event)
{
  if(!isInvalidate() &&
     index_ == event.getIndex() &&
     begin_ == event.getBegin() &&
     blockLength_ == event.getLength()) {
    A2_LOG_DEBUG(fmt(MSG_REJECT_PIECE_CANCEL,
                     getCuid(),
                     static_cast<unsigned long>(index_),
                     begin_,
                     blockLength_));
    if(getPeer()->isFastExtensionEnabled()) {
      SharedHandle<BtMessage> rej =
        getBtMessageFactory()->createRejectMessage
        (index_, begin_, blockLength_);
      getBtMessageDispatcher()->addMessageToQueue(rej);
    }
    setInvalidate(true);
  }
}

void BtPieceMessage::setDownloadContext
(const SharedHandle<DownloadContext>& downloadContext)
{
  downloadContext_ = downloadContext;
}

void BtPieceMessage::setPeerStorage
(const SharedHandle<PeerStorage>& peerStorage)
{
  peerStorage_ = peerStorage;
}

} // namespace aria2
