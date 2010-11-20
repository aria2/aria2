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
#include "DefaultBtMessageDispatcher.h"

#include <algorithm>

#include "prefs.h"
#include "BtAbortOutstandingRequestEvent.h"
#include "BtCancelSendingPieceEvent.h"
#include "BtChokingEvent.h"
#include "BtMessageFactory.h"
#include "message.h"
#include "DownloadContext.h"
#include "PeerStorage.h"
#include "PieceStorage.h"
#include "BtMessage.h"
#include "Peer.h"
#include "Piece.h"
#include "LogFactory.h"
#include "Logger.h"
#include "a2functional.h"
#include "a2algo.h"
#include "RequestGroupMan.h"
#include "RequestGroup.h"
#include "util.h"
#include "fmt.h"

namespace aria2 {

DefaultBtMessageDispatcher::DefaultBtMessageDispatcher()
  : cuid(0),
    messageFactory_(0),
    requestGroupMan_(0),
    requestTimeout_(0)
{}

DefaultBtMessageDispatcher::~DefaultBtMessageDispatcher()
{
  A2_LOG_DEBUG("DefaultBtMessageDispatcher::deleted");
}

void DefaultBtMessageDispatcher::addMessageToQueue
(const BtMessageHandle& btMessage)
{
  btMessage->onQueued();
  messageQueue_.push_back(btMessage);
}

void DefaultBtMessageDispatcher::addMessageToQueue
(const std::vector<SharedHandle<BtMessage> >& btMessages)
{
  for(std::vector<SharedHandle<BtMessage> >::const_iterator itr =
        btMessages.begin(), eoi = btMessages.end(); itr != eoi; ++itr) {
    addMessageToQueue(*itr);
  }
}

void DefaultBtMessageDispatcher::sendMessages() {
  std::vector<SharedHandle<BtMessage> > tempQueue;
  while(!messageQueue_.empty()) {
    BtMessageHandle msg = messageQueue_.front();
    messageQueue_.pop_front();
    if(msg->isUploading() && !msg->isSendingInProgress()) {
      if(requestGroupMan_->doesOverallUploadSpeedExceed() ||
         downloadContext_->getOwnerRequestGroup()->doesUploadSpeedExceed()) {
        tempQueue.push_back(msg);
        continue;
      }
    }
    msg->send();
    if(msg->isUploading()) {
      peerStorage_->updateTransferStatFor(peer_);
    }
    if(msg->isSendingInProgress()) {
      messageQueue_.push_front(msg);
      break;
    }
  }
  if(!tempQueue.empty()) {
    // Insert pending message to the front, so that message is likely sent in
    // the same order as it is queued.
    if(!messageQueue_.empty() && messageQueue_.front()->isSendingInProgress()) {
      messageQueue_.insert(messageQueue_.begin()+1,
                           tempQueue.begin(), tempQueue.end());
    } else {
      messageQueue_.insert(messageQueue_.begin(),
                           tempQueue.begin(), tempQueue.end());
    }
  }
}

// Cancel sending piece message to peer.
void DefaultBtMessageDispatcher::doCancelSendingPieceAction
(size_t index, uint32_t begin, size_t length)
{
  BtCancelSendingPieceEvent event(index, begin, length);

  std::vector<SharedHandle<BtMessage> > tempQueue
    (messageQueue_.begin(), messageQueue_.end());

  forEachMemFunSH(tempQueue.begin(), tempQueue.end(),
                  &BtMessage::onCancelSendingPieceEvent, event);
}

// Cancel sending piece message to peer.
// TODO Is this method really necessary?
void DefaultBtMessageDispatcher::doCancelSendingPieceAction
(const SharedHandle<Piece>& piece)
{
}

namespace {
class AbortOutstandingRequest {
private:
  SharedHandle<Piece> piece_;
  cuid_t cuid_;
public:
  AbortOutstandingRequest(const SharedHandle<Piece>& piece, cuid_t cuid)
    : piece_(piece),
      cuid_(cuid)
  {}

