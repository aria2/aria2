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
#include "BtSeederStateChoke.h"

#include <algorithm>

#include "Peer.h"
#include "Logger.h"
#include "LogFactory.h"
#include "SimpleRandomizer.h"
#include "wallclock.h"

namespace aria2 {

BtSeederStateChoke::BtSeederStateChoke():
  _round(0),
  _lastRound(0),
  _logger(LogFactory::getInstance()) {}

BtSeederStateChoke::~BtSeederStateChoke() {}

BtSeederStateChoke::PeerEntry::PeerEntry
(const SharedHandle<Peer>& peer):
  _peer(peer),
  _outstandingUpload(peer->countOutstandingUpload()),
  _lastAmUnchoking(peer->getLastAmUnchoking()),
  _recentUnchoking(_lastAmUnchoking.difference(global::wallclock) < TIME_FRAME),
  _uploadSpeed(peer->calculateUploadSpeed())
{}

bool
BtSeederStateChoke::PeerEntry::operator<(const PeerEntry& rhs) const
{
  if(this->_outstandingUpload && !rhs._outstandingUpload) {
    return true;
  } else if(!this->_outstandingUpload && rhs._outstandingUpload) {
    return false;
  }
  if(this->_recentUnchoking &&
     this->_lastAmUnchoking.isNewer(rhs._lastAmUnchoking)) {
    return true;
  } else if(rhs._recentUnchoking) {
    return false;
  } else {
    return this->_uploadSpeed > rhs._uploadSpeed;
  }
}

void BtSeederStateChoke::PeerEntry::disableOptUnchoking()
{
  _peer->optUnchoking(false);
}

void BtSeederStateChoke::unchoke
(std::vector<BtSeederStateChoke::PeerEntry>& peers)
{
  int count = (_round == 2) ? 4 : 3;

  std::sort(peers.begin(), peers.end());

  std::vector<PeerEntry>::iterator r = peers.begin();
  for(std::vector<PeerEntry>::iterator eoi = peers.end();
      r != eoi && count; ++r, --count) {
    (*r).getPeer()->chokingRequired(false);
    _logger->info("RU: %s, ulspd=%u", (*r).getPeer()->ipaddr.c_str(),
                  (*r).getUploadSpeed());
  }

  if(_round < 2) {
    std::for_each(peers.begin(), peers.end(),
                  std::mem_fun_ref(&PeerEntry::disableOptUnchoking));
    if(r != peers.end()) {
      std::random_shuffle(r, peers.end(),
                          *(SimpleRandomizer::getInstance().get()));
      (*r).getPeer()->optUnchoking(true);
      _logger->info("POU: %s", (*r).getPeer()->ipaddr.c_str());
    }
  }
}

class ChokingRequired {
public:
  void operator()(const SharedHandle<Peer>& peer) const
  {
    peer->chokingRequired(true);
  }
};

class GenPeerEntry {
public:
  BtSeederStateChoke::PeerEntry operator()(const SharedHandle<Peer>& peer) const
  {
    return BtSeederStateChoke::PeerEntry(peer);
  }
};

class NotInterestedPeer {
public:
  bool operator()(const BtSeederStateChoke::PeerEntry& peerEntry) const
  {
    return !peerEntry.getPeer()->peerInterested();
  }
};

void
BtSeederStateChoke::executeChoke
(const std::vector<SharedHandle<Peer> >& peerSet)
{
  _logger->info("Seeder state, %d choke round started", _round);
  _lastRound.reset();

  std::vector<PeerEntry> peerEntries;

  std::for_each(peerSet.begin(), peerSet.end(), ChokingRequired());

  std::transform(peerSet.begin(), peerSet.end(),
                 std::back_inserter(peerEntries), GenPeerEntry());
              
  peerEntries.erase(std::remove_if(peerEntries.begin(), peerEntries.end(),
                                   NotInterestedPeer()),
                    peerEntries.end());

  unchoke(peerEntries);
  
  if(++_round == 3) {
    _round = 0;
  }
}

} // namespace aria2
