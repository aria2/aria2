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

const size_t MAX_PEER_LIST_SIZE = 1024;
const size_t MAX_PEER_LIST_UPDATE = 100;

} // namespace

DefaultPeerStorage::DefaultPeerStorage()
  : maxPeerListSize_(MAX_PEER_LIST_SIZE),
    removedPeerSessionDownloadLength_(0LL),
    removedPeerSessionUploadLength_(0LL),
    seederStateChoke_(new BtSeederStateChoke()),
    leecherStateChoke_(new BtLeecherStateChoke()),
    lastTransferStatMapUpdated_(0)
{}

DefaultPeerStorage::~DefaultPeerStorage()
{
  delete seederStateChoke_;
  delete leecherStateChoke_;
}

namespace {
class FindIdenticalPeer {
private:
  SharedHandle<Peer> peer_;
public:
  FindIdenticalPeer(const SharedHandle<Peer>& peer):peer_(peer) {}

  bool operator()(const SharedHandle<Peer>& peer) const {
    return (*peer_ == *peer) ||
      ((peer_->getIPAddress() == peer->getIPAddress()) &&
       (peer_->getPort() == peer->getPort()));
  }
};
} // namespace

bool DefaultPeerStorage::isPeerAlreadyAdded(const SharedHandle<Peer>& peer)
{
  return std::find_if(peers_.begin(), peers_.end(),
                      FindIdenticalPeer(peer)) != peers_.end();
}

bool DefaultPeerStorage::addPeer(const SharedHandle<Peer>& peer) {
  if(isPeerAlreadyAdded(peer)) {
    A2_LOG_DEBUG(fmt("Adding %s:%u is rejected because it has been already"
                     " added.",
                     peer->getIPAddress().c_str(), peer->getPort()));
    return false;
  }
  if(isBadPeer(peer->getIPAddress())) {
    A2_LOG_DEBUG(fmt("Adding %s:%u is rejected because it is marked bad.",
                     peer->getIPAddress().c_str(), peer->getPort()));
    return false;
  }
  const size_t peerListSize = peers_.size();
  if(peerListSize >= maxPeerListSize_) {
    deleteUnusedPeer(peerListSize-maxPeerListSize_+1);
  }
  peers_.push_front(peer);
  A2_LOG_DEBUG(fmt("Now peer list contains %lu peers",
                   static_cast<unsigned long>(peers_.size())));
  return true;
}

void DefaultPeerStorage::addPeer(const std::vector<SharedHandle<Peer> >& peers)
{
  size_t added = 0;
  size_t addMax = std::min(maxPeerListSize_, MAX_PEER_LIST_UPDATE);
  for(std::vector<SharedHandle<Peer> >::const_iterator itr = peers.begin(),
        eoi = peers.end(); itr != eoi && added < addMax; ++itr) {
    const SharedHandle<Peer>& peer = *itr;
    if(isPeerAlreadyAdded(peer)) {
      A2_LOG_DEBUG(fmt("Adding %s:%u is rejected because it has been already"
                       " added.",
                       peer->getIPAddress().c_str(), peer->getPort()));
      continue;
    } else if(isBadPeer(peer->getIPAddress())) {
      A2_LOG_DEBUG(fmt("Adding %s:%u is rejected because it is marked bad.",
                       peer->getIPAddress().c_str(), peer->getPort()));
      continue;
    } else {
      A2_LOG_DEBUG(fmt(MSG_ADDING_PEER,
                       peer->getIPAddress().c_str(), peer->getPort()));
    }
    peers_.push_front(peer);
    ++added;
  }
  const size_t peerListSize = peers_.size();
  if(peerListSize >= maxPeerListSize_) {
    deleteUnusedPeer(peerListSize-maxPeerListSize_);
  }
  A2_LOG_DEBUG(fmt("Now peer list contains %lu peers",
                   static_cast<unsigned long>(peers_.size())));
}

void DefaultPeerStorage::addDroppedPeer(const SharedHandle<Peer>& peer)
{
  droppedPeers_.push_front(peer);
  if(droppedPeers_.size() > 50) {
    droppedPeers_.pop_back();
  }
}

const std::deque<SharedHandle<Peer> >& DefaultPeerStorage::getPeers()
{
  return peers_;
}

const std::deque<SharedHandle<Peer> >& DefaultPeerStorage::getDroppedPeers()
{
  return droppedPeers_;
}

namespace {
class FindFinePeer {
public:
  bool operator()(const SharedHandle<Peer>& peer) const {
    return peer->unused() && peer->isGood();
  }
};
} // namespace

