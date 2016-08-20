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
#include "DefaultPeerStorage.h"

#include <algorithm>

#include "LogFactory.h"
#include "Logger.h"
#include "message.h"
#include "Peer.h"
#include "BtRuntime.h"
#include "BtSeederStateChoke.h"
#include "BtLeecherStateChoke.h"
#include "PieceStorage.h"
#include "wallclock.h"
#include "a2functional.h"
#include "fmt.h"
#include "SimpleRandomizer.h"

namespace aria2 {

namespace {

const size_t MAX_PEER_LIST_SIZE = 512;

} // namespace

DefaultPeerStorage::DefaultPeerStorage()
    : maxPeerListSize_(MAX_PEER_LIST_SIZE),
      seederStateChoke_(make_unique<BtSeederStateChoke>()),
      leecherStateChoke_(make_unique<BtLeecherStateChoke>()),
      lastTransferStatMapUpdated_(Timer::zero())
{
}

DefaultPeerStorage::~DefaultPeerStorage()
{
  assert(uniqPeers_.size() == unusedPeers_.size() + usedPeers_.size());
}

size_t DefaultPeerStorage::countAllPeer() const
{
  return unusedPeers_.size() + usedPeers_.size();
}

bool DefaultPeerStorage::isPeerAlreadyAdded(const std::shared_ptr<Peer>& peer)
{
  return uniqPeers_.count(
      std::make_pair(peer->getIPAddress(), peer->getOrigPort()));
}

void DefaultPeerStorage::addUniqPeer(const std::shared_ptr<Peer>& peer)
{
  uniqPeers_.insert(std::make_pair(peer->getIPAddress(), peer->getOrigPort()));
}

bool DefaultPeerStorage::addPeer(const std::shared_ptr<Peer>& peer)
{
  if (unusedPeers_.size() >= maxPeerListSize_) {
    A2_LOG_DEBUG(fmt("Adding %s:%u is rejected, since unused peer list is full "
                     "(%lu peers > %lu)",
                     peer->getIPAddress().c_str(), peer->getPort(),
                     static_cast<unsigned long>(unusedPeers_.size()),
                     static_cast<unsigned long>(maxPeerListSize_)));
    return false;
  }
  if (isPeerAlreadyAdded(peer)) {
    A2_LOG_DEBUG(fmt("Adding %s:%u is rejected because it has been already"
                     " added.",
                     peer->getIPAddress().c_str(), peer->getPort()));
    return false;
  }
  if (isBadPeer(peer->getIPAddress())) {
    A2_LOG_DEBUG(fmt("Adding %s:%u is rejected because it is marked bad.",
                     peer->getIPAddress().c_str(), peer->getPort()));
    return false;
  }
  const size_t peerListSize = unusedPeers_.size();
  if (peerListSize >= maxPeerListSize_) {
    deleteUnusedPeer(peerListSize - maxPeerListSize_ + 1);
  }
  unusedPeers_.push_back(peer);
  addUniqPeer(peer);
  A2_LOG_DEBUG(fmt("Now unused peer list contains %lu peers",
                   static_cast<unsigned long>(unusedPeers_.size())));
  return true;
}

void DefaultPeerStorage::addPeer(
    const std::vector<std::shared_ptr<Peer>>& peers)
{
  if (unusedPeers_.size() < maxPeerListSize_) {
    for (auto& peer : peers) {
      if (isPeerAlreadyAdded(peer)) {
        A2_LOG_DEBUG(fmt("Adding %s:%u is rejected because it has been already"
                         " added.",
                         peer->getIPAddress().c_str(), peer->getPort()));
        continue;
      }
      else if (isBadPeer(peer->getIPAddress())) {
        A2_LOG_DEBUG(fmt("Adding %s:%u is rejected because it is marked bad.",
                         peer->getIPAddress().c_str(), peer->getPort()));
        continue;
      }
      else {
        A2_LOG_DEBUG(fmt(MSG_ADDING_PEER, peer->getIPAddress().c_str(),
                         peer->getPort()));
      }
      unusedPeers_.push_back(peer);
      addUniqPeer(peer);
    }
  }
  else {
    for (auto& peer : peers) {
      A2_LOG_DEBUG(
          fmt("Adding %s:%u is rejected, since unused peer list is full "
              "(%lu peers > %lu)",
              peer->getIPAddress().c_str(), peer->getPort(),
              static_cast<unsigned long>(unusedPeers_.size()),
              static_cast<unsigned long>(maxPeerListSize_)));
    }
  }
  const size_t peerListSize = unusedPeers_.size();
  if (peerListSize > maxPeerListSize_) {
    deleteUnusedPeer(peerListSize - maxPeerListSize_);
  }
  A2_LOG_DEBUG(fmt("Now unused peer list contains %lu peers",
                   static_cast<unsigned long>(unusedPeers_.size())));
}

std::shared_ptr<Peer>
DefaultPeerStorage::addAndCheckoutPeer(const std::shared_ptr<Peer>& peer,
                                       cuid_t cuid)
{
  if (isPeerAlreadyAdded(peer)) {
    auto it = std::find_if(std::begin(unusedPeers_), std::end(unusedPeers_),
                           [&peer](const std::shared_ptr<Peer>& p) {
                             return p->getIPAddress() == peer->getIPAddress() &&
                                    p->getOrigPort() == peer->getOrigPort();
                           });
    if (it == std::end(unusedPeers_)) {
      // peer is in usedPeers_.
      return nullptr;
    }

    unusedPeers_.erase(it);
  }
  else {
    addUniqPeer(peer);
  }

  unusedPeers_.push_front(peer);

  return checkoutPeer(cuid);
}

void DefaultPeerStorage::addDroppedPeer(const std::shared_ptr<Peer>& peer)
{
  // Make sure that no duplicated peer exists in droppedPeers_. If
  // exists, erase older one.
  for (auto i = std::begin(droppedPeers_), eoi = std::end(droppedPeers_);
       i != eoi; ++i) {
    if ((*i)->getIPAddress() == peer->getIPAddress() &&
        (*i)->getPort() == peer->getPort()) {
      droppedPeers_.erase(i);
      break;
    }
  }
  droppedPeers_.push_front(peer);
  if (droppedPeers_.size() > 50) {
    droppedPeers_.pop_back();
  }
}

const std::deque<std::shared_ptr<Peer>>& DefaultPeerStorage::getUnusedPeers()
{
  return unusedPeers_;
}

const PeerSet& DefaultPeerStorage::getUsedPeers() { return usedPeers_; }

const std::deque<std::shared_ptr<Peer>>& DefaultPeerStorage::getDroppedPeers()
{
  return droppedPeers_;
}

bool DefaultPeerStorage::isPeerAvailable() { return !unusedPeers_.empty(); }

bool DefaultPeerStorage::isBadPeer(const std::string& ipaddr)
{
  auto i = badPeers_.find(ipaddr);
  if (i == std::end(badPeers_)) {
    return false;
  }

  if ((*i).second <= global::wallclock()) {
    badPeers_.erase(i);
    return false;
  }

  return true;
}

void DefaultPeerStorage::addBadPeer(const std::string& ipaddr)
{
  if (lastBadPeerCleaned_.difference(global::wallclock()) >= 1_h) {
    for (auto i = std::begin(badPeers_); i != std::end(badPeers_);) {
      if ((*i).second <= global::wallclock()) {
        A2_LOG_DEBUG(fmt("Purge %s from bad peer", (*i).first.c_str()));
        badPeers_.erase(i++);
        // badPeers_.end() will not be invalidated.
      }
      else {
        ++i;
      }
    }
    lastBadPeerCleaned_ = global::wallclock();
  }
  A2_LOG_DEBUG(fmt("Added %s as bad peer", ipaddr.c_str()));
  // We use variable timeout to avoid many bad peers wake up at once.
  auto t = global::wallclock();
  t.advance(std::chrono::seconds(
      std::max(SimpleRandomizer::getInstance()->getRandomNumber(601), 120L)));

  badPeers_[ipaddr] = std::move(t);
}

void DefaultPeerStorage::deleteUnusedPeer(size_t delSize)
{
  for (; delSize > 0 && !unusedPeers_.empty(); --delSize) {
    auto& peer = unusedPeers_.back();
    onErasingPeer(peer);
    A2_LOG_DEBUG(fmt("Remove peer %s:%u", peer->getIPAddress().c_str(),
                     peer->getOrigPort()));
    unusedPeers_.pop_back();
  }
}

std::shared_ptr<Peer> DefaultPeerStorage::checkoutPeer(cuid_t cuid)
{
  if (!isPeerAvailable()) {
    return nullptr;
  }
  auto peer = unusedPeers_.front();
  unusedPeers_.pop_front();
  if (peer->usedBy() != 0) {
    A2_LOG_WARN(fmt("CUID#%" PRId64 " is already set for peer %s:%u",
                    peer->usedBy(), peer->getIPAddress().c_str(),
                    peer->getOrigPort()));
  }
  peer->usedBy(cuid);
  usedPeers_.insert(peer);
  A2_LOG_DEBUG(fmt("Checkout peer %s:%u to CUID#%" PRId64,
                   peer->getIPAddress().c_str(), peer->getOrigPort(),
                   peer->usedBy()));
  return peer;
}

void DefaultPeerStorage::onErasingPeer(const std::shared_ptr<Peer>& peer)
{
  uniqPeers_.erase(std::make_pair(peer->getIPAddress(), peer->getOrigPort()));
}

void DefaultPeerStorage::onReturningPeer(const std::shared_ptr<Peer>& peer)
{
  if (peer->isActive()) {
    if (peer->isDisconnectedGracefully() && !peer->isIncomingPeer()) {
      peer->startDrop();
      addDroppedPeer(peer);
    }
    // Execute choking algorithm if unchoked and interested peer is
    // disconnected.
    if (!peer->amChoking() && peer->peerInterested()) {
      executeChoke();
    }
  }
  peer->usedBy(0);
}

void DefaultPeerStorage::returnPeer(const std::shared_ptr<Peer>& peer)
{
  A2_LOG_DEBUG(fmt("Peer %s:%u returned from CUID#%" PRId64,
                   peer->getIPAddress().c_str(), peer->getOrigPort(),
                   peer->usedBy()));
  if (usedPeers_.erase(peer)) {
    onReturningPeer(peer);
    onErasingPeer(peer);
  }
  else {
    A2_LOG_WARN(fmt("Cannot find peer %s:%u in usedPeers_",
                    peer->getIPAddress().c_str(), peer->getOrigPort()));
  }
}

bool DefaultPeerStorage::chokeRoundIntervalElapsed()
{
  constexpr auto CHOKE_ROUND_INTERVAL = 10_s;

  if (pieceStorage_->downloadFinished()) {
    return seederStateChoke_->getLastRound().difference(global::wallclock()) >=
           CHOKE_ROUND_INTERVAL;
  }

  return leecherStateChoke_->getLastRound().difference(global::wallclock()) >=
         CHOKE_ROUND_INTERVAL;
}

void DefaultPeerStorage::executeChoke()
{
  if (pieceStorage_->downloadFinished()) {
    return seederStateChoke_->executeChoke(usedPeers_);
  }
  else {
    return leecherStateChoke_->executeChoke(usedPeers_);
  }
}

void DefaultPeerStorage::setPieceStorage(
    const std::shared_ptr<PieceStorage>& ps)
{
  pieceStorage_ = ps;
}

void DefaultPeerStorage::setBtRuntime(
    const std::shared_ptr<BtRuntime>& btRuntime)
{
  btRuntime_ = btRuntime;
}

} // namespace aria2
