#ifndef D_MOCK_PEER_STORAGE_H
#  define D_MOCK_PEER_STORAGE_H

#  include "PeerStorage.h"

#  include <algorithm>

#  include "Peer.h"

namespace aria2 {

class MockPeerStorage : public PeerStorage {
private:
  std::deque<std::shared_ptr<Peer>> unusedPeers;
  PeerSet usedPeers;
  std::deque<std::shared_ptr<Peer>> droppedPeers;
  std::vector<std::shared_ptr<Peer>> activePeers;
  int numChokeExecuted_;

public:
  MockPeerStorage() : numChokeExecuted_(0) {}
  virtual ~MockPeerStorage() {}

  virtual bool addPeer(const std::shared_ptr<Peer>& peer) CXX11_OVERRIDE
  {
    unusedPeers.push_back(peer);
    return true;
  }

  virtual void
  addPeer(const std::vector<std::shared_ptr<Peer>>& peers) CXX11_OVERRIDE
  {
    unusedPeers.insert(unusedPeers.end(), peers.begin(), peers.end());
  }

  const std::deque<std::shared_ptr<Peer>>& getUnusedPeers()
  {
    return unusedPeers;
  }

  virtual std::shared_ptr<Peer>
  addAndCheckoutPeer(const std::shared_ptr<Peer>& peer,
                     cuid_t cuid) CXX11_OVERRIDE
  {
    unusedPeers.push_back(peer);
    return nullptr;
  }

  virtual size_t countAllPeer() const CXX11_OVERRIDE
  {
    return unusedPeers.size() + usedPeers.size();
  }

  virtual const std::deque<std::shared_ptr<Peer>>&
  getDroppedPeers() CXX11_OVERRIDE
  {
    return droppedPeers;
  }

  void addDroppedPeer(const std::shared_ptr<Peer>& peer)
  {
    droppedPeers.push_back(peer);
  }

  virtual bool isPeerAvailable() CXX11_OVERRIDE { return false; }

  void setActivePeers(const std::vector<std::shared_ptr<Peer>>& activePeers)
  {
    this->activePeers = activePeers;
  }

  void getActivePeers(std::vector<std::shared_ptr<Peer>>& peers)
  {
    peers.insert(peers.end(), activePeers.begin(), activePeers.end());
  }

  virtual const PeerSet& getUsedPeers() CXX11_OVERRIDE { return usedPeers; }

  virtual bool isBadPeer(const std::string& ipaddr) CXX11_OVERRIDE
  {
    return false;
  }

  virtual void addBadPeer(const std::string& ipaddr) CXX11_OVERRIDE {}

  virtual std::shared_ptr<Peer> checkoutPeer(cuid_t cuid) CXX11_OVERRIDE
  {
    return nullptr;
  }

  virtual void returnPeer(const std::shared_ptr<Peer>& peer) CXX11_OVERRIDE {}

  virtual bool chokeRoundIntervalElapsed() CXX11_OVERRIDE { return false; }

  virtual void executeChoke() CXX11_OVERRIDE { ++numChokeExecuted_; }

  int getNumChokeExecuted() const { return numChokeExecuted_; }
};

#endif // D_MOCK_PEER_STORAGE_H

} // namespace aria2
