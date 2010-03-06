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
#include "BtLeecherStateChoke.h"

#include <algorithm>

#include "Peer.h"
#include "Logger.h"
#include "LogFactory.h"
#include "a2time.h"
#include "SimpleRandomizer.h"
#include "wallclock.h"

namespace aria2 {

BtLeecherStateChoke::BtLeecherStateChoke():
  _round(0),
  _lastRound(0),
  _logger(LogFactory::getInstance()) {}

BtLeecherStateChoke::~BtLeecherStateChoke() {}

BtLeecherStateChoke::PeerEntry::PeerEntry(const SharedHandle<Peer>& peer):
  _peer(peer), _downloadSpeed(peer->calculateDownloadSpeed()),
  // peer must be interested to us and sent block in the last 30 seconds
  _regularUnchoker
  (peer->peerInterested() &&
   peer->getLastDownloadUpdate().difference(global::wallclock) < 30) {}

const SharedHandle<Peer>& BtLeecherStateChoke::PeerEntry::getPeer() const
{
  return _peer;
}

unsigned int BtLeecherStateChoke::PeerEntry::getDownloadSpeed() const
{
  return _downloadSpeed;
}

bool BtLeecherStateChoke::PeerEntry::isRegularUnchoker() const
{
  return _regularUnchoker;
}

void BtLeecherStateChoke::PeerEntry::enableChokingRequired()
{
  _peer->chokingRequired(true);
}

void BtLeecherStateChoke::PeerEntry::disableChokingRequired()
{
  _peer->chokingRequired(false);
}

void BtLeecherStateChoke::PeerEntry::enableOptUnchoking()
{
  _peer->optUnchoking(true);
}

void BtLeecherStateChoke::PeerEntry::disableOptUnchoking()
{
  _peer->optUnchoking(false);
}

bool BtLeecherStateChoke::PeerEntry::isSnubbing() const
{
  return _peer->snubbing();
}

bool BtLeecherStateChoke::PeerEntry::operator<(const PeerEntry& peerEntry) const
{
  return _downloadSpeed > peerEntry._downloadSpeed;
}

class PeerFilter {
private:
  bool _amChoking;
  bool _peerInterested;
public:
  PeerFilter(bool amChoking, bool peerInterested):
    _amChoking(amChoking),
    _peerInterested(peerInterested) {}

  bool operator()(const BtLeecherStateChoke::PeerEntry& peerEntry) const
  {
    return peerEntry.getPeer()->amChoking() == _amChoking &&
      peerEntry.getPeer()->peerInterested() == _peerInterested;
  }
};

void BtLeecherStateChoke::plannedOptimisticUnchoke
(std::vector<PeerEntry>& peerEntries)
{
  std::for_each(peerEntries.begin(), peerEntries.end(),
                std::mem_fun_ref(&PeerEntry::disableOptUnchoking));
  
  std::vector<PeerEntry>::iterator i =
    std::partition(peerEntries.begin(), peerEntries.end(),
                   PeerFilter(true, true));
  if(i != peerEntries.begin()) {
    std::random_shuffle(peerEntries.begin(), i,
                        *(SimpleRandomizer::getInstance().get()));
    (*peerEntries.begin()).enableOptUnchoking();
    _logger->info("POU: %s", (*peerEntries.begin()).getPeer()->ipaddr.c_str());
  }
}

void BtLeecherStateChoke::regularUnchoke(std::vector<PeerEntry>& peerEntries)
{
  std::vector<PeerEntry>::iterator rest =
    std::partition(peerEntries.begin(), peerEntries.end(),
                   std::mem_fun_ref(&PeerEntry::isRegularUnchoker));
  
  std::sort(peerEntries.begin(), rest);

  // the number of regular unchokers
  int count = 3;

  bool fastOptUnchoker = false;
  std::vector<PeerEntry>::iterator peerIter = peerEntries.begin();
  for(;peerIter != rest && count; ++peerIter, --count) {
    (*peerIter).disableChokingRequired();
    _logger->info("RU: %s, dlspd=%u",
                  (*peerIter).getPeer()->ipaddr.c_str(),
                  (*peerIter).getDownloadSpeed());
    if((*peerIter).getPeer()->optUnchoking()) {
      fastOptUnchoker = true;
      (*peerIter).disableOptUnchoking();
    }
  }
  if(fastOptUnchoker) {
    std::random_shuffle(peerIter, peerEntries.end(),
                        *(SimpleRandomizer::getInstance().get()));
    for(std::vector<PeerEntry>::iterator i = peerIter,
          eoi = peerEntries.end(); i != eoi; ++i) {
      if((*i).getPeer()->peerInterested()) {
        (*i).enableOptUnchoking();
        _logger->info("OU: %s", (*i).getPeer()->ipaddr.c_str());
        break;
      } else {
        (*i).disableChokingRequired();
        _logger->info("OU: %s", (*i).getPeer()->ipaddr.c_str());
      }
    }
  }
}

class BtLeecherStateChokeGenPeerEntry {
public:
  BtLeecherStateChoke::PeerEntry operator()
  (const SharedHandle<Peer>& peer) const
  {
    return BtLeecherStateChoke::PeerEntry(peer);
  }
};

void
BtLeecherStateChoke::executeChoke
(const std::vector<SharedHandle<Peer> >& peerSet)
{
  _logger->info("Leecher state, %d choke round started", _round);
  _lastRound.reset();

  std::vector<PeerEntry> peerEntries;
  std::transform(peerSet.begin(), peerSet.end(),
                 std::back_inserter(peerEntries),
                 BtLeecherStateChokeGenPeerEntry());

  peerEntries.erase(std::remove_if(peerEntries.begin(), peerEntries.end(),
                                   std::mem_fun_ref(&PeerEntry::isSnubbing)),
                    peerEntries.end());
              
  std::for_each(peerEntries.begin(), peerEntries.end(),
                std::mem_fun_ref(&PeerEntry::enableChokingRequired));

  // planned optimistic unchoke
  if(_round == 0) {
    plannedOptimisticUnchoke(peerEntries);
  }
  regularUnchoke(peerEntries);

  if(++_round == 3) {
    _round = 0;
  }
}

const Time& BtLeecherStateChoke::getLastRound() const
{
  return _lastRound;
}

} // namespace aria2
