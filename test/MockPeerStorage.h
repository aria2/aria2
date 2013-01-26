#ifndef D_MOCK_PEER_STORAGE_H
#define D_MOCK_PEER_STORAGE_H

#include "PeerStorage.h"

#include <algorithm>

#include "Peer.h"

namespace aria2 {

class MockPeerStorage : public PeerStorage {
private:
  std::deque<SharedHandle<Peer> > unusedPeers;
  PeerSet usedPeers;
  std::deque<SharedHandle<Peer> > droppedPeers;
  std::vector<SharedHandle<Peer> > activePeers;
  int numChokeExecuted_;
public:
  MockPeerStorage():numChokeExecuted_(0) {}
  virtual ~MockPeerStorage() {}

  virtual bool addPeer(const SharedHandle<Peer>& peer)
  {
    unusedPeers.push_back(peer);
    return true;
  }

  virtual void addPeer(const std::vector<SharedHandle<Peer> >& peers) {
    unusedPeers.insert(unusedPeers.end(), peers.begin(), peers.end());
  }

  const std::deque<SharedHandle<Peer> >& getUnusedPeers()
  {
    return unusedPeers;
  }

  virtual size_t countAllPeer() const
  {
    return unusedPeers.size() + usedPeers.size();
  }

  virtual const std::deque<SharedHandle<Peer> >& getDroppedPeers() {
    return droppedPeers;
  }

  void addDroppedPeer(const SharedHandle<Peer>& peer) {
    droppedPeers.push_back(peer);
  }

  virtual bool isPeerAvailable() {
    return false;
  }

  void setActivePeers(const std::vector<SharedHandle<Peer> >& activePeers)
  {
    this->activePeers = activePeers;
  }

  void getActivePeers(std::vector<SharedHandle<Peer> >& peers) {
    peers.insert(peers.end(), activePeers.begin(), activePeers.end());
  }

  virtual const PeerSet& getUsedPeers()
  {
    return usedPeers;
  }

  virtual bool isBadPeer(const std::string& ipaddr)
  {
    return false;
  }

  virtual void addBadPeer(const std::string& ipaddr)
  {
  }

  virtual SharedHandle<Peer> checkoutPeer(cuid_t cuid)
  {
    return SharedHandle<Peer>();
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

  int getNumChokeExecuted() const
  {
    return numChokeExecuted_;
  }
};

#endif // D_MOCK_PEER_STORAGE_H

} // namespace aria2
