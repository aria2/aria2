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
#include "PeerConnection.h"
#include "BtCancelMessage.h"

namespace aria2 {

DefaultBtMessageDispatcher::DefaultBtMessageDispatcher()
    : cuid_{0},
      downloadContext_{nullptr},
      peerConnection_{nullptr},
      messageFactory_{nullptr},
      requestGroupMan_{nullptr},
      requestTimeout_{0}
{
}

DefaultBtMessageDispatcher::~DefaultBtMessageDispatcher()
{
  A2_LOG_DEBUG("DefaultBtMessageDispatcher::deleted");
}

void DefaultBtMessageDispatcher::addMessageToQueue(
    std::unique_ptr<BtMessage> btMessage)
{
  btMessage->onQueued();
  messageQueue_.push_back(std::move(btMessage));
}

void DefaultBtMessageDispatcher::sendMessagesInternal()
{
  auto tempQueue = std::vector<std::unique_ptr<BtMessage>>{};
  while (!messageQueue_.empty()) {
    auto msg = std::move(messageQueue_.front());
    messageQueue_.pop_front();
    if (msg->isUploading()) {
      if (requestGroupMan_->doesOverallUploadSpeedExceed() ||
          downloadContext_->getOwnerRequestGroup()->doesUploadSpeedExceed()) {
        tempQueue.push_back(std::move(msg));
        continue;
      }
    }
    msg->send();
  }
  if (!tempQueue.empty()) {
    messageQueue_.insert(std::begin(messageQueue_),
                         std::make_move_iterator(std::begin(tempQueue)),
                         std::make_move_iterator(std::end(tempQueue)));
  }
}

void DefaultBtMessageDispatcher::sendMessages()
{
  if (peerConnection_->getBufferEntrySize() < A2_IOV_MAX) {
    sendMessagesInternal();
  }
  peerConnection_->sendPendingData();
}

namespace {
std::vector<BtMessage*>
toRawPointers(const std::deque<std::unique_ptr<BtMessage>>& v)
{
  auto x = std::vector<BtMessage*>{};
  x.reserve(v.size());
  for (auto& i : v) {
    x.push_back(i.get());
  }
  return x;
}
} // namespace

// Cancel sending piece message to peer.
void DefaultBtMessageDispatcher::doCancelSendingPieceAction(size_t index,
                                                            int32_t begin,
                                                            int32_t length)
{
  BtCancelSendingPieceEvent event(index, begin, length);
  auto q = toRawPointers(messageQueue_);
  for (auto i : q) {
    i->onCancelSendingPieceEvent(event);
  }
}

// Cancel sending piece message to peer.
// TODO Is this method really necessary?
void DefaultBtMessageDispatcher::doCancelSendingPieceAction(
    const std::shared_ptr<Piece>& piece)
{
}

namespace {
void abortOutstandingRequest(const RequestSlot* slot,
                             const std::shared_ptr<Piece>& piece, cuid_t cuid)
{
  A2_LOG_DEBUG(fmt(MSG_DELETING_REQUEST_SLOT, cuid,
                   static_cast<unsigned long>(slot->getIndex()),
                   slot->getBegin(),
                   static_cast<unsigned long>(slot->getBlockIndex())));
  piece->cancelBlock(slot->getBlockIndex());
}
} // namespace

// localhost cancels outstanding download requests to the peer.
void DefaultBtMessageDispatcher::doAbortOutstandingRequestAction(
    const std::shared_ptr<Piece>& piece)
{
  for (auto& slot : requestSlots_) {
    if (slot->getIndex() == piece->getIndex()) {
      abortOutstandingRequest(slot.get(), piece, cuid_);
    }
  }
  requestSlots_.erase(
      std::remove_if(std::begin(requestSlots_), std::end(requestSlots_),
                     [&](const std::unique_ptr<RequestSlot>& slot) {
                       return slot->getIndex() == piece->getIndex();
                     }),
      std::end(requestSlots_));

  BtAbortOutstandingRequestEvent event(piece);

  auto tempQueue = toRawPointers(messageQueue_);
  for (auto i : tempQueue) {
    i->onAbortOutstandingRequestEvent(event);
  }
}

// localhost received choke message from the peer.
void DefaultBtMessageDispatcher::doChokedAction()
{
  for (auto& slot : requestSlots_) {
    if (!peer_->isInPeerAllowedIndexSet(slot->getIndex())) {
      A2_LOG_DEBUG(fmt(MSG_DELETING_REQUEST_SLOT_CHOKED, cuid_,
                       static_cast<unsigned long>(slot->getIndex()),
                       slot->getBegin(),
                       static_cast<unsigned long>(slot->getBlockIndex())));
      slot->getPiece()->cancelBlock(slot->getBlockIndex());
    }
  }
  requestSlots_.erase(
      std::remove_if(std::begin(requestSlots_), std::end(requestSlots_),
                     [&](const std::unique_ptr<RequestSlot>& slot) {
                       return !peer_->isInPeerAllowedIndexSet(slot->getIndex());
                     }),
      std::end(requestSlots_));
}

// localhost dispatched choke message to the peer.
void DefaultBtMessageDispatcher::doChokingAction()
{
  BtChokingEvent event;

  auto tempQueue = toRawPointers(messageQueue_);
  for (auto i : tempQueue) {
    i->onChokingEvent(event);
  }
}

void DefaultBtMessageDispatcher::checkRequestSlotAndDoNecessaryThing()
{
  for (auto& slot : requestSlots_) {
    if (slot->isTimeout(requestTimeout_)) {
      A2_LOG_DEBUG(fmt(MSG_DELETING_REQUEST_SLOT_TIMEOUT, cuid_,
                       static_cast<unsigned long>(slot->getIndex()),
                       slot->getBegin(),
                       static_cast<unsigned long>(slot->getBlockIndex())));
      slot->getPiece()->cancelBlock(slot->getBlockIndex());
      peer_->snubbing(true);
    }
    else if (slot->getPiece()->hasBlock(slot->getBlockIndex())) {
      A2_LOG_DEBUG(fmt(MSG_DELETING_REQUEST_SLOT_ACQUIRED, cuid_,
                       static_cast<unsigned long>(slot->getIndex()),
                       slot->getBegin(),
                       static_cast<unsigned long>(slot->getBlockIndex())));
      addMessageToQueue(messageFactory_->createCancelMessage(
          slot->getIndex(), slot->getBegin(), slot->getLength()));
    }
  }
  requestSlots_.erase(
      std::remove_if(std::begin(requestSlots_), std::end(requestSlots_),
                     [&](const std::unique_ptr<RequestSlot>& slot) {
                       return slot->isTimeout(requestTimeout_) ||
                              slot->getPiece()->hasBlock(slot->getBlockIndex());
                     }),
      std::end(requestSlots_));
}

bool DefaultBtMessageDispatcher::isSendingInProgress()
{
  return peerConnection_->getBufferEntrySize();
}

bool DefaultBtMessageDispatcher::isOutstandingRequest(size_t index,
                                                      size_t blockIndex)
{
  for (auto& slot : requestSlots_) {
    if (slot->getIndex() == index && slot->getBlockIndex() == blockIndex) {
      return true;
    }
  }
  return false;
}

const RequestSlot*
DefaultBtMessageDispatcher::getOutstandingRequest(size_t index, int32_t begin,
                                                  int32_t length)
{
  for (auto& slot : requestSlots_) {
    if (slot->getIndex() == index && slot->getBegin() == begin &&
        slot->getLength() == length) {
      return slot.get();
    }
  }
  return nullptr;
}

void DefaultBtMessageDispatcher::removeOutstandingRequest(
    const RequestSlot* slot)
{
  for (auto i = std::begin(requestSlots_), eoi = std::end(requestSlots_);
       i != eoi; ++i) {
    if (*(*i) == *slot) {
      abortOutstandingRequest((*i).get(), (*i)->getPiece(), cuid_);
      requestSlots_.erase(i);
      break;
    }
  }
}

void DefaultBtMessageDispatcher::addOutstandingRequest(
    std::unique_ptr<RequestSlot> slot)
{
  requestSlots_.push_back(std::move(slot));
}

size_t DefaultBtMessageDispatcher::countOutstandingUpload()
{
  return std::count_if(std::begin(messageQueue_), std::end(messageQueue_),
                       std::mem_fn(&BtMessage::isUploading));
}

void DefaultBtMessageDispatcher::setPeer(const std::shared_ptr<Peer>& peer)
{
  peer_ = peer;
}

void DefaultBtMessageDispatcher::setDownloadContext(
    DownloadContext* downloadContext)
{
  downloadContext_ = downloadContext;
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
