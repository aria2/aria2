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
#include "fmt.h"

namespace aria2 {

BtLeecherStateChoke::BtLeecherStateChoke()
    : round_(0), lastRound_(Timer::zero())
{
}

BtLeecherStateChoke::~BtLeecherStateChoke() = default;

BtLeecherStateChoke::PeerEntry::PeerEntry(const std::shared_ptr<Peer>& peer)
    : peer_(peer),
      downloadSpeed_(peer->calculateDownloadSpeed()),
      // peer must be interested to us and sent block in the last 30 seconds
      regularUnchoker_(
          peer->peerInterested() &&
          peer->getLastDownloadUpdate().difference(global::wallclock()) < 30_s)
{
}

BtLeecherStateChoke::PeerEntry::PeerEntry(const PeerEntry& c) = default;

void BtLeecherStateChoke::PeerEntry::swap(PeerEntry& c)
{
  using std::swap;
  swap(peer_, c.peer_);
  swap(downloadSpeed_, c.downloadSpeed_);
  swap(regularUnchoker_, c.regularUnchoker_);
}

BtLeecherStateChoke::PeerEntry& BtLeecherStateChoke::PeerEntry::
operator=(const PeerEntry& c)
{
  if (this != &c) {
    peer_ = c.peer_;
    downloadSpeed_ = c.downloadSpeed_;
    regularUnchoker_ = c.regularUnchoker_;
  }
  return *this;
}

BtLeecherStateChoke::PeerEntry::~PeerEntry() = default;

const std::shared_ptr<Peer>& BtLeecherStateChoke::PeerEntry::getPeer() const
{
  return peer_;
}

int BtLeecherStateChoke::PeerEntry::getDownloadSpeed() const
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

bool BtLeecherStateChoke::PeerEntry::operator<(const PeerEntry& peerEntry) const
{
  return downloadSpeed_ > peerEntry.downloadSpeed_;
}

void swap(BtLeecherStateChoke::PeerEntry& a, BtLeecherStateChoke::PeerEntry& b)
{
  a.swap(b);
}

bool BtLeecherStateChoke::PeerFilter::
operator()(const PeerEntry& peerEntry) const
{
  return peerEntry.getPeer()->amChoking() == amChoking_ &&
         peerEntry.getPeer()->peerInterested() == peerInterested_;
}

void BtLeecherStateChoke::plannedOptimisticUnchoke(
    std::vector<PeerEntry>& peerEntries)
{
  std::for_each(std::begin(peerEntries), std::end(peerEntries),
                std::mem_fn(&PeerEntry::disableOptUnchoking));

  auto i = std::partition(std::begin(peerEntries), std::end(peerEntries),
                          PeerFilter(true, true));
  if (i != std::begin(peerEntries)) {
    std::shuffle(std::begin(peerEntries), i, *SimpleRandomizer::getInstance());

    auto& ent = *std::begin(peerEntries);
    auto& peer = ent.getPeer();

    ent.enableOptUnchoking();

    A2_LOG_INFO(
        fmt("POU: %s:%u", peer->getIPAddress().c_str(), peer->getPort()));
  }
}

void BtLeecherStateChoke::regularUnchoke(std::vector<PeerEntry>& peerEntries)
{
  auto rest = std::partition(std::begin(peerEntries), std::end(peerEntries),
                             std::mem_fn(&PeerEntry::isRegularUnchoker));

  std::sort(std::begin(peerEntries), rest);
  std::shuffle(rest, std::end(peerEntries), *SimpleRandomizer::getInstance());

  // the number of regular unchokers
  int count = 3;

  bool fastOptUnchoker = false;
  auto peerIter = std::begin(peerEntries);
  for (; peerIter != std::end(peerEntries) && count; ++peerIter, --count) {
    auto& peer = peerIter->getPeer();

    if (!peer->peerInterested()) {
      continue;
    }

    peerIter->disableChokingRequired();

    A2_LOG_INFO(fmt("RU: %s:%u, dlspd=%d", peer->getIPAddress().c_str(),
                    peer->getPort(), (*peerIter).getDownloadSpeed()));

    if (peer->optUnchoking()) {
      fastOptUnchoker = true;
      peerIter->disableOptUnchoking();
    }
  }
  if (fastOptUnchoker) {
    for (auto& p : peerEntries) {
      if (!p.getPeer()->peerInterested()) {
        continue;
      }

      p.enableOptUnchoking();

      auto& peer = p.getPeer();

      A2_LOG_INFO(
          fmt("OU: %s:%u", peer->getIPAddress().c_str(), peer->getPort()));

      break;
    }
  }
}

void BtLeecherStateChoke::executeChoke(const PeerSet& peerSet)
{
  A2_LOG_INFO(fmt("Leecher state, %d choke round started", round_));
  lastRound_ = global::wallclock();

  std::vector<PeerEntry> peerEntries;
  for (const auto& p : peerSet) {
    if (!p->isActive()) {
      continue;
    }

    p->chokingRequired(true);

    if (p->snubbing()) {
      p->optUnchoking(false);
      continue;
    }

    peerEntries.push_back(PeerEntry(p));
  }

  // planned optimistic unchoke
  if (round_ == 0) {
    plannedOptimisticUnchoke(peerEntries);
  }
  regularUnchoke(peerEntries);

  if (++round_ == 3) {
    round_ = 0;
  }
}

const Timer& BtLeecherStateChoke::getLastRound() const { return lastRound_; }

} // namespace aria2
