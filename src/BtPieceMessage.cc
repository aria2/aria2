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
#include "MessageDigestHelper.h"
#include "DiskAdaptor.h"
#include "Logger.h"
#include "Peer.h"
#include "Piece.h"
#include "PieceStorage.h"
#include "BtMessageDispatcher.h"
#include "BtMessageFactory.h"
#include "BtRequestFactory.h"
#include "PeerConnection.h"
#include "StringFormat.h"
#include "DownloadContext.h"

namespace aria2 {

const std::string BtPieceMessage::NAME("piece");

BtPieceMessage::BtPieceMessage
(size_t index, uint32_t begin, size_t blockLength):
  AbstractBtMessage(ID, NAME),
  _index(index),
  _begin(begin),
  _blockLength(blockLength),
  _block(0),
  _rawData(0)
{
  setUploading(true);
}

BtPieceMessage::~BtPieceMessage()
{
  delete [] _rawData;
}

void BtPieceMessage::setRawMessage(unsigned char* data)
{
  delete [] _rawData;
  _rawData = data;
  _block = data+9;
}

BtPieceMessageHandle BtPieceMessage::create
(const unsigned char* data, size_t dataLength)
{
  bittorrent::assertPayloadLengthGreater(9, dataLength, NAME);
  bittorrent::assertID(ID, data, NAME);
  BtPieceMessageHandle message(new BtPieceMessage());
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
    (_index, _begin, _blockLength);
  getPeer()->updateDownloadLength(_blockLength);
  if(!RequestSlot::isNull(slot)) {
    getPeer()->snubbing(false);
    SharedHandle<Piece> piece = getPieceStorage()->getPiece(_index);
    off_t offset = (off_t)_index*_downloadContext->getPieceLength()+_begin;
    if(getLogger()->debug()) {
      getLogger()->debug(MSG_PIECE_RECEIVED,
                         util::itos(getCuid()).c_str(),
                         _index, _begin, _blockLength, offset,
                         slot.getBlockIndex());
    }
    getPieceStorage()->getDiskAdaptor()->writeData
      (_block, _blockLength, offset);
    piece->completeBlock(slot.getBlockIndex());
    if(getLogger()->debug()) {
      getLogger()->debug(MSG_PIECE_BITFIELD, util::itos(getCuid()).c_str(),
                         util::toHex(piece->getBitfield(),
                                     piece->getBitfieldLength()).c_str());
    }
    piece->updateHash(_begin, _block, _blockLength);
    getBtMessageDispatcher()->removeOutstandingRequest(slot);
    if(piece->pieceComplete()) {
      if(checkPieceHash(piece)) {
        onNewPiece(piece);
      } else {
        onWrongPiece(piece);
      }
    }
  } else {
    if(getLogger()->debug()) {
      getLogger()->debug("CUID#%s - RequestSlot not found, index=%d, begin=%d",
                         util::itos(getCuid()).c_str(), _index, _begin);
    }
  }
}

size_t BtPieceMessage::MESSAGE_HEADER_LENGTH = 13;

unsigned char* BtPieceMessage::createMessageHeader()
{
  /**
   * len --- 9+blockLength, 4bytes
   * id --- 7, 1byte
   * index --- index, 4bytes
   * begin --- begin, 4bytes
   * total: 13bytes
   */
  unsigned char* msgHeader = new unsigned char[MESSAGE_HEADER_LENGTH];
  bittorrent::createPeerMessageString(msgHeader, MESSAGE_HEADER_LENGTH,
                                      9+_blockLength, ID);
  bittorrent::setIntParam(&msgHeader[5], _index);
  bittorrent::setIntParam(&msgHeader[9], _begin);
  return msgHeader;
}

size_t BtPieceMessage::getMessageHeaderLength()
{
  return MESSAGE_HEADER_LENGTH;
}

void BtPieceMessage::send()
{
  if(isInvalidate()) {
    return;
  }
  size_t writtenLength;
  if(!isSendingInProgress()) {
    if(getLogger()->info()) {
      getLogger()->info(MSG_SEND_PEER_MESSAGE,
                        util::itos(getCuid()).c_str(),
                        getPeer()->getIPAddress().c_str(),
                        getPeer()->getPort(),
                        toString().c_str());
    }
    unsigned char* msgHdr = createMessageHeader();
    size_t msgHdrLen = getMessageHeaderLength();
    if(getLogger()->debug()) {
      getLogger()->debug("msglength = %lu bytes",
                         static_cast<unsigned long>(msgHdrLen+_blockLength));
    }
    getPeerConnection()->pushBytes(msgHdr, msgHdrLen);
    getPeerConnection()->sendPendingData();
    off_t pieceDataOffset =
      (off_t)_index*_downloadContext->getPieceLength()+_begin;
    writtenLength = sendPieceData(pieceDataOffset, _blockLength);
  } else {
    writtenLength = getPeerConnection()->sendPendingData();
  }
  getPeer()->updateUploadLength(writtenLength);
  setSendingInProgress(!getPeerConnection()->sendBufferIsEmpty());
}

size_t BtPieceMessage::sendPieceData(off_t offset, size_t length) const
{
  assert(length <= 16*1024);
  unsigned char* buf = new unsigned char[length];
  ssize_t r;
  try {
    r = getPieceStorage()->getDiskAdaptor()->readData(buf, length, offset);
  } catch(RecoverableException& e) {
    delete [] buf;
    throw;
  }
  if(r == static_cast<ssize_t>(length)) {
    getPeerConnection()->pushBytes(buf, length);
    return getPeerConnection()->sendPendingData();
  } else {
    throw DL_ABORT_EX(EX_DATA_READ);
  }
}

std::string BtPieceMessage::toString() const
{
  return strconcat(NAME, " index=", util::itos(_index), ", begin=",
                   util::itos(_begin), ", length=", util::itos(_blockLength));
}

bool BtPieceMessage::checkPieceHash(const SharedHandle<Piece>& piece)
{
  if(piece->isHashCalculated()) {
    if(getLogger()->debug()) {
      getLogger()->debug("Hash is available!! index=%lu",
                         static_cast<unsigned long>(piece->getIndex()));
    }
    return
      piece->getHashString()==_downloadContext->getPieceHash(piece->getIndex());
  } else {
    off_t offset = (off_t)piece->getIndex()*_downloadContext->getPieceLength();
    
    return MessageDigestHelper::staticSHA1Digest
      (getPieceStorage()->getDiskAdaptor(), offset, piece->getLength())
      == _downloadContext->getPieceHash(piece->getIndex());
  }
}

void BtPieceMessage::onNewPiece(const SharedHandle<Piece>& piece)
{
  if(getLogger()->info()) {
    getLogger()->info(MSG_GOT_NEW_PIECE,
                      util::itos(getCuid()).c_str(), piece->getIndex());
  }
  getPieceStorage()->completePiece(piece);
  getPieceStorage()->advertisePiece(getCuid(), piece->getIndex());
}

void BtPieceMessage::onWrongPiece(const SharedHandle<Piece>& piece)
{
  if(getLogger()->info()) {
    getLogger()->info(MSG_GOT_WRONG_PIECE,
                      util::itos(getCuid()).c_str(), piece->getIndex());
  }
  erasePieceOnDisk(piece);
  piece->clearAllBlock();
  piece->destroyHashContext();
  getBtRequestFactory()->removeTargetPiece(piece);
}

void BtPieceMessage::erasePieceOnDisk(const SharedHandle<Piece>& piece)
{
  size_t BUFSIZE = 4096;
  unsigned char buf[BUFSIZE];
  memset(buf, 0, BUFSIZE);
  off_t offset = (off_t)piece->getIndex()*_downloadContext->getPieceLength();
  div_t res = div(piece->getLength(), BUFSIZE);
  for(int i = 0; i < res.quot; ++i) {
    getPieceStorage()->getDiskAdaptor()->writeData(buf, BUFSIZE, offset);
    offset += BUFSIZE;
  }
  if(res.rem > 0) {
    getPieceStorage()->getDiskAdaptor()->writeData(buf, res.rem, offset);
  }
}

void BtPieceMessage::onChokingEvent(const BtChokingEvent& event)
{
  if(!isInvalidate() &&
     !isSendingInProgress() &&
     !getPeer()->isInAmAllowedIndexSet(_index)) {
    if(getLogger()->debug()) {
      getLogger()->debug(MSG_REJECT_PIECE_CHOKED,
                         util::itos(getCuid()).c_str(),
                         _index, _begin, _blockLength);
    }
    if(getPeer()->isFastExtensionEnabled()) {
      BtMessageHandle rej =
        getBtMessageFactory()->createRejectMessage
        (_index, _begin, _blockLength);
      getBtMessageDispatcher()->addMessageToQueue(rej);
    }
    setInvalidate(true);
  }
}

void BtPieceMessage::onCancelSendingPieceEvent
(const BtCancelSendingPieceEvent& event)
{
  if(!isInvalidate() &&
     !isSendingInProgress() &&
     _index == event.getIndex() &&
     _begin == event.getBegin() &&
     _blockLength == event.getLength()) {
    if(getLogger()->debug()) {
      getLogger()->debug(MSG_REJECT_PIECE_CANCEL,
                         util::itos(getCuid()).c_str(),
                         _index, _begin, _blockLength);
    }
    if(getPeer()->isFastExtensionEnabled()) {
      BtMessageHandle rej =
        getBtMessageFactory()->createRejectMessage
        (_index, _begin, _blockLength);
      getBtMessageDispatcher()->addMessageToQueue(rej);
    }
    setInvalidate(true);
  } 
}

void BtPieceMessage::setDownloadContext
(const SharedHandle<DownloadContext>& downloadContext)
{
  _downloadContext = downloadContext;
}

} // namespace aria2