  void operator()(const RequestSlot& slot) const
  {
    A2_LOG_DEBUG(fmt(MSG_DELETING_REQUEST_SLOT,
                     util::itos(cuid_).c_str(),
                     static_cast<unsigned long>(slot.getIndex()),
                     slot.getBegin(),
                     static_cast<unsigned long>(slot.getBlockIndex())));
    piece_->cancelBlock(slot.getBlockIndex());
  }
};
} // namespace

// localhost cancels outstanding download requests to the peer.
void DefaultBtMessageDispatcher::doAbortOutstandingRequestAction
(const SharedHandle<Piece>& piece) {
  RequestSlot rs(piece->getIndex(), 0, 0, 0);
  std::deque<RequestSlot>::iterator first =
    std::lower_bound(requestSlots_.begin(), requestSlots_.end(), rs);

  rs.setIndex(piece->getIndex()+1);
  std::deque<RequestSlot>::iterator last =
    std::lower_bound(requestSlots_.begin(), requestSlots_.end(), rs);

  std::for_each(first, last, AbortOutstandingRequest(piece, cuid));
  requestSlots_.erase(first, last);

  BtAbortOutstandingRequestEvent event(piece);

  std::vector<SharedHandle<BtMessage> > tempQueue
    (messageQueue_.begin(), messageQueue_.end());
  forEachMemFunSH(tempQueue.begin(), tempQueue.end(),
                  &BtMessage::onAbortOutstandingRequestEvent, event);
}

namespace {
class ProcessChokedRequestSlot {
private:
  cuid_t cuid_;
  SharedHandle<Peer> peer_;
  SharedHandle<PieceStorage> pieceStorage_;
public:
  ProcessChokedRequestSlot
  (cuid_t cuid,
   const SharedHandle<Peer>& peer,
   const SharedHandle<PieceStorage>& pieceStorage)
    : cuid_(cuid),
      peer_(peer),
      pieceStorage_(pieceStorage)
  {}
  
  void operator()(const RequestSlot& slot) const
  {
    if(!peer_->isInPeerAllowedIndexSet(slot.getIndex())) {
      A2_LOG_DEBUG(fmt(MSG_DELETING_REQUEST_SLOT_CHOKED,
                       util::itos(cuid_).c_str(),
                       static_cast<unsigned long>(slot.getIndex()),
                       slot.getBegin(),
                       static_cast<unsigned long>(slot.getBlockIndex())));
      SharedHandle<Piece> piece = pieceStorage_->getPiece(slot.getIndex());
      piece->cancelBlock(slot.getBlockIndex());
    }
  }

};
} // namespace

namespace {
class FindChokedRequestSlot {
private:
  SharedHandle<Peer> peer_;
public:
  FindChokedRequestSlot(const SharedHandle<Peer>& peer):
    peer_(peer) {}
  
  bool operator()(const RequestSlot& slot) const
  {
    return !peer_->isInPeerAllowedIndexSet(slot.getIndex());
  }
};
} // namespace

// localhost received choke message from the peer.
void DefaultBtMessageDispatcher::doChokedAction()
{
  std::for_each(requestSlots_.begin(), requestSlots_.end(),
                ProcessChokedRequestSlot(cuid, peer_, pieceStorage_));

  requestSlots_.erase(std::remove_if(requestSlots_.begin(), requestSlots_.end(),
                                     FindChokedRequestSlot(peer_)),
                      requestSlots_.end());
}

// localhost dispatched choke message to the peer.
void DefaultBtMessageDispatcher::doChokingAction()
{
  BtChokingEvent event;

  std::vector<SharedHandle<BtMessage> > tempQueue
    (messageQueue_.begin(), messageQueue_.end());
  forEachMemFunSH(tempQueue.begin(), tempQueue.end(),
                  &BtMessage::onChokingEvent, event);
}

namespace {
class ProcessStaleRequestSlot {
private:
  cuid_t cuid_;
  SharedHandle<Peer> peer_;
  SharedHandle<PieceStorage> pieceStorage_;
  BtMessageDispatcher* messageDispatcher_;
  BtMessageFactory* messageFactory_;
  time_t requestTimeout_;
public:
  ProcessStaleRequestSlot
  (cuid_t cuid, const SharedHandle<Peer>& peer,
   const SharedHandle<PieceStorage>& pieceStorage,
   BtMessageDispatcher* dispatcher,
   BtMessageFactory* factory,
   time_t requestTimeout)
    : cuid_(cuid),
      peer_(peer),
      pieceStorage_(pieceStorage),
      messageDispatcher_(dispatcher),
      messageFactory_(factory),
      requestTimeout_(requestTimeout)
  {}

  void operator()(const RequestSlot& slot)
  {
    if(slot.isTimeout(requestTimeout_)) {
      A2_LOG_DEBUG(fmt(MSG_DELETING_REQUEST_SLOT_TIMEOUT,
                       util::itos(cuid_).c_str(),
                       static_cast<unsigned long>(slot.getIndex()),
                       slot.getBegin(),
                       static_cast<unsigned long>(slot.getBlockIndex())));
      slot.getPiece()->cancelBlock(slot.getBlockIndex());
      peer_->snubbing(true);
    } else if(slot.getPiece()->hasBlock(slot.getBlockIndex())) {
      A2_LOG_DEBUG(fmt(MSG_DELETING_REQUEST_SLOT_ACQUIRED,
                       util::itos(cuid_).c_str(),
                       static_cast<unsigned long>(slot.getIndex()),
                       slot.getBegin(),
                       static_cast<unsigned long>(slot.getBlockIndex())));
      messageDispatcher_->addMessageToQueue
        (messageFactory_->createCancelMessage(slot.getIndex(),
                                              slot.getBegin(),
                                              slot.getLength()));
    }
  }
};
} // namespace

namespace {
class FindStaleRequestSlot {
private:
  SharedHandle<PieceStorage> pieceStorage_;
  time_t requestTimeout_;
public:
  FindStaleRequestSlot(const SharedHandle<PieceStorage>& pieceStorage,
                       time_t requestTimeout):
    pieceStorage_(pieceStorage),
    requestTimeout_(requestTimeout) {}

