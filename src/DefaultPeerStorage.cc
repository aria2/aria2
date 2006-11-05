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
#include "DefaultPeerStorage.h"
#include "LogFactory.h"
#include "BtRegistry.h"

extern PeerHandle nullPeer;

DefaultPeerStorage::DefaultPeerStorage(BtContextHandle btContext,
				       const Option* option):
  btContext(btContext),
  option(option),
  maxPeerListSize(MAX_PEER_LIST_SIZE),
  peerEntryIdCounter(0),
  btRuntime(BT_RUNTIME(btContext))
{
  logger = LogFactory::getInstance();
}

DefaultPeerStorage::~DefaultPeerStorage() {}

bool DefaultPeerStorage::addPeer(const PeerHandle& peer) {
  Peers::iterator itr = find(peers.begin(), peers.end(), peer);
  if(itr == peers.end()) {
    if(peers.size() >= (size_t)maxPeerListSize) {
      deleteUnusedPeer(peers.size()-maxPeerListSize+1);
    }
    ++peerEntryIdCounter;
    peer->entryId = peerEntryIdCounter;
    peers.push_back(peer);
    return true;
  } else {
    const PeerHandle& peer = *itr;
    if(peer->error >= MAX_PEER_ERROR || peer->cuid != 0) {
      return false;
    } else {
      *itr = peer;
      return true;
    }      
  }
}

void DefaultPeerStorage::addPeer(const Peers& peers) {
  for(Peers::const_iterator itr = peers.begin();
      itr != peers.end(); itr++) {
    const PeerHandle& peer = *itr;
    if(addPeer(peer)) {
      logger->debug("Adding peer %s:%d",
		    peer->ipaddr.c_str(), peer->port);
    }
  }  
}

const Peers& DefaultPeerStorage::getPeers() {
  return peers;
}

class FindFinePeer {
public:
  bool operator()(const PeerHandle& peer) const {
    return peer->cuid == 0 && peer->error < MAX_PEER_ERROR;
  }
};

PeerHandle DefaultPeerStorage::getUnusedPeer() {
  Peers::const_iterator itr = find_if(peers.begin(), peers.end(),
				      FindFinePeer());
  if(itr == peers.end()) {
    return nullPeer;
  } else {
    return *itr;
  }
}

class FindPeer {
private:
  string ipaddr;
  int port;
public:
  FindPeer(const string& ipaddr, int port):ipaddr(ipaddr), port(port) {}

  bool operator()(const PeerHandle& peer) const {
    return ipaddr == peer->ipaddr && port == peer->port;
  }
};

PeerHandle DefaultPeerStorage::getPeer(const string& ipaddr,
				       int port) const {
  Peers::const_iterator itr = find_if(peers.begin(), peers.end(),
				      FindPeer(ipaddr, port));
  if(itr == peers.end()) {
    return nullPeer;
  } else {
    return *itr;
  }
}

int DefaultPeerStorage::countPeer() const {
  return peers.size();
}

bool DefaultPeerStorage::isPeerAvailable() {
  return getUnusedPeer() != nullPeer;
}

Peers DefaultPeerStorage::getActivePeers() {
  Peers activePeers;
  for(Peers::iterator itr = peers.begin(); itr != peers.end(); itr++) {
    PeerHandle& peer = *itr;
    if(peer->isActive()) {
      activePeers.push_back(peer);
    }
  }
  return activePeers;
}

TransferStat DefaultPeerStorage::calculateStat() {
  TransferStat stat;
  Peers activePeers = getActivePeers();
  for(Peers::iterator itr = activePeers.begin();
      itr != activePeers.end(); itr++) {
    PeerHandle& peer = *itr;
    stat.downloadSpeed += peer->calculateDownloadSpeed();
    stat.uploadSpeed += peer->calculateUploadSpeed();
    stat.sessionDownloadLength += peer->getSessionDownloadLength();
    stat.sessionUploadLength += peer->getSessionUploadLength();
  }
  return stat;
}

void DefaultPeerStorage::deleteUnusedPeer(int delSize) {
  for(Peers::iterator itr = peers.begin();
      itr != peers.end() && delSize > 0;) {
    const PeerHandle& p = *itr;
    if(p->cuid == 0) {
      itr = peers.erase(itr);
      delSize--;
    } else {
      itr++;
    }
  }
}
