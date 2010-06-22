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
#include "UTMetadataRequestFactory.h"
#include "PieceStorage.h"
#include "DownloadContext.h"
#include "Peer.h"
#include "BtMessageDispatcher.h"
#include "BtMessageFactory.h"
#include "UTMetadataRequestExtensionMessage.h"
#include "UTMetadataRequestTracker.h"
#include "BtMessage.h"
#include "LogFactory.h"

namespace aria2 {

UTMetadataRequestFactory::UTMetadataRequestFactory():
  logger_(LogFactory::getInstance()) {}

void UTMetadataRequestFactory::create
(std::vector<SharedHandle<BtMessage> >& msgs, size_t num,
 const SharedHandle<PieceStorage>& pieceStorage)
{
  for(size_t index = 0, numPieces = dctx_->getNumPieces();
      index < numPieces && num; ++index) {
    SharedHandle<Piece> p = pieceStorage->getMissingPiece(index);
    if(p.isNull()) {
      if(logger_->debug()) {
        logger_->debug
          ("ut_metadata piece %lu is used or already acquired.",
           static_cast<unsigned long>(index));
      }
      continue;
    }
    --num;
    if(logger_->debug()) {
      logger_->debug("Creating ut_metadata request index=%lu",
                     static_cast<unsigned long>(index));
    }
    SharedHandle<UTMetadataRequestExtensionMessage> m
      (new UTMetadataRequestExtensionMessage
       (peer_->getExtensionMessageID("ut_metadata")));
    m->setIndex(index);
    m->setDownloadContext(dctx_);
    m->setBtMessageDispatcher(dispatcher_);
    m->setBtMessageFactory(messageFactory_);
    m->setPeer(peer_);
    
    SharedHandle<BtMessage> msg = messageFactory_->createBtExtendedMessage(m);
    msgs.push_back(msg);
    tracker_->add(index);
  }
}

} // namespace aria2
