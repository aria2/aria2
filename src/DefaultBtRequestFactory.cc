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
#include "DefaultBtRequestFactory.h"
#include "LogFactory.h"
#include "Logger.h"
#include "Piece.h"
#include "Peer.h"
#include "BtContext.h"
#include "PieceStorage.h"
#include "BtMessageDispatcher.h"
#include "BtMessageFactory.h"
#include "BtMessage.h"
#include "BtRegistry.h"
#include <algorithm>

namespace aria2 {

DefaultBtRequestFactory::DefaultBtRequestFactory():
  cuid(0),
  btContext(0),
  pieceStorage(0),
  peer(0),
  dispatcher(0)
{
  LogFactory::getInstance()->debug("DefaultBtRequestFactory::instantiated");
}

DefaultBtRequestFactory::~DefaultBtRequestFactory()
{
  LogFactory::getInstance()->debug("DefaultBtRequestFactory::deleted");
}

void DefaultBtRequestFactory::addTargetPiece(const PieceHandle& piece)
{
  pieces.push_back(piece);
}

void DefaultBtRequestFactory::removeCompletedPiece() {
  for(Pieces::iterator itr = pieces.begin(); itr != pieces.end();) {
    PieceHandle& piece = *itr;
    if(piece->pieceComplete()) {
      dispatcher->doAbortOutstandingRequestAction(piece);
      itr = pieces.erase(itr);
    } else {
      itr++;
    }
  }
}

void DefaultBtRequestFactory::removeTargetPiece(const PieceHandle& piece) {
  Pieces temp;
  std::remove_copy(pieces.begin(), pieces.end(), std::back_inserter(temp), piece);
  pieces = temp;
  dispatcher->doAbortOutstandingRequestAction(piece);
  pieceStorage->cancelPiece(piece);
}

void DefaultBtRequestFactory::doChokedAction()
{
  Pieces temp;
  for(Pieces::iterator itr = pieces.begin(); itr != pieces.end(); itr++) {
    PieceHandle& piece = *itr;
    if(peer->isInPeerAllowedIndexSet(piece->getIndex())) {
      temp.push_back(piece);
    } else {
      pieceStorage->cancelPiece(*itr);
    }
  }
  pieces = temp;
}

void DefaultBtRequestFactory::removeAllTargetPiece() {
  for(Pieces::iterator itr = pieces.begin(); itr != pieces.end(); itr++) {
    dispatcher->doAbortOutstandingRequestAction(*itr);
    pieceStorage->cancelPiece(*itr);
  }
  pieces.clear();
}

BtMessages DefaultBtRequestFactory::createRequestMessages(int32_t max) {
  BtMessages requests;
  for(Pieces::iterator itr = pieces.begin();
      itr != pieces.end() && requests.size() < (size_t)max; itr++) {
    PieceHandle& piece = *itr;
    int32_t blockIndex;
    while(requests.size() < (size_t)max &&
	  (blockIndex = piece->getMissingUnusedBlockIndex()) != -1) {
      requests.push_back(messageFactory->createRequestMessage(piece, blockIndex));
    }
  }
  return requests;
}

BtMessages DefaultBtRequestFactory::createRequestMessagesOnEndGame(int32_t max) {
  BtMessages requests;
  for(Pieces::iterator itr = pieces.begin();
      itr != pieces.end() && requests.size() < (size_t)max; itr++) {
    PieceHandle& piece = *itr;
    std::deque<int32_t> missingBlockIndexes = piece->getAllMissingBlockIndexes();
    random_shuffle(missingBlockIndexes.begin(), missingBlockIndexes.end());
    for(std::deque<int32_t>::const_iterator bitr = missingBlockIndexes.begin();
	bitr != missingBlockIndexes.end() && requests.size() < (size_t)max;
	bitr++) {
      int32_t blockIndex = *bitr;
      if(!dispatcher->isOutstandingRequest(piece->getIndex(),
					   blockIndex)) {
	requests.push_back(messageFactory->createRequestMessage(piece, blockIndex));
      }
    }
  }
  return requests;
}

std::deque<SharedHandle<Piece> >& DefaultBtRequestFactory::getTargetPieces()
{
  return pieces;
}

void DefaultBtRequestFactory::setBtContext(const SharedHandle<BtContext>& btContext)
{
  this->btContext = btContext;
  this->pieceStorage = PIECE_STORAGE(btContext);
}

void DefaultBtRequestFactory::setPeer(const SharedHandle<Peer>& peer)
{
  this->peer = peer;
}

void DefaultBtRequestFactory::setBtMessageDispatcher(const WeakHandle<BtMessageDispatcher>& dispatcher)
{
  this->dispatcher = dispatcher;
}

void DefaultBtRequestFactory::setBtMessageFactory(const WeakHandle<BtMessageFactory>& factory)
{
  this->messageFactory = factory;
}

} // namespace aria2
