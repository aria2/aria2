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
#include "UTMetadataRequestExtensionMessage.h"
#include "bencode2.h"
#include "util.h"
#include "a2functional.h"
#include "bittorrent_helper.h"
#include "DlAbortEx.h"
#include "fmt.h"
#include "BtMessageFactory.h"
#include "BtMessageDispatcher.h"
#include "Peer.h"
#include "UTMetadataRejectExtensionMessage.h"
#include "UTMetadataDataExtensionMessage.h"
#include "BtConstants.h"
#include "DownloadContext.h"
#include "BtMessage.h"
#include "PieceStorage.h"

namespace aria2 {

UTMetadataRequestExtensionMessage::UTMetadataRequestExtensionMessage
(uint8_t extensionMessageID):UTMetadataExtensionMessage(extensionMessageID),
                             dispatcher_(0),
                             messageFactory_(0)
{}

UTMetadataRequestExtensionMessage::~UTMetadataRequestExtensionMessage() {}

std::string UTMetadataRequestExtensionMessage::getPayload()
{
  Dict dict;
  dict.put("msg_type", Integer::g(0));
  dict.put("piece", Integer::g(getIndex()));
  return bencode2::encode(&dict);
}

std::string UTMetadataRequestExtensionMessage::toString() const
{
  return fmt("ut_metadata request piece=%lu",
             static_cast<unsigned long>(getIndex()));
}

void UTMetadataRequestExtensionMessage::doReceivedAction()
{
  SharedHandle<TorrentAttribute> attrs = bittorrent::getTorrentAttrs(dctx_);
  uint8_t id = peer_->getExtensionMessageID("ut_metadata");
  if(attrs->metadata.empty()) {
    SharedHandle<UTMetadataRejectExtensionMessage> m
      (new UTMetadataRejectExtensionMessage(id));
    m->setIndex(getIndex());
    SharedHandle<BtMessage> msg = messageFactory_->createBtExtendedMessage(m);
    dispatcher_->addMessageToQueue(msg);
  }else if(getIndex()*METADATA_PIECE_SIZE < attrs->metadataSize) {
    SharedHandle<UTMetadataDataExtensionMessage> m
      (new UTMetadataDataExtensionMessage(id));
    m->setIndex(getIndex());
    m->setTotalSize(attrs->metadataSize);
    std::string::const_iterator begin =
      attrs->metadata.begin()+getIndex()*METADATA_PIECE_SIZE;
    std::string::const_iterator end =
      (getIndex()+1)*METADATA_PIECE_SIZE <= attrs->metadata.size()?
      attrs->metadata.begin()+(getIndex()+1)*METADATA_PIECE_SIZE:
      attrs->metadata.end();
    m->setData(begin, end);
    SharedHandle<BtMessage> msg = messageFactory_->createBtExtendedMessage(m);
    dispatcher_->addMessageToQueue(msg);
  } else {
    throw DL_ABORT_EX
      (fmt("Metadata piece index is too big. piece=%lu",
           static_cast<unsigned long>(getIndex())));
  }
}

void UTMetadataRequestExtensionMessage::setDownloadContext
(const SharedHandle<DownloadContext>& dctx)
{
  dctx_ = dctx;
}

void UTMetadataRequestExtensionMessage::setPeer(const SharedHandle<Peer>& peer)
{
  peer_ = peer;
}

} // namespace aria2
