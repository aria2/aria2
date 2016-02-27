#ifndef D_MOCK_BT_ANNOUNCE_H
#define D_MOCK_BT_ANNOUNCE_H

#include "BtAnnounce.h"

namespace aria2 {

class MockBtAnnounce : public BtAnnounce {
private:
  bool announceReady;
  std::string announceUrl;
  std::string peerId;

public:
  MockBtAnnounce() {}
  virtual ~MockBtAnnounce() {}

  virtual bool isAnnounceReady() CXX11_OVERRIDE { return announceReady; }

  void setAnnounceReady(bool flag) { this->announceReady = flag; }

  virtual std::string getAnnounceUrl() CXX11_OVERRIDE { return announceUrl; }

  virtual std::shared_ptr<UDPTrackerRequest>
  createUDPTrackerRequest(const std::string& remoteAddr, uint16_t remotePort,
                          uint16_t localPort) CXX11_OVERRIDE
  {
    return nullptr;
  }

  void setAnnounceUrl(const std::string& url) { this->announceUrl = url; }

  virtual void announceStart() CXX11_OVERRIDE {}

  virtual void announceSuccess() CXX11_OVERRIDE {}

  virtual void announceFailure() CXX11_OVERRIDE {}

  virtual bool isAllAnnounceFailed() CXX11_OVERRIDE { return false; }

  virtual void resetAnnounce() CXX11_OVERRIDE {}

  virtual void
  processAnnounceResponse(const unsigned char* trackerResponse,
                          size_t trackerResponseLength) CXX11_OVERRIDE
  {
  }

  virtual void processUDPTrackerResponse(
      const std::shared_ptr<UDPTrackerRequest>& req) CXX11_OVERRIDE
  {
  }

  virtual bool noMoreAnnounce() CXX11_OVERRIDE { return false; }

  virtual void shuffleAnnounce() CXX11_OVERRIDE {}

  virtual void overrideMinInterval(std::chrono::seconds interval) CXX11_OVERRIDE
  {
  }

  virtual void setTcpPort(uint16_t port) CXX11_OVERRIDE {}

  void setPeerId(const std::string& peerId) { this->peerId = peerId; }
};

} // namespace aria2

#endif // D_MOCK_BT_ANNOUNCE_H
