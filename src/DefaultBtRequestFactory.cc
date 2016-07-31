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
#include "fmt.h"
#include "BtRequestMessage.h"

namespace aria2 {

DefaultBtRequestFactory::DefaultBtRequestFactory()
    : pieceStorage_(nullptr),
      dispatcher_(nullptr),
      messageFactory_(nullptr),
      cuid_(0)
{
}

DefaultBtRequestFactory::~DefaultBtRequestFactory() = default;

void DefaultBtRequestFactory::addTargetPiece(
    const std::shared_ptr<Piece>& piece)
{
  pieces_.push_back(piece);
}

namespace {
class AbortCompletedPieceRequest {
private:
  BtMessageDispatcher* dispatcher_;

public:
  AbortCompletedPieceRequest(BtMessageDispatcher* dispatcher)
      : dispatcher_(dispatcher)
  {
  }

  void operator()(const std::shared_ptr<Piece>& piece)
  {
    if (piece->pieceComplete()) {
      dispatcher_->doAbortOutstandingRequestAction(piece);
    }
  }
};
} // namespace

void DefaultBtRequestFactory::removeCompletedPiece()
{
  std::for_each(pieces_.begin(), pieces_.end(),
                AbortCompletedPieceRequest(dispatcher_));
  pieces_.erase(std::remove_if(pieces_.begin(), pieces_.end(),
                               std::mem_fn(&Piece::pieceComplete)),
                pieces_.end());
}

void DefaultBtRequestFactory::removeTargetPiece(
    const std::shared_ptr<Piece>& piece)
{
  pieces_.erase(
      std::remove_if(pieces_.begin(), pieces_.end(), derefEqual(piece)),
      pieces_.end());
  dispatcher_->doAbortOutstandingRequestAction(piece);
  pieceStorage_->cancelPiece(piece, cuid_);
}

namespace {
class ProcessChokedPiece {
private:
  std::shared_ptr<Peer> peer_;
  PieceStorage* pieceStorage_;
  cuid_t cuid_;

public:
  ProcessChokedPiece(std::shared_ptr<Peer> peer, PieceStorage* pieceStorage,
                     cuid_t cuid)
      : peer_(std::move(peer)), pieceStorage_(pieceStorage), cuid_(cuid)
  {
  }

  void operator()(const std::shared_ptr<Piece>& piece)
  {
    if (!peer_->isInPeerAllowedIndexSet(piece->getIndex())) {
      pieceStorage_->cancelPiece(piece, cuid_);
    }
  }
};
} // namespace

namespace {
class FindChokedPiece {
private:
  std::shared_ptr<Peer> peer_;

public:
  FindChokedPiece(std::shared_ptr<Peer> peer) : peer_(std::move(peer)) {}

