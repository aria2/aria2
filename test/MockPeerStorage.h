#ifndef _D_MOCK_PEER_STORAGE_H_
#define _D_MOCK_PEER_STORAGE_H_

#include "PeerStorage.h"

class MockPeerStorage : public PeerStorage {
private:
  TransferStat stat;
  Peers peers;
  Peers activePeers;
public:
  MockPeerStorage() {}
  virtual ~MockPeerStorage() {}

  virtual bool addPeer(const PeerHandle& peer) {
    peers.push_back(peer);
    return true;
  }

  virtual void addPeer(const Peers& peers) {
    copy(peers.begin(), peers.end(), back_inserter(this->peers));
  }

  virtual const Peers& getPeers() {
    return peers;
  }

  virtual PeerHandle getUnusedPeer() {
    return 0;
  }

  virtual bool isPeerAvailable() {
    return false;
  }
  
  virtual Peers getActivePeers() {
    return activePeers;
  }

  virtual TransferStat calculateStat() {
    return stat;
  }

  void setStat(const TransferStat& stat) {
    this->stat = stat;
  }
};

typedef SharedHandle<MockPeerStorage> MockPeerStorageHandle;

#endif // _D_MOCK_PEER_STORAGE_H_
