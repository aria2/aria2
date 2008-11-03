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

#include <algorithm>

#include "LogFactory.h"
#include "Logger.h"
#include "Piece.h"
#include "Peer.h"
#include "BtContext.h"
#include "PieceStorage.h"
#include "BtMessageDispatcher.h"
#include "BtMessageFactory.h"
#include "BtMessage.h"
#include "a2functional.h"

namespace aria2 {

DefaultBtRequestFactory::DefaultBtRequestFactory():
  cuid(0),
  _logger(LogFactory::getInstance())
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

class AbortCompletedPieceRequest
{
private:
  WeakHandle<BtMessageDispatcher> _dispatcher;
public:
  AbortCompletedPieceRequest(const WeakHandle<BtMessageDispatcher>& dispatcher):
    _dispatcher(dispatcher) {}

  void operator()(const SharedHandle<Piece>& piece)
  {
    if(piece->pieceComplete()) {
      _dispatcher->doAbortOutstandingRequestAction(piece);
    }
  }
};

void DefaultBtRequestFactory::removeCompletedPiece() {
  std::for_each(pieces.begin(), pieces.end(),
		AbortCompletedPieceRequest(dispatcher));
  pieces.erase(std::remove_if(pieces.begin(), pieces.end(),
			      mem_fun_sh(&Piece::pieceComplete)),
	       pieces.end());
}

void DefaultBtRequestFactory::removeTargetPiece(const PieceHandle& piece) {
  pieces.erase(std::remove(pieces.begin(), pieces.end(), piece),
	       pieces.end());
  dispatcher->doAbortOutstandingRequestAction(piece);
  _pieceStorage->cancelPiece(piece);
}

class ProcessChokedPiece {
private:
  SharedHandle<Peer> _peer;
  WeakHandle<PieceStorage> _pieceStorage;
public:
  ProcessChokedPiece(const SharedHandle<Peer>& peer,
		     const WeakHandle<PieceStorage>& pieceStorage):
    _peer(peer),
    _pieceStorage(pieceStorage) {}

  void operator()(const SharedHandle<Piece>& piece)
  {
    if(!_peer->isInPeerAllowedIndexSet(piece->getIndex())) {
      _pieceStorage->cancelPiece(piece);
    }
  }
};

class FindChokedPiece {
private:
  SharedHandle<Peer> _peer;
public:
  FindChokedPiece(const SharedHandle<Peer>& peer):_peer(peer) {}

  bool operator()(const SharedHandle<Piece>& piece)
  {
    return !_peer->isInPeerAllowedIndexSet(piece->getIndex());
  }
};

void DefaultBtRequestFactory::doChokedAction()
{
  std::for_each(pieces.begin(), pieces.end(),
		ProcessChokedPiece(peer, _pieceStorage));
  pieces.erase(std::remove_if(pieces.begin(), pieces.end(),
			      FindChokedPiece(peer)),
	       pieces.end());
}

void DefaultBtRequestFactory::removeAllTargetPiece() {
  for(Pieces::iterator itr = pieces.begin(); itr != pieces.end(); ++itr) {
    dispatcher->doAbortOutstandingRequestAction(*itr);
    _pieceStorage->cancelPiece(*itr);
  }
  pieces.clear();
}

void DefaultBtRequestFactory::createRequestMessages
(std::deque<SharedHandle<BtMessage> >& requests, size_t max)
{
  for(Pieces::iterator itr = pieces.begin();
      itr != pieces.end() && requests.size() < max; ++itr) {
    PieceHandle& piece = *itr;
    size_t blockIndex;
    while(requests.size() < max &&
	  piece->getMissingUnusedBlockIndex(blockIndex)) {
      _logger->debug("Creating RequestMessage index=%u, begin=%u, blockIndex=%u",
		    piece->getIndex(),
		    blockIndex*piece->getBlockLength(),
		    blockIndex);

      requests.push_back(messageFactory->createRequestMessage(piece, blockIndex));
    }
  }
}

void DefaultBtRequestFactory::createRequestMessagesOnEndGame
(std::deque<SharedHandle<BtMessage> >& requests, size_t max)
{
  for(Pieces::iterator itr = pieces.begin();
      itr != pieces.end() && requests.size() < max; ++itr) {
    PieceHandle& piece = *itr;
    std::deque<size_t> missingBlockIndexes;
    piece->getAllMissingBlockIndexes(missingBlockIndexes);
    std::random_shuffle(missingBlockIndexes.begin(), missingBlockIndexes.end());
    for(std::deque<size_t>::const_iterator bitr = missingBlockIndexes.begin();
	bitr != missingBlockIndexes.end() && requests.size() < max; bitr++) {
      size_t blockIndex = *bitr;
      if(!dispatcher->isOutstandingRequest(piece->getIndex(),
					   blockIndex)) {
      _logger->debug("Creating RequestMessage index=%u, begin=%u, blockIndex=%u",
		    piece->getIndex(),
		    blockIndex*piece->getBlockLength(),
		    blockIndex);
	requests.push_back(messageFactory->createRequestMessage(piece, blockIndex));
      }
    }
  }
}

class CountMissingBlock
{
private:
  size_t _numMissingBlock;
public:
  CountMissingBlock():_numMissingBlock(0) {}

  size_t getNumMissingBlock()
  {
    return _numMissingBlock;
  }

  void operator()(const SharedHandle<Piece>& piece)
  {
    _numMissingBlock += piece->countMissingBlock();
  }
};

size_t DefaultBtRequestFactory::countMissingBlock()
{
  return std::for_each(pieces.begin(), pieces.end(),
		       CountMissingBlock()).getNumMissingBlock();
}

void DefaultBtRequestFactory::getTargetPieceIndexes
(std::deque<size_t>& indexes) const
{
  std::transform(pieces.begin(), pieces.end(), std::back_inserter(indexes),
		 mem_fun_sh(&Piece::getIndex));
}

std::deque<SharedHandle<Piece> >& DefaultBtRequestFactory::getTargetPieces()
{
  return pieces;
}

void DefaultBtRequestFactory::setBtContext(const SharedHandle<BtContext>& btContext)
{
  this->btContext = btContext;
}

void DefaultBtRequestFactory::setPieceStorage
(const SharedHandle<PieceStorage>& pieceStorage)
{
  _pieceStorage = pieceStorage;
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