  bool operator()(const std::shared_ptr<Piece>& piece)
  {
    return !peer_->isInPeerAllowedIndexSet(piece->getIndex());
  }
};
} // namespace

void DefaultBtRequestFactory::doChokedAction()
{
  std::for_each(pieces_.begin(), pieces_.end(),
                ProcessChokedPiece(peer_, pieceStorage_, cuid_));
  pieces_.erase(
      std::remove_if(pieces_.begin(), pieces_.end(), FindChokedPiece(peer_)),
      pieces_.end());
}

void DefaultBtRequestFactory::removeAllTargetPiece()
{
  for (auto& elem : pieces_) {
    dispatcher_->doAbortOutstandingRequestAction(elem);
    pieceStorage_->cancelPiece(elem, cuid_);
  }
  pieces_.clear();
}

std::vector<std::unique_ptr<BtRequestMessage>>
DefaultBtRequestFactory::createRequestMessages(size_t max, bool endGame)
{
  if (endGame) {
    return createRequestMessagesOnEndGame(max);
  }
  auto requests = std::vector<std::unique_ptr<BtRequestMessage>>{};
  size_t getnum = max - requests.size();
  auto blockIndexes = std::vector<size_t>{};
  blockIndexes.reserve(getnum);
  for (auto itr = std::begin(pieces_), eoi = std::end(pieces_);
       itr != eoi && getnum; ++itr) {
    auto& piece = *itr;
    if (piece->getMissingUnusedBlockIndex(blockIndexes, getnum)) {
      getnum -= blockIndexes.size();
      for (auto i = std::begin(blockIndexes), eoi2 = std::end(blockIndexes);
           i != eoi2; ++i) {
        A2_LOG_DEBUG(
            fmt("Creating RequestMessage index=%lu, begin=%u,"
                " blockIndex=%lu",
                static_cast<unsigned long>(piece->getIndex()),
                static_cast<unsigned int>((*i) * piece->getBlockLength()),
                static_cast<unsigned long>(*i)));
        requests.push_back(messageFactory_->createRequestMessage(piece, *i));
      }
      blockIndexes.clear();
    }
  }
  return requests;
}

std::vector<std::unique_ptr<BtRequestMessage>>
DefaultBtRequestFactory::createRequestMessagesOnEndGame(size_t max)
{
  auto requests = std::vector<std::unique_ptr<BtRequestMessage>>{};
  for (auto itr = std::begin(pieces_), eoi = std::end(pieces_);
       itr != eoi && requests.size() < max; ++itr) {
    auto& piece = *itr;
    const size_t mislen = piece->getBitfieldLength();
    auto misbitfield = make_unique<unsigned char[]>(mislen);

    piece->getAllMissingBlockIndexes(misbitfield.get(), mislen);

    auto missingBlockIndexes = std::vector<size_t>{};
    size_t blockIndex = 0;
    for (size_t i = 0; i < mislen; ++i) {
      unsigned char bits = misbitfield[i];
      unsigned char mask = 128;
      for (size_t bi = 0; bi < 8; ++bi, mask >>= 1, ++blockIndex) {
        if (bits & mask) {
          missingBlockIndexes.push_back(blockIndex);
        }
      }
    }
    std::shuffle(std::begin(missingBlockIndexes), std::end(missingBlockIndexes),
                 *SimpleRandomizer::getInstance());
    for (auto bitr = std::begin(missingBlockIndexes),
              eoi2 = std::end(missingBlockIndexes);
         bitr != eoi2 && requests.size() < max; ++bitr) {
      size_t blockIndex = *bitr;
      if (!dispatcher_->isOutstandingRequest(piece->getIndex(), blockIndex)) {
        A2_LOG_DEBUG(
            fmt("Creating RequestMessage index=%lu, begin=%u,"
                " blockIndex=%lu",
                static_cast<unsigned long>(piece->getIndex()),
                static_cast<unsigned int>(blockIndex * piece->getBlockLength()),
                static_cast<unsigned long>(blockIndex)));
        requests.push_back(
            messageFactory_->createRequestMessage(piece, blockIndex));
      }
    }
  }
  return requests;
}

namespace {
class CountMissingBlock {
private:
  size_t numMissingBlock_;

public:
  CountMissingBlock() : numMissingBlock_(0) {}

  size_t getNumMissingBlock() { return numMissingBlock_; }

  void operator()(const std::shared_ptr<Piece>& piece)
  {
    numMissingBlock_ += piece->countMissingBlock();
  }
};
} // namespace

size_t DefaultBtRequestFactory::countMissingBlock()
{
  return std::for_each(pieces_.begin(), pieces_.end(), CountMissingBlock())
      .getNumMissingBlock();
}

std::vector<size_t> DefaultBtRequestFactory::getTargetPieceIndexes() const
{
  auto res = std::vector<size_t>{};
  res.reserve(pieces_.size());
  std::transform(std::begin(pieces_), std::end(pieces_),
                 std::back_inserter(res), std::mem_fn(&Piece::getIndex));
  return res;
}

void DefaultBtRequestFactory::setPieceStorage(PieceStorage* pieceStorage)
{
  pieceStorage_ = pieceStorage;
}

void DefaultBtRequestFactory::setPeer(const std::shared_ptr<Peer>& peer)
{
  peer_ = peer;
}

void DefaultBtRequestFactory::setBtMessageDispatcher(
    BtMessageDispatcher* dispatcher)
{
  dispatcher_ = dispatcher;
}

void DefaultBtRequestFactory::setBtMessageFactory(BtMessageFactory* factory)
{
  messageFactory_ = factory;
}

} // namespace aria2