SharedHandle<Peer> DefaultPeerStorage::getUnusedPeer() {
  std::deque<SharedHandle<Peer> >::const_iterator itr =
    std::find_if(peers_.begin(), peers_.end(), FindFinePeer());
  if(itr == peers_.end()) {
    return SharedHandle<Peer>();
  } else {
    return *itr;
  }
}

namespace {
class FindPeer {
private:
  std::string ipaddr;
  uint16_t port;
public:
  FindPeer(const std::string& ipaddr, uint16_t port):
    ipaddr(ipaddr), port(port) {}

  bool operator()(const SharedHandle<Peer>& peer) const {
    return ipaddr == peer->getIPAddress() && port == peer->getPort();
  }
};
} // namespace

SharedHandle<Peer> DefaultPeerStorage::getPeer(const std::string& ipaddr,
                                               uint16_t port) const {
  std::deque<SharedHandle<Peer> >::const_iterator itr =
    std::find_if(peers_.begin(), peers_.end(), FindPeer(ipaddr, port));
  if(itr == peers_.end()) {
    return SharedHandle<Peer>();
  } else {
    return *itr;
  }
}

size_t DefaultPeerStorage::countPeer() const {
  return peers_.size();
}

bool DefaultPeerStorage::isPeerAvailable() {
  return getUnusedPeer();
}

namespace {
class CollectActivePeer {
private:
  std::vector<SharedHandle<Peer> >& activePeers_;
public:
  CollectActivePeer(std::vector<SharedHandle<Peer> >& activePeers):
    activePeers_(activePeers) {}

  void operator()(const SharedHandle<Peer>& peer)
  {
    if(peer->isActive()) {
      activePeers_.push_back(peer);
    }
  }
};
} // namespace

void DefaultPeerStorage::getActivePeers
(std::vector<SharedHandle<Peer> >& activePeers)
{
  std::for_each(peers_.begin(), peers_.end(), CollectActivePeer(activePeers));
}

namespace {
TransferStat calculateStatFor(const SharedHandle<Peer>& peer)
{
  TransferStat s;
  s.downloadSpeed = peer->calculateDownloadSpeed();
  s.uploadSpeed = peer->calculateUploadSpeed();
  s.sessionDownloadLength = peer->getSessionDownloadLength();
  s.sessionUploadLength = peer->getSessionUploadLength();
  return s;
}
} // namespace

TransferStat DefaultPeerStorage::calculateStat()
{
  TransferStat stat;
  if(lastTransferStatMapUpdated_.differenceInMillis(global::wallclock())+
     A2_DELTA_MILLIS >= 250) {
    A2_LOG_DEBUG("Updating TransferStat of PeerStorage");
    lastTransferStatMapUpdated_ = global::wallclock();
    peerTransferStatMap_.clear();
    std::vector<SharedHandle<Peer> > activePeers;
    getActivePeers(activePeers);
    for(std::vector<SharedHandle<Peer> >::const_iterator i =
          activePeers.begin(), eoi = activePeers.end(); i != eoi; ++i) {
      TransferStat s;
      s.downloadSpeed = (*i)->calculateDownloadSpeed();
      s.uploadSpeed = (*i)->calculateUploadSpeed();
      s.sessionDownloadLength = (*i)->getSessionDownloadLength();
      s.sessionUploadLength = (*i)->getSessionUploadLength();

      peerTransferStatMap_[(*i)->getID()] = calculateStatFor(*i);
      stat += s;
    }
    cachedTransferStat_ = stat;
  } else {
    stat = cachedTransferStat_;
  }
  stat.sessionDownloadLength += removedPeerSessionDownloadLength_;
  stat.sessionUploadLength += removedPeerSessionUploadLength_;
  stat.setAllTimeUploadLength(btRuntime_->getUploadLengthAtStartup()+
                              stat.getSessionUploadLength());
  return stat;
}

void DefaultPeerStorage::updateTransferStatFor(const SharedHandle<Peer>& peer)
{
  A2_LOG_DEBUG(fmt("Updating TransferStat for peer %s", peer->getID().c_str()));
  std::map<std::string, TransferStat>::iterator itr =
    peerTransferStatMap_.find(peer->getID());
  if(itr == peerTransferStatMap_.end()) {
    return;
  }
  cachedTransferStat_ -= (*itr).second;
  TransferStat s = calculateStatFor(peer);
  cachedTransferStat_ += s;
  (*itr).second = s;
}

TransferStat DefaultPeerStorage::getTransferStatFor
(const SharedHandle<Peer>& peer)
{
  std::map<std::string, TransferStat>::const_iterator itr =
    peerTransferStatMap_.find(peer->getID());
  if(itr == peerTransferStatMap_.end()) {
    return TransferStat();
  } else {
    return (*itr).second;
  }
}

