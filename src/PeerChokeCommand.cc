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
#include "PeerChokeCommand.h"
#include "Util.h"
#include "Peer.h"
#include "DownloadEngine.h"
#include "BtContext.h"
#include "BtRuntime.h"
#include "PieceStorage.h"
#include "PeerStorage.h"
#include "Logger.h"
#include <algorithm>

namespace aria2 {

PeerChokeCommand::PeerChokeCommand(int32_t cuid,
				   RequestGroup* requestGroup,
				   DownloadEngine* e,
				   const BtContextHandle& btContext,
				   int32_t interval):
  Command(cuid),
  BtContextAwareCommand(btContext),
  RequestGroupAware(requestGroup),
  interval(interval),
  e(e),
  rotate(0)
{}

PeerChokeCommand::~PeerChokeCommand() {}

class ChokePeer {
public:
  ChokePeer() {}
  void operator()(PeerHandle& peer) {
    peer->chokingRequired(true);
  }
};

void PeerChokeCommand::optUnchokingPeer(Peers& peers) const {
  if(peers.empty()) {
    return;
  }
  std::random_shuffle(peers.begin(), peers.end());
  int32_t optUnchokCount = 1;
  for(Peers::iterator itr = peers.begin(); itr != peers.end(); itr++) {
    Peers::value_type peer = *itr;
    if(optUnchokCount > 0 && !peer->snubbing()) {
      optUnchokCount--;
      peer->optUnchoking(true);
      logger->debug("opt, unchoking %s, download speed=%d",
		    peer->ipaddr.c_str(), peer->calculateDownloadSpeed());
    } else {
      peer->optUnchoking(false);
    }
  }
}

class UploadFaster {
public:
  bool operator() (const PeerHandle& left, const PeerHandle& right) const {
    return left->calculateUploadSpeed() > right->calculateUploadSpeed();
  }
};

void PeerChokeCommand::orderByUploadRate(Peers& peers) const {
  std::sort(peers.begin(), peers.end(), UploadFaster());
}

class DownloadFaster {
public:
  bool operator() (const PeerHandle& left, const PeerHandle& right) const {
    return left->calculateDownloadSpeed() > right->calculateDownloadSpeed();
  }
};

void PeerChokeCommand::orderByDownloadRate(Peers& peers) const {
  std::sort(peers.begin(), peers.end(), DownloadFaster());
}

bool PeerChokeCommand::execute() {
  if(btRuntime->isHalt()) {
    return true;
  }
  if(checkPoint.elapsed(interval)) {
    checkPoint.reset();
    Peers peers = peerStorage->getActivePeers();
    std::for_each(peers.begin(), peers.end(), ChokePeer());
    if(pieceStorage->downloadFinished()) {
      orderByUploadRate(peers);
    } else {
      orderByDownloadRate(peers);
    }
    int32_t unchokingCount = 4;//peers.size() >= 4 ? 4 : peers.size();
    for(Peers::iterator itr = peers.begin(); itr != peers.end() && unchokingCount > 0; ) {
      PeerHandle peer = *itr;
      if(peer->peerInterested() && !peer->snubbing()) {
	unchokingCount--;
	peer->chokingRequired(false);
	peer->optUnchoking(false);
	itr = peers.erase(itr);
	if(pieceStorage->downloadFinished()) {
	  logger->debug("cat01, unchoking %s, upload speed=%d",
			peer->ipaddr.c_str(),
			peer->calculateUploadSpeed());
	} else {
	  logger->debug("cat01, unchoking %s, download speed=%d",
			peer->ipaddr.c_str(),
			peer->calculateDownloadSpeed());
	}
      } else {
	itr++;
      }
    }
    for(Peers::iterator itr = peers.begin(); itr != peers.end(); ) {
      PeerHandle peer = *itr;
      if(!peer->peerInterested() && !peer->snubbing()) {
	peer->chokingRequired(false);
	peer->optUnchoking(false);
	itr = peers.erase(itr);
	if(pieceStorage->downloadFinished()) {
	  logger->debug("cat02, unchoking %s, upload speed=%d",
			peer->ipaddr.c_str(),
			peer->calculateUploadSpeed());
	} else {
	  logger->debug("cat02, unchoking %s, download speed=%d",
			peer->ipaddr.c_str(),
			peer->calculateDownloadSpeed());
	}
	break;
      } else {
	itr++;
      }
    }
    if(rotate%3 == 0) {
      optUnchokingPeer(peers);
      rotate = 0;
    }
    rotate++;
  }
  e->commands.push_back(this);
  return false;
}

} // namespace aria2
