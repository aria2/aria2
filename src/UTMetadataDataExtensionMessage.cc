/* <!-- copyright */
/*
 * aria2 - The high speed download utility
 *
 * Copyright (C) 2009 Tatsuhiro Tsujikawa
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
#include "UTMetadataDataExtensionMessage.h"
#include "bencode2.h"
#include "util.h"
#include "a2functional.h"
#include "DownloadContext.h"
#include "UTMetadataRequestTracker.h"
#include "PieceStorage.h"
#include "BtConstants.h"
#include "MessageDigest.h"
#include "message_digest_helper.h"
#include "bittorrent_helper.h"
#include "DiskAdaptor.h"
#include "Piece.h"
#include "Logger.h"
#include "LogFactory.h"
#include "DlAbortEx.h"
#include "fmt.h"

namespace aria2 {

UTMetadataDataExtensionMessage::UTMetadataDataExtensionMessage(
    uint8_t extensionMessageID)
    : UTMetadataExtensionMessage{extensionMessageID},
      totalSize_{0},
      dctx_{nullptr},
      pieceStorage_{nullptr},
      tracker_{nullptr}
{
}

std::string UTMetadataDataExtensionMessage::getPayload()
{
  Dict dict;
  dict.put("msg_type", Integer::g(1));
  dict.put("piece", Integer::g(getIndex()));
  dict.put("total_size", Integer::g(totalSize_));
  return bencode2::encode(&dict) + data_;
}

std::string UTMetadataDataExtensionMessage::toString() const
{
  return fmt("ut_metadata data piece=%lu",
             static_cast<unsigned long>(getIndex()));
}

void UTMetadataDataExtensionMessage::doReceivedAction()
{
  if (tracker_->tracks(getIndex())) {
    A2_LOG_DEBUG(fmt("ut_metadata index=%lu found in tracking list",
                     static_cast<unsigned long>(getIndex())));
    tracker_->remove(getIndex());
    pieceStorage_->getDiskAdaptor()->writeData(
        reinterpret_cast<const unsigned char*>(data_.c_str()), data_.size(),
        getIndex() * METADATA_PIECE_SIZE);
    pieceStorage_->completePiece(pieceStorage_->getPiece(getIndex()));
    if (pieceStorage_->downloadFinished()) {
      std::string metadata = util::toString(pieceStorage_->getDiskAdaptor());
      unsigned char infoHash[INFO_HASH_LENGTH];
      message_digest::digest(infoHash, INFO_HASH_LENGTH,
                             MessageDigest::sha1().get(), metadata.data(),
                             metadata.size());
      if (memcmp(infoHash, bittorrent::getInfoHash(dctx_), INFO_HASH_LENGTH) ==
          0) {
        A2_LOG_INFO("Got ut_metadata");
      }
      else {
        A2_LOG_INFO("Got wrong ut_metadata");
        for (size_t i = 0; i < dctx_->getNumPieces(); ++i) {
          pieceStorage_->markPieceMissing(i);
        }
        throw DL_ABORT_EX("Got wrong ut_metadata");
      }
    }
  }
  else {
    A2_LOG_DEBUG(fmt("ut_metadata index=%lu is not tracked",
                     static_cast<unsigned long>(getIndex())));
  }
}

void UTMetadataDataExtensionMessage::setTotalSize(size_t totalSize)
{
  totalSize_ = totalSize;
}

size_t UTMetadataDataExtensionMessage::getTotalSize() const
{
  return totalSize_;
}

void UTMetadataDataExtensionMessage::setData(const std::string& data)
{
  data_ = data;
}

const std::string& UTMetadataDataExtensionMessage::getData() const
{
  return data_;
}

void UTMetadataDataExtensionMessage::setPieceStorage(PieceStorage* pieceStorage)
{
  pieceStorage_ = pieceStorage;
}

void UTMetadataDataExtensionMessage::setUTMetadataRequestTracker(
    UTMetadataRequestTracker* tracker)
{
  tracker_ = tracker;
}

void UTMetadataDataExtensionMessage::setDownloadContext(DownloadContext* dctx)
{
  dctx_ = dctx;
}

} // namespace aria2
