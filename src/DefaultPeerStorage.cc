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
#include "message.h"
#include "a2time.h"

DefaultPeerStorage::DefaultPeerStorage(BtContextHandle btContext,
				       const Option* option):
  btContext(btContext),
  option(option),
  maxPeerListSize(MAX_PEER_LIST_SIZE),
  btRuntime(BT_RUNTIME(btContext)),
  removedPeerSessionDownloadLength(0),
  removedPeerSessionUploadLength(0)
{
  logger = LogFactory::getInstance();
}

DefaultPeerStorage::~DefaultPeerStorage() {}

bool DefaultPeerStorage::isPeerAlreadyAdded(const PeerHandle& peer)
{
  return find(peers.begin(), peers.end(), peer) != peers.end() ||
    find(incomingPeers.begin(), incomingPeers.end(), peer) != incomingPeers.end();
}

bool DefaultPeerStorage::addPeer(const PeerHandle& peer) {
  if(isPeerAlreadyAdded(peer)) {
    logger->debug("Adding %s:%u is rejected because it is already in PeerStorage.", peer->ipaddr.c_str(), peer->port);
    return false;
  } else {
    if(peers.size() >= (size_t)maxPeerListSize) {
      deleteUnusedPeer(peers.size()-maxPeerListSize+1);
    }
    peers.push_front(peer);
    return true;
  }
}

bool DefaultPeerStorage::addIncomingPeer(const PeerHandle& peer)
{
  if(isPeerAlreadyAdded(peer)) {
    logger->debug("Adding %s:%u is rejected because it is already in PeerStorage.", peer->ipaddr.c_str(), peer->port);
    return false;
  } else {
    incomingPeers.push_back(peer);
    return true;
  }
}

void DefaultPeerStorage::addPeer(const Peers& peers) {
  for(Peers::const_iterator itr = peers.begin();
      itr != peers.end(); itr++) {
    const PeerHandle& peer = *itr;
    if(addPeer(peer)) {
      logger->debug(MSG_ADDING_PEER,
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
    return peer->cuid == 0 && peer->isGood();
  }
};

PeerHandle DefaultPeerStorage::getUnusedPeer() {
  Peers::const_iterator itr = find_if(peers.begin(), peers.end(),
				      FindFinePeer());
  if(itr == peers.end()) {
    return 0;
  } else {
    return *itr;
  }
}

class FindPeer {
private:
  string ipaddr;
  int32_t port;
public:
  FindPeer(const string& ipaddr, int32_t port):ipaddr(ipaddr), port(port) {}

  bool operator()(const PeerHandle& peer) const {
    return ipaddr == peer->ipaddr && port == peer->port;
  }
};

PeerHandle DefaultPeerStorage::getPeer(const string& ipaddr,
				       int32_t port) const {
  Peers::const_iterator itr = find_if(peers.begin(), peers.end(),
				      FindPeer(ipaddr, port));
  if(itr == peers.end()) {
    return 0;
  } else {
    return *itr;
  }
}

int32_t DefaultPeerStorage::countPeer() const {
  return peers.size();
}

bool DefaultPeerStorage::isPeerAvailable() {
  return !getUnusedPeer().isNull();
}

class CollectActivePeer {
private:
  Peers _activePeers;
public:
  void operator()(const PeerHandle& peer)
  {
    if(peer->isActive()) {
      _activePeers.push_back(peer);
    }
  }

  const Peers& getActivePeers() { return _activePeers; }
};

Peers DefaultPeerStorage::getActivePeers() {
  CollectActivePeer funcObj;
  funcObj = for_each(peers.begin(), peers.end(), funcObj);
  funcObj = for_each(incomingPeers.begin(), incomingPeers.end(), funcObj);
  return funcObj.getActivePeers();
}

class CalculateStat {
private:
  TransferStat _stat;
  struct timeval _now;
public:
  CalculateStat()
  {
    gettimeofday(&_now, 0);
  }

  void operator()(const PeerHandle& peer)
  {
    if(peer->isActive()) {
      _stat.downloadSpeed += peer->calculateDownloadSpeed(_now);
      _stat.uploadSpeed += peer->calculateUploadSpeed(_now);
    }
    _stat.sessionDownloadLength += peer->getSessionDownloadLength();
    _stat.sessionUploadLength += peer->getSessionUploadLength();    
  }

  const TransferStat& getTransferStat() { return _stat; }
};

TransferStat DefaultPeerStorage::calculateStat() {
  CalculateStat calStat;
  calStat = for_each(peers.begin(), peers.end(), calStat);
  calStat = for_each(incomingPeers.begin(), incomingPeers.end(), calStat);

  TransferStat stat = calStat.getTransferStat();
  stat.sessionDownloadLength += removedPeerSessionDownloadLength;
  stat.sessionUploadLength += removedPeerSessionUploadLength;
  stat.setAllTimeUploadLength(btRuntime->getUploadLengthAtStartup()+
			      stat.getSessionUploadLength());
  return stat;
}

void DefaultPeerStorage::deleteUnusedPeer(int32_t delSize) {
  Peers temp;
  for(Peers::reverse_iterator itr = peers.rbegin();
      itr != peers.rend(); ++itr) {
    const PeerHandle& p = *itr;
    if(p->cuid == 0 && delSize > 0) {
      // Update removedPeerSession******Length
      onErasingPeer(p);
      delSize--;
    } else {
      temp.push_front(p);
    }
  }
  peers = temp;
}

void DefaultPeerStorage::onErasingPeer(const PeerHandle& peer)
{
  removedPeerSessionDownloadLength += peer->getSessionDownloadLength();
  removedPeerSessionUploadLength += peer->getSessionUploadLength();
}

void DefaultPeerStorage::returnPeer(const PeerHandle& peer)
{
  Peers::iterator itr = find(peers.begin(), peers.end(), peer);
  if(itr == peers.end()) {
    itr = find(incomingPeers.begin(), incomingPeers.end(), peer);
    if(itr == peers.end()) {
      // do nothing
    } else {
      // erase incoming peer because we cannot connect to it with port number
      // (*itr)->port. It is not the listening port.
      onErasingPeer(*itr);
      incomingPeers.erase(itr);
    }
  } else {
    peer->startBadCondition();
    peer->resetStatus();
    peers.erase(itr);
    peers.push_back(peer);
  }
}
