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
#include "BtLeecherStateChoke.h"
#include "Peer.h"
#include "Logger.h"
#include "LogFactory.h"
#include "a2time.h"
#include <algorithm>

namespace aria2 {

BtLeecherStateChoke::BtLeecherStateChoke():
  _round(0),
  _lastRound(0),
  _logger(LogFactory::getInstance()) {}

BtLeecherStateChoke::~BtLeecherStateChoke() {}

class PeerFilter {
private:
  bool _amChoking;
  bool _peerInterested;
public:
  PeerFilter(bool amChoking, bool peerInterested):
    _amChoking(amChoking),
    _peerInterested(peerInterested) {}

  bool operator()(const Peer* peer) const
  {
    return peer->amChoking() == _amChoking &&
      peer->peerInterested() == _peerInterested;
  }
};

class RegularUnchoker {
public:
  bool operator()(const Peer* peer) const
  {
    // peer must be interested to us and sent block in the last 30 seconds
    return peer->peerInterested() && !peer->getLastDownloadUpdate().elapsed(30);
  }
};

class DownloadFaster {
private:
  const struct timeval _now;
public:

  DownloadFaster(const struct timeval& now):_now(now) {}

  bool operator() (Peer* left, Peer* right) const
  {
    return left->calculateDownloadSpeed(_now) > right->calculateDownloadSpeed(_now);
  }
};

class SnubbedPeer {
public:
  bool operator() (const Peer* peer) const
  {
    return peer->snubbing();
  }
};

void BtLeecherStateChoke::plannedOptimisticUnchoke(std::deque<Peer*>& peers)
{
  std::for_each(peers.begin(), peers.end(),
		std::bind2nd(std::mem_fun((void (Peer::*)(bool))&Peer::optUnchoking), false));
  
  std::deque<Peer*>::iterator i = std::partition(peers.begin(), peers.end(), PeerFilter(true, true));
  if(i != peers.begin()) {
    std::random_shuffle(peers.begin(), i);
    (*peers.begin())->optUnchoking(true);
    _logger->info("POU: %s", (*peers.begin())->ipaddr.c_str());
  }
}

void BtLeecherStateChoke::regularUnchoke(std::deque<Peer*>& peers)
{
  std::deque<Peer*>::iterator rest = std::partition(peers.begin(), peers.end(), RegularUnchoker());
  
  struct timeval now;
  gettimeofday(&now, 0);

  std::sort(peers.begin(), rest, DownloadFaster(now));

  // the number of regular unchokers
  int count = 3;

  bool fastOptUnchoker = false;
  std::deque<Peer*>::iterator peerIter = peers.begin();
  for(;peerIter != rest && count; ++peerIter, --count) {
    (*peerIter)->chokingRequired(false);
    _logger->info("RU: %s, dlspd=%u", (*peerIter)->ipaddr.c_str(), (*peerIter)->calculateDownloadSpeed(now));
    if((*peerIter)->optUnchoking()) {
      fastOptUnchoker = true;
      (*peerIter)->optUnchoking(false);
    }
  }
  if(fastOptUnchoker) {
    std::random_shuffle(peerIter, peers.end());
    for(std::deque<Peer*>::iterator i = peerIter; i != peers.end(); ++i) {
      if((*i)->peerInterested()) {
	(*i)->optUnchoking(true);
	_logger->info("OU: %s", (*i)->ipaddr.c_str());
	break;
      } else {
	(*i)->chokingRequired(false);
	_logger->info("OU: %s", (*i)->ipaddr.c_str());
      }
    }
  }
}

void
BtLeecherStateChoke::executeChoke(const std::deque<SharedHandle<Peer> >& peerSet)
{
  _logger->info("Leecher state, %d choke round started", _round);
  _lastRound.reset();

  std::deque<Peer*> peers;
  std::transform(peerSet.begin(), peerSet.end(), std::back_inserter(peers),
		 std::mem_fun_ref(&SharedHandle<Peer>::get));

  peers.erase(std::remove_if(peers.begin(), peers.end(), SnubbedPeer()),
	      peers.end());
	      
  std::for_each(peers.begin(), peers.end(),
		std::bind2nd(std::mem_fun((void (Peer::*)(bool))&Peer::chokingRequired), true));

  // planned optimistic unchoke
  if(_round == 0) {
    plannedOptimisticUnchoke(peers);
  }
  regularUnchoke(peers);

  if(++_round == 3) {
    _round = 0;
  }
}

const Time& BtLeecherStateChoke::getLastRound() const
{
  return _lastRound;
}

} // namespace aria2
