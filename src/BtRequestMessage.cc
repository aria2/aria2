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
#include "BtRequestMessage.h"
#include "Peer.h"
#include "Piece.h"
#include "PieceStorage.h"
#include "BtMessageDispatcher.h"
#include "BtMessageFactory.h"

namespace aria2 {

const std::string BtRequestMessage::NAME("request");

SharedHandle<BtRequestMessage> BtRequestMessage::create
(const unsigned char* data, size_t dataLength)
{
  return RangeBtMessage::create<BtRequestMessage>(data, dataLength);
}

void BtRequestMessage::doReceivedAction()
{
  if(pieceStorage->hasPiece(getIndex()) &&
     (!peer->amChoking() ||
      (peer->amChoking() && peer->isInAmAllowedIndexSet(getIndex())))) {
    BtMessageHandle msg = messageFactory->createPieceMessage(getIndex(),
							     getBegin(),
							     getLength());
    dispatcher->addMessageToQueue(msg);
  } else {
    if(peer->isFastExtensionEnabled()) {
      BtMessageHandle msg = messageFactory->createRejectMessage(getIndex(),
								getBegin(),
								getLength());
      dispatcher->addMessageToQueue(msg);
    }
  }
}

void BtRequestMessage::onQueued()
{
  RequestSlot requestSlot(getIndex(), getBegin(), getLength(), _blockIndex,
			  pieceStorage->getPiece(getIndex()));
  dispatcher->addOutstandingRequest(requestSlot);
}

void BtRequestMessage::onAbortOutstandingRequestEvent
(const BtAbortOutstandingRequestEvent& event)
{
  if(getIndex() == event.getPiece()->getIndex() &&
     !invalidate && !sendingInProgress) {
    invalidate = true;
  }
}

} // namespace aria2
