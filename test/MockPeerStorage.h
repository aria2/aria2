#ifndef D_MOCK_PEER_STORAGE_H
#define D_MOCK_PEER_STORAGE_H

#include "PeerStorage.h"

#include <algorithm>

#include "Peer.h"

namespace aria2 {

class MockPeerStorage : public PeerStorage {
private:
  TransferStat stat;
  std::deque<SharedHandle<Peer> > peers;
  std::deque<SharedHandle<Peer> > droppedPeers;
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

  virtual size_t countPeer() const
  {
    return peers.size();
  }

  virtual const std::deque<SharedHandle<Peer> >& getDroppedPeers() {
    return droppedPeers;
  }

  void addDroppedPeer(const SharedHandle<Peer>& peer) {
    droppedPeers.push_back(peer);
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

  virtual bool isBadPeer(const std::string& ipaddr)
  {
    return false;
  }

  virtual void addBadPeer(const std::string& ipaddr)
  {
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

#endif // D_MOCK_PEER_STORAGE_H

} // namespace aria2