bool DefaultPeerStorage::isBadPeer(const std::string& ipaddr)
{
  std::map<std::string, time_t>::iterator i = badPeers_.find(ipaddr);
  if(i == badPeers_.end()) {
    return false;
  } else if(global::wallclock().getTime() >= (*i).second) {
    badPeers_.erase(i);
    return false;
  } else {
    return true;
  }
}

void DefaultPeerStorage::addBadPeer(const std::string& ipaddr)
{
  if(lastBadPeerCleaned_.difference(global::wallclock()) >= 3600) {
    for(std::map<std::string, time_t>::iterator i = badPeers_.begin(),
          eoi = badPeers_.end(); i != eoi;) {
      if(global::wallclock().getTime() >= (*i).second) {
        A2_LOG_DEBUG(fmt("Purge %s from bad peer", (*i).first.c_str()));
        badPeers_.erase(i++);
        eoi = badPeers_.end();
      } else {
        ++i;
      }
    }
    lastBadPeerCleaned_ = global::wallclock();
  }
  A2_LOG_DEBUG(fmt("Added %s as bad peer", ipaddr.c_str()));
  // We use variable timeout to avoid many bad peers wake up at once.
  badPeers_[ipaddr] = global::wallclock().getTime()+
    std::max(SimpleRandomizer::getInstance()->getRandomNumber(601), 120L);
}

void DefaultPeerStorage::deleteUnusedPeer(size_t delSize) {
  std::deque<SharedHandle<Peer> > temp;
  for(std::deque<SharedHandle<Peer> >::const_reverse_iterator itr =
        peers_.rbegin(), eoi = peers_.rend(); itr != eoi; ++itr) {
    const SharedHandle<Peer>& p = *itr;
    if(p->unused() && delSize > 0) {
      onErasingPeer(p);
      --delSize;
    } else {
      temp.push_front(p);
    }
  }
  peers_.swap(temp);
}

void DefaultPeerStorage::onErasingPeer(const SharedHandle<Peer>& peer) {}

void DefaultPeerStorage::onReturningPeer(const SharedHandle<Peer>& peer)
{
  if(peer->isActive()) {
    TransferStat removedStat(calculateStatFor(peer));
    removedPeerSessionDownloadLength_ += removedStat.getSessionDownloadLength();
    removedPeerSessionUploadLength_ += removedStat.getSessionUploadLength();
    cachedTransferStat_ -= removedStat;

    if(peer->isDisconnectedGracefully() && !peer->isIncomingPeer()) {
      peer->startBadCondition();
      addDroppedPeer(peer);
    }
    // Execute choking algorithm if unchoked and interested peer is
    // disconnected.
    if(!peer->amChoking() && peer->peerInterested()) {
      executeChoke();
    }
  }
}

void DefaultPeerStorage::returnPeer(const SharedHandle<Peer>& peer)
{
  std::deque<SharedHandle<Peer> >::iterator itr =
    std::find_if(peers_.begin(), peers_.end(), derefEqual(peer));
  if(itr == peers_.end()) {
    A2_LOG_DEBUG(fmt("Cannot find peer %s:%u in PeerStorage.",
                     peer->getIPAddress().c_str(), peer->getPort()));
  } else {
    peers_.erase(itr);

    onReturningPeer(peer);
    onErasingPeer(peer);
  }
}

bool DefaultPeerStorage::chokeRoundIntervalElapsed()
{
  const time_t CHOKE_ROUND_INTERVAL = 10;
  if(pieceStorage_->downloadFinished()) {
    return seederStateChoke_->getLastRound().
      difference(global::wallclock()) >= CHOKE_ROUND_INTERVAL;
  } else {
    return leecherStateChoke_->getLastRound().
      difference(global::wallclock()) >= CHOKE_ROUND_INTERVAL;
  }
}

void DefaultPeerStorage::executeChoke()
{
  std::vector<SharedHandle<Peer> > activePeers;
  getActivePeers(activePeers);
  if(pieceStorage_->downloadFinished()) {
    return seederStateChoke_->executeChoke(activePeers);
  } else {
    return leecherStateChoke_->executeChoke(activePeers);
  }
}

void DefaultPeerStorage::setPieceStorage(const SharedHandle<PieceStorage>& ps)
{
  pieceStorage_ = ps;
}

void DefaultPeerStorage::setBtRuntime(const SharedHandle<BtRuntime>& btRuntime)
{
  btRuntime_ = btRuntime;
}

} // namespace aria2
