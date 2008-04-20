#ifndef _D_MOCK_PEER_STORAGE_H_
#define _D_MOCK_PEER_STORAGE_H_

#include "PeerStorage.h"
#include "Peer.h"
#include <algorithm>

namespace aria2 {

class MockPeerStorage : public PeerStorage {
private:
  TransferStat stat;
  std::deque<SharedHandle<Peer> > peers;
  std::deque<SharedHandle<Peer> > activePeers;
public:
  MockPeerStorage() {}
  virtual ~MockPeerStorage() {}

  virtual bool addPeer(const SharedHandle<Peer>& peer) {
    peers.push_back(peer);
    return true;
  }

  virtual void addPeer(const std::deque<SharedHandle<Peer> >& peers) {
    std::copy(peers.begin(), peers.end(), back_inserter(this->peers));
  }

  virtual const std::deque<SharedHandle<Peer> >& getPeers() {
    return peers;
  }

  virtual SharedHandle<Peer> getUnusedPeer() {
    return SharedHandle<Peer>();
  }

  virtual bool isPeerAvailable() {
    return false;
  }
  
  void setActivePeers(const std::deque<SharedHandle<Peer> >& activePeers)
  {
    this->activePeers = activePeers;
  }

  virtual std::deque<SharedHandle<Peer> > getActivePeers() {
    return activePeers;
  }

  virtual TransferStat calculateStat() {
    return stat;
  }

  void setStat(const TransferStat& stat) {
    this->stat = stat;
  }

  virtual void returnPeer(const SharedHandle<Peer>& peer)
  {
  }

  virtual bool chokeRoundIntervalElapsed()
  {
    return false;
  }

  virtual void executeChoke() {}
};

#endif // _D_MOCK_PEER_STORAGE_H_

} // namespace aria2
