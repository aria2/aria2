#ifndef D_MOCK_PEER_STORAGE_H
#define D_MOCK_PEER_STORAGE_H

#include "PeerStorage.h"

#include <algorithm>

#include "Peer.h"

namespace aria2 {

class MockPeerStorage : public PeerStorage {
private:
  std::deque<std::shared_ptr<Peer> > unusedPeers;
  PeerSet usedPeers;
  std::deque<std::shared_ptr<Peer> > droppedPeers;
  std::vector<std::shared_ptr<Peer> > activePeers;
  int numChokeExecuted_;
public:
  MockPeerStorage():numChokeExecuted_(0) {}
  virtual ~MockPeerStorage() {}

  virtual bool addPeer(const std::shared_ptr<Peer>& peer)
  {
    unusedPeers.push_back(peer);
    return true;
  }

  virtual void addPeer(const std::vector<std::shared_ptr<Peer> >& peers) {
    unusedPeers.insert(unusedPeers.end(), peers.begin(), peers.end());
  }

  const std::deque<std::shared_ptr<Peer> >& getUnusedPeers()
  {
    return unusedPeers;
  }

  virtual size_t countAllPeer() const
  {
    return unusedPeers.size() + usedPeers.size();
  }

  virtual const std::deque<std::shared_ptr<Peer> >& getDroppedPeers() {
    return droppedPeers;
  }

  void addDroppedPeer(const std::shared_ptr<Peer>& peer) {
    droppedPeers.push_back(peer);
  }

  virtual bool isPeerAvailable() {
    return false;
  }

  void setActivePeers(const std::vector<std::shared_ptr<Peer> >& activePeers)
  {
    this->activePeers = activePeers;
  }

  void getActivePeers(std::vector<std::shared_ptr<Peer> >& peers) {
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

  virtual std::shared_ptr<Peer> checkoutPeer(cuid_t cuid)
  {
    return nullptr;
  }

  virtual void returnPeer(const std::shared_ptr<Peer>& peer)
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
