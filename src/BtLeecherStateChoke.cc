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
#include "SimpleRandomizer.h"
#include "wallclock.h"

namespace aria2 {

BtLeecherStateChoke::BtLeecherStateChoke():
  round_(0),
  lastRound_(0),
  logger_(LogFactory::getInstance()) {}

BtLeecherStateChoke::~BtLeecherStateChoke() {}

BtLeecherStateChoke::PeerEntry::PeerEntry(const SharedHandle<Peer>& peer):
  peer_(peer), downloadSpeed_(peer->calculateDownloadSpeed()),
  // peer must be interested to us and sent block in the last 30 seconds
  regularUnchoker_
  (peer->peerInterested() &&
   peer->getLastDownloadUpdate().difference(global::wallclock) < 30) {}

const SharedHandle<Peer>& BtLeecherStateChoke::PeerEntry::getPeer() const
{
  return peer_;
}

unsigned int BtLeecherStateChoke::PeerEntry::getDownloadSpeed() const
{
  return downloadSpeed_;
}

bool BtLeecherStateChoke::PeerEntry::isRegularUnchoker() const
{
  return regularUnchoker_;
}

void BtLeecherStateChoke::PeerEntry::enableChokingRequired()
{
  peer_->chokingRequired(true);
}

void BtLeecherStateChoke::PeerEntry::disableChokingRequired()
{
  peer_->chokingRequired(false);
}

void BtLeecherStateChoke::PeerEntry::enableOptUnchoking()
{
  peer_->optUnchoking(true);
}

void BtLeecherStateChoke::PeerEntry::disableOptUnchoking()
{
  peer_->optUnchoking(false);
}

bool BtLeecherStateChoke::PeerEntry::isSnubbing() const
{
  return peer_->snubbing();
}

bool BtLeecherStateChoke::PeerEntry::operator<(const PeerEntry& peerEntry) const
{
  return downloadSpeed_ > peerEntry.downloadSpeed_;
}

bool BtLeecherStateChoke::PeerFilter::operator()
  (const PeerEntry& peerEntry) const
{
  return peerEntry.getPeer()->amChoking() == amChoking_ &&
    peerEntry.getPeer()->peerInterested() == peerInterested_;
}

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
    logger_->info
      ("POU: %s", (*peerEntries.begin()).getPeer()->getIPAddress().c_str());
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
    logger_->info("RU: %s, dlspd=%u",
                  (*peerIter).getPeer()->getIPAddress().c_str(),
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
        logger_->info("OU: %s", (*i).getPeer()->getIPAddress().c_str());
        break;
      } else {
        (*i).disableChokingRequired();
        logger_->info("OU: %s", (*i).getPeer()->getIPAddress().c_str());
      }
    }
  }
}

void
BtLeecherStateChoke::executeChoke
(const std::vector<SharedHandle<Peer> >& peerSet)
{
  logger_->info("Leecher state, %d choke round started", round_);
  lastRound_ = global::wallclock;

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
  if(round_ == 0) {
    plannedOptimisticUnchoke(peerEntries);
  }
  regularUnchoke(peerEntries);

  if(++round_ == 3) {
    round_ = 0;
  }
}

const Timer& BtLeecherStateChoke::getLastRound() const
{
  return lastRound_;
}

} // namespace aria2
