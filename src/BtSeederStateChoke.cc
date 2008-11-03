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
#include "BtSeederStateChoke.h"

#include <algorithm>

#include "BtContext.h"
#include "Peer.h"
#include "BtMessageDispatcher.h"
#include "BtMessageFactory.h"
#include "BtRequestFactory.h"
#include "BtMessageReceiver.h"
#include "PeerConnection.h"
#include "ExtensionMessageFactory.h"
#include "Logger.h"
#include "LogFactory.h"
#include "a2time.h"

namespace aria2 {

BtSeederStateChoke::BtSeederStateChoke(const SharedHandle<BtContext>& btContext):
  _btContext(btContext),
  _round(0),
  _lastRound(0),
  _logger(LogFactory::getInstance()) {}

BtSeederStateChoke::~BtSeederStateChoke() {}

class RecentUnchoke {
private:
  SharedHandle<BtContext> _btContext;

  const struct timeval _now;
public:
  RecentUnchoke(const SharedHandle<BtContext>& btContext,
		const struct timeval& now):
    _btContext(btContext), _now(now) {}

  bool operator()(Peer* left, Peer* right) const
  {
    // TODO Should peer have the reference to message dispatcher?
//     size_t leftUpload = BT_MESSAGE_DISPATCHER(_btContext, left)->countOutstandingUpload();
//     size_t rightUpload = BT_MESSAGE_DISPATCHER(_btContext, right)->countOutstandingUpload();
//     if(leftUpload && !rightUpload) {
//       return true;
//     } else if(!leftUpload && rightUpload) {
//       return false;
//     }
    const int TIME_FRAME = 20;
    if(!left->getLastAmUnchoking().elapsed(TIME_FRAME) &&
       left->getLastAmUnchoking().isNewer(right->getLastAmUnchoking())) {
      return true;
    } else if(!right->getLastAmUnchoking().elapsed(TIME_FRAME)) {
      return false;
    } else {
      return left->calculateUploadSpeed(_now) > right->calculateUploadSpeed(_now);
    }
  }
};

class NotInterestedPeer {
public:
  bool operator()(const Peer* peer) const
  {
    return !peer->peerInterested();
  }
};

void BtSeederStateChoke::unchoke(std::deque<Peer*>& peers)
{
  int count = (_round == 2) ? 4 : 3;

  struct timeval now;
  gettimeofday(&now, 0);

  std::sort(peers.begin(), peers.end(), RecentUnchoke(_btContext, now));

  std::deque<Peer*>::iterator r = peers.begin();
  for(; r != peers.end() && count; ++r, --count) {
    (*r)->chokingRequired(false);
    _logger->info("RU: %s, ulspd=%u", (*r)->ipaddr.c_str(),
		  (*r)->calculateUploadSpeed(now));
  }
  if(_round == 2 && r != peers.end()) {
    std::random_shuffle(r, peers.end());
    (*r)->optUnchoking(true);
    _logger->info("POU: %s", (*r)->ipaddr.c_str());
  }
}

void
BtSeederStateChoke::executeChoke(const std::deque<SharedHandle<Peer> >& peerSet)
{
  _logger->info("Seeder state, %d choke round started", _round);
  _lastRound.reset();

  std::deque<Peer*> peers;
  std::transform(peerSet.begin(), peerSet.end(), std::back_inserter(peers),
		 std::mem_fun_ref(&SharedHandle<Peer>::get));
	      
  std::for_each(peers.begin(), peers.end(),
		std::bind2nd(std::mem_fun((void (Peer::*)(bool))&Peer::chokingRequired), true));

  peers.erase(std::remove_if(peers.begin(), peers.end(), NotInterestedPeer()),
	      peers.end());

  unchoke(peers);
  
  if(++_round == 3) {
    _round = 0;
  }
}

const Time& BtSeederStateChoke::getLastRound() const
{
  return _lastRound;
}

} // namespace aria2
