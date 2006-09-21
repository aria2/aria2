/* <!-- copyright */
/*
 * aria2 - a simple utility for downloading files faster
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
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */
/* copyright --> */
#include "PeerChokeCommand.h"
#include "Util.h"

PeerChokeCommand::PeerChokeCommand(int cuid, TorrentDownloadEngine* e, int interval):Command(cuid), interval(interval), e(e), rotate(0) {}

PeerChokeCommand::~PeerChokeCommand() {}

class ChokePeer {
public:
  ChokePeer() {}
  void operator()(PeerHandle& peer) {
    peer->chokingRequired = true;
  }
};

void PeerChokeCommand::optUnchokingPeer(Peers& peers) const {
  if(peers.empty()) {
    return;
  }
  random_shuffle(peers.begin(), peers.end());
  int optUnchokCount = 1;
  for(Peers::iterator itr = peers.begin(); itr != peers.end(); itr++) {
    Peers::value_type peer = *itr;
    if(optUnchokCount > 0 && !peer->snubbing) {
      optUnchokCount--;
      peer->optUnchoking = true;
      logger->debug("opt, unchoking %s, download speed=%d",
		    peer->ipaddr.c_str(), peer->calculateDownloadSpeed());
    } else {
      peer->optUnchoking = false;
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
  sort(peers.begin(), peers.end(), UploadFaster());
}

class DownloadFaster {
public:
  bool operator() (const PeerHandle& left, const PeerHandle& right) const {
    return left->calculateDownloadSpeed() > right->calculateDownloadSpeed();
  }
};

void PeerChokeCommand::orderByDownloadRate(Peers& peers) const {
  sort(peers.begin(), peers.end(), DownloadFaster());
}

bool PeerChokeCommand::execute() {
  if(e->torrentMan->isHalt()) {
    return true;
  }
  if(checkPoint.elapsed(interval)) {
    checkPoint.reset();
    Peers peers = e->torrentMan->getActivePeers();
    for_each(peers.begin(), peers.end(), ChokePeer());
    if(e->torrentMan->downloadComplete()) {
      orderByUploadRate(peers);
    } else {
      orderByDownloadRate(peers);
    }
    int unchokingCount = 4;//peers.size() >= 4 ? 4 : peers.size();
    for(Peers::iterator itr = peers.begin(); itr != peers.end() && unchokingCount > 0; ) {
      PeerHandle peer = *itr;
      if(peer->peerInterested && !peer->snubbing) {
	unchokingCount--;
	peer->chokingRequired = false;
	peer->optUnchoking = false;
	itr = peers.erase(itr);
	if(e->torrentMan->downloadComplete()) {
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
      if(!peer->peerInterested && !peer->snubbing) {
	peer->chokingRequired = false;
	peer->optUnchoking = false;
	itr = peers.erase(itr);
	if(e->torrentMan->downloadComplete()) {
	  logger->debug("cat01, unchoking %s, upload speed=%d",
			peer->ipaddr.c_str(),
			peer->calculateUploadSpeed());
	} else {
	  logger->debug("cat01, unchoking %s, download speed=%d",
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
