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
#include "DefaultBtRequestFactory.h"

#include <algorithm>

#include "LogFactory.h"
#include "Logger.h"
#include "Piece.h"
#include "Peer.h"
#include "PieceStorage.h"
#include "BtMessageDispatcher.h"
#include "BtMessageFactory.h"
#include "BtMessage.h"
#include "a2functional.h"
#include "SimpleRandomizer.h"
#include "array_fun.h"

namespace aria2 {

DefaultBtRequestFactory::DefaultBtRequestFactory():
  logger_(LogFactory::getInstance())
{
  if(logger_->debug()) {
    logger_->debug("DefaultBtRequestFactory::instantiated");
  }
}

DefaultBtRequestFactory::~DefaultBtRequestFactory()
{
  if(logger_->debug()) {
    logger_->debug("DefaultBtRequestFactory::deleted");
  }
}

void DefaultBtRequestFactory::addTargetPiece(const SharedHandle<Piece>& piece)
{
  pieces_.push_back(piece);
}

namespace {
class AbortCompletedPieceRequest
{
private:
  WeakHandle<BtMessageDispatcher> dispatcher_;
public:
  AbortCompletedPieceRequest(const WeakHandle<BtMessageDispatcher>& dispatcher):
    dispatcher_(dispatcher) {}

  void operator()(const SharedHandle<Piece>& piece)
  {
    if(piece->pieceComplete()) {
      dispatcher_->doAbortOutstandingRequestAction(piece);
    }
  }
};
}

void DefaultBtRequestFactory::removeCompletedPiece() {
  std::for_each(pieces_.begin(), pieces_.end(),
                AbortCompletedPieceRequest(dispatcher_));
  pieces_.erase(std::remove_if(pieces_.begin(), pieces_.end(),
                              mem_fun_sh(&Piece::pieceComplete)),
               pieces_.end());
}

void DefaultBtRequestFactory::removeTargetPiece
(const SharedHandle<Piece>& piece)
{
  pieces_.erase(std::remove(pieces_.begin(), pieces_.end(), piece),
               pieces_.end());
  dispatcher_->doAbortOutstandingRequestAction(piece);
  pieceStorage_->cancelPiece(piece);
}

namespace {
class ProcessChokedPiece {
private:
  SharedHandle<Peer> peer_;
  WeakHandle<PieceStorage> pieceStorage_;
public:
  ProcessChokedPiece(const SharedHandle<Peer>& peer,
                     const WeakHandle<PieceStorage>& pieceStorage):
    peer_(peer),
    pieceStorage_(pieceStorage) {}

  void operator()(const SharedHandle<Piece>& piece)
  {
    if(!peer_->isInPeerAllowedIndexSet(piece->getIndex())) {
      pieceStorage_->cancelPiece(piece);
    }
  }
};
}

namespace {
class FindChokedPiece {
private:
  SharedHandle<Peer> peer_;
public:
  FindChokedPiece(const SharedHandle<Peer>& peer):peer_(peer) {}

