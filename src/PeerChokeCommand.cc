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
#include "SleepCommand.h"
#include "Util.h"

PeerChokeCommand::PeerChokeCommand(int cuid, int interval, TorrentDownloadEngine* e):Command(cuid), interval(interval), e(e), rotate(0) {}

PeerChokeCommand::~PeerChokeCommand() {}

void PeerChokeCommand::setAllPeerChoked(Peers& peers) const {
  for(Peers::iterator itr = peers.begin(); itr != peers.end(); itr++) {
    Peer* peer = *itr;
    peer->chokingRequired = true;
  }
}

void PeerChokeCommand::optUnchokingPeer(Peers& peers) const {
  if(peers.empty()) {
    return;
  }
  random_shuffle(peers.begin(), peers.end());
  for(Peers::iterator itr = peers.begin(); itr != peers.end(); itr++) {
    (*itr)->optUnchoking = false;
  }
  Peer* peer = peers.front();
  peer->optUnchoking = true;
  logger->debug("opt, unchoking %s, delta=%d",
		peer->ipaddr.c_str(), peer->getDeltaUpload());
  if(e->torrentMan->isEndGame()) {
    Peers::iterator itr = peers.begin()+1;
    for(; itr != peers.end(); itr++) {
      Peer* peer = *itr;
      if(peer->amInterested && peer->peerInterested) {
	peer->optUnchoking = true;
	logger->debug("opt, unchoking %s, delta=%d",
		      peer->ipaddr.c_str(), peer->getDeltaUpload());
	break;
      }
    }
  }
}

void PeerChokeCommand::setAllPeerResetDelta(Peers& peers) const {
  for(Peers::iterator itr = peers.begin(); itr != peers.end(); itr++) {
    Peer* peer = *itr;
    peer->resetDeltaUpload();
    peer->resetDeltaDownload();
  }
}

class UploadFaster {
public:
  bool operator() (const Peer* left, const Peer* right) const {
    return left->getDeltaUpload() > right->getDeltaUpload();
  }
};

void PeerChokeCommand::orderByUploadRate(Peers& peers) const {
  sort(peers.begin(), peers.end(), UploadFaster());
}

class DownloadFaster {
public:
  bool operator() (const Peer* left, const Peer* right) const {
    return left->getDeltaDownload() > right->getDeltaDownload();
  }
};

void PeerChokeCommand::orderByDownloadRate(Peers& peers) const {
  sort(peers.begin(), peers.end(), UploadFaster());
}

bool PeerChokeCommand::execute() {
  Peers peers = e->torrentMan->getActivePeers();
  setAllPeerChoked(peers);
  if(e->torrentMan->downloadComplete()) {
    orderByDownloadRate(peers);
  } else {
    orderByUploadRate(peers);
  }
  int unchokingCount = peers.size() >= 4 ? 4 : peers.size();
  for(Peers::iterator itr = peers.begin(); unchokingCount > 0 && itr != peers.end(); ) {
    Peer* peer = *itr;
    if(peer->peerInterested) {
      peer->chokingRequired = false;
      peer->optUnchoking = false;
      itr = peers.erase(itr);
      unchokingCount--;
      logger->debug("cat01, unchoking %s, delta=%d", peer->ipaddr.c_str(), peer->getDeltaUpload());
    } else {
      itr++;
    }
  }
  for(Peers::iterator itr = peers.begin(); itr != peers.end(); ) {
    Peer* peer = *itr;
    if(!peer->peerInterested) {
      peer->chokingRequired = false;
      peer->optUnchoking = false;
      itr = peers.erase(itr);
      logger->debug("cat02, unchoking %s, delta=%d", peer->ipaddr.c_str(), peer->getDeltaUpload());
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
  setAllPeerResetDelta(e->torrentMan->getActivePeers());

  SleepCommand* command = new SleepCommand(cuid, e, this, interval);
  e->commands.push(command);

  return false;
}