  bool operator()(const RequestSlot& slot)
  {
    if(slot.isTimeout(requestTimeout_)) {
      return true;
    } else {
      if(slot.getPiece()->hasBlock(slot.getBlockIndex())) {
        return true;
      } else {
        return false;
      }
    }
  }
};
} // namespace

void DefaultBtMessageDispatcher::checkRequestSlotAndDoNecessaryThing()
{
  std::for_each(requestSlots_.begin(), requestSlots_.end(),
                ProcessStaleRequestSlot(cuid,
                                        peer_,
                                        pieceStorage_,
                                        this,
                                        messageFactory_,
                                        requestTimeout_));
  requestSlots_.erase(std::remove_if(requestSlots_.begin(), requestSlots_.end(),
                                     FindStaleRequestSlot(pieceStorage_,
                                                          requestTimeout_)),
                      requestSlots_.end());
}

bool DefaultBtMessageDispatcher::isSendingInProgress()
{
  if(messageQueue_.empty()) {
    return false;
  } else {
    return messageQueue_.front()->isSendingInProgress();
  }
}

namespace {
class BlockIndexLess {
public:
  bool operator()(const RequestSlot& lhs, const RequestSlot& rhs) const
  {
    if(lhs.getIndex() == rhs.getIndex()) {
      return lhs.getBlockIndex() < rhs.getBlockIndex();
    } else {
      return lhs.getIndex() < rhs.getIndex();
    }
  }
};
} // namespace

bool DefaultBtMessageDispatcher::isOutstandingRequest
(size_t index, size_t blockIndex) {
  RequestSlot rs(index, 0, 0, blockIndex);

  std::deque<RequestSlot>::iterator i =
    std::lower_bound(requestSlots_.begin(), requestSlots_.end(),
                     rs, BlockIndexLess());
  return i != requestSlots_.end() &&
    (*i).getIndex() == index && (*i).getBlockIndex() == blockIndex;
}

RequestSlot
DefaultBtMessageDispatcher::getOutstandingRequest
(size_t index, uint32_t begin, size_t length)
{
  RequestSlot ret;
  RequestSlot rs(index, begin, length, 0);
  std::deque<RequestSlot>::iterator i =
    std::lower_bound(requestSlots_.begin(), requestSlots_.end(), rs);
  if(i != requestSlots_.end() && (*i) == rs) {
    ret = *i;
  } else {
    ret = RequestSlot::nullSlot;
  }
  return ret;
}

void DefaultBtMessageDispatcher::removeOutstandingRequest
(const RequestSlot& slot)
{
  std::deque<RequestSlot>::iterator i =
    std::lower_bound(requestSlots_.begin(), requestSlots_.end(), slot);
  if(i != requestSlots_.end() && (*i) == slot) {
    AbortOutstandingRequest(slot.getPiece(), cuid)(*i);
    requestSlots_.erase(i);
  }
}

void DefaultBtMessageDispatcher::addOutstandingRequest
(const RequestSlot& slot)
{
  std::deque<RequestSlot>::iterator i =
    std::lower_bound(requestSlots_.begin(), requestSlots_.end(), slot);
  if(i == requestSlots_.end() || (*i) != slot) {
    requestSlots_.insert(i, slot);
  }
}

size_t DefaultBtMessageDispatcher::countOutstandingUpload()
{
  return std::count_if(messageQueue_.begin(), messageQueue_.end(),
                       mem_fun_sh(&BtMessage::isUploading));
}

void DefaultBtMessageDispatcher::setPeer(const SharedHandle<Peer>& peer)
{
  peer_ = peer;
}

void DefaultBtMessageDispatcher::setDownloadContext
(const SharedHandle<DownloadContext>& downloadContext)
{
  downloadContext_ = downloadContext;
}

void DefaultBtMessageDispatcher::setPieceStorage
(const SharedHandle<PieceStorage>& pieceStorage)
{
  pieceStorage_ = pieceStorage;
}

void DefaultBtMessageDispatcher::setPeerStorage
(const SharedHandle<PeerStorage>& peerStorage)
{
  peerStorage_ = peerStorage;
}

void DefaultBtMessageDispatcher::setBtMessageFactory(BtMessageFactory* factory)
{
  messageFactory_ = factory;
}

void DefaultBtMessageDispatcher::setRequestGroupMan(RequestGroupMan* rgman)
{
  requestGroupMan_ = rgman;
}

} // namespace aria2
