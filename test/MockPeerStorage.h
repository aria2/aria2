#ifndef _D_MOCK_PEER_STORAGE_H_
#define _D_MOCK_PEER_STORAGE_H_

#include "PeerStorage.h"

#include <algorithm>

#include "Peer.h"

namespace aria2 {

class MockPeerStorage : public PeerStorage {
private:
  TransferStat stat;
  std::deque<SharedHandle<Peer> > peers;
  std::vector<SharedHandle<Peer> > activePeers;
  int numChokeExecuted_;
public:
  MockPeerStorage():numChokeExecuted_(0) {}
  virtual ~MockPeerStorage() {}

  virtual bool addPeer(const SharedHandle<Peer>& peer) {
    peers.push_back(peer);
    return true;
  }

  virtual void addPeer(const std::vector<SharedHandle<Peer> >& peers) {
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
  
  void setActivePeers(const std::vector<SharedHandle<Peer> >& activePeers)
  {
    this->activePeers = activePeers;
  }

  virtual void getActivePeers(std::vector<SharedHandle<Peer> >& peers) {
    peers.insert(peers.end(), activePeers.begin(), activePeers.end());
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

  virtual void executeChoke()
  {
    ++numChokeExecuted_;
  }

  virtual void updateTransferStatFor(const SharedHandle<Peer>& peer) {}

  virtual TransferStat getTransferStatFor(const SharedHandle<Peer>& peer)
  {
    return TransferStat();
  }

  int getNumChokeExecuted() const
  {
    return numChokeExecuted_;
  }
};

#endif // _D_MOCK_PEER_STORAGE_H_

} // namespace aria2