  bool operator()(const SharedHandle<Piece>& piece)
  {
    return !peer_->isInPeerAllowedIndexSet(piece->getIndex());
  }
};
}

void DefaultBtRequestFactory::doChokedAction()
{
  std::for_each(pieces_.begin(), pieces_.end(),
                ProcessChokedPiece(peer_, pieceStorage_));
  pieces_.erase(std::remove_if(pieces_.begin(), pieces_.end(),
                              FindChokedPiece(peer_)),
               pieces_.end());
}

void DefaultBtRequestFactory::removeAllTargetPiece() {
  for(std::deque<SharedHandle<Piece> >::iterator itr = pieces_.begin(),
        eoi = pieces_.end(); itr != eoi; ++itr) {
    dispatcher_->doAbortOutstandingRequestAction(*itr);
    pieceStorage_->cancelPiece(*itr);
  }
  pieces_.clear();
}

void DefaultBtRequestFactory::createRequestMessages
(std::vector<SharedHandle<BtMessage> >& requests, size_t max)
{
  if(requests.size() >= max) {
    return;
  }
  size_t getnum = max-requests.size();
  std::vector<size_t> blockIndexes;
  blockIndexes.reserve(getnum);
  for(std::deque<SharedHandle<Piece> >::iterator itr = pieces_.begin(),
        eoi = pieces_.end(); itr != eoi && getnum; ++itr) {
    SharedHandle<Piece>& piece = *itr;
    if(piece->getMissingUnusedBlockIndex(blockIndexes, getnum)) {
      getnum -= blockIndexes.size();
      for(std::vector<size_t>::const_iterator i = blockIndexes.begin(),
            eoi2 = blockIndexes.end(); i != eoi2; ++i) {
        if(logger_->debug()) {
          logger_->debug("Creating RequestMessage index=%u, begin=%u,"
                         " blockIndex=%u",
                         piece->getIndex(),
                         (*i)*piece->getBlockLength(),
                         (*i));
        }
        requests.push_back
          (messageFactory_->createRequestMessage(piece, *i));
      }
      blockIndexes.clear();
    }
  }
}

void DefaultBtRequestFactory::createRequestMessagesOnEndGame
(std::vector<SharedHandle<BtMessage> >& requests, size_t max)
{
  for(std::deque<SharedHandle<Piece> >::iterator itr = pieces_.begin(),
        eoi = pieces_.end(); itr != eoi && requests.size() < max; ++itr) {
    SharedHandle<Piece>& piece = *itr;
    const size_t mislen = piece->getBitfieldLength();
    array_ptr<unsigned char> misbitfield(new unsigned char[mislen]);

    piece->getAllMissingBlockIndexes(misbitfield, mislen);

    std::vector<size_t> missingBlockIndexes;
    size_t blockIndex = 0;
    for(size_t i = 0; i < mislen; ++i) {
      unsigned char bits = misbitfield[i];
      unsigned char mask = 128;
      for(size_t bi = 0; bi < 8; ++bi, mask >>= 1, ++blockIndex) {
        if(bits & mask) {
          missingBlockIndexes.push_back(blockIndex);
        }
      }
    }
    std::random_shuffle(missingBlockIndexes.begin(), missingBlockIndexes.end(),
                        *(SimpleRandomizer::getInstance().get()));
    for(std::vector<size_t>::const_iterator bitr = missingBlockIndexes.begin(),
          eoi2 = missingBlockIndexes.end();
        bitr != eoi2 && requests.size() < max; ++bitr) {
      const size_t& blockIndex = *bitr;
      if(!dispatcher_->isOutstandingRequest(piece->getIndex(),
                                           blockIndex)) {
        if(logger_->debug()) {
          logger_->debug("Creating RequestMessage index=%u, begin=%u,"
                         " blockIndex=%u",
                         piece->getIndex(),
                         blockIndex*piece->getBlockLength(),
                         blockIndex);
        }
        requests.push_back(messageFactory_->createRequestMessage
                           (piece, blockIndex));
      }
    }
  }
}

namespace {
class CountMissingBlock
{
private:
  size_t numMissingBlock_;
public:
  CountMissingBlock():numMissingBlock_(0) {}

  size_t getNumMissingBlock()
  {
    return numMissingBlock_;
  }

  void operator()(const SharedHandle<Piece>& piece)
  {
    numMissingBlock_ += piece->countMissingBlock();
  }
};
}

size_t DefaultBtRequestFactory::countMissingBlock()
{
  return std::for_each(pieces_.begin(), pieces_.end(),
                       CountMissingBlock()).getNumMissingBlock();
}

void DefaultBtRequestFactory::getTargetPieceIndexes
(std::vector<size_t>& indexes) const
{
  std::transform(pieces_.begin(), pieces_.end(), std::back_inserter(indexes),
                 mem_fun_sh(&Piece::getIndex));
}

void DefaultBtRequestFactory::setPieceStorage
(const SharedHandle<PieceStorage>& pieceStorage)
{
  pieceStorage_ = pieceStorage;
}

void DefaultBtRequestFactory::setPeer(const SharedHandle<Peer>& peer)
{
  peer_ = peer;
}

void DefaultBtRequestFactory::setBtMessageDispatcher
(const WeakHandle<BtMessageDispatcher>& dispatcher)
{
  dispatcher_ = dispatcher;
}

void DefaultBtRequestFactory::setBtMessageFactory
(const WeakHandle<BtMessageFactory>& factory)
{
  messageFactory_ = factory;
}

} // namespace aria2
