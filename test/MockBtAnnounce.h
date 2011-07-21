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

  virtual bool isAnnounceReady() {
    return announceReady;
  }

  void setAnnounceReady(bool flag) {
    this->announceReady = flag;
  }

  virtual std::string getAnnounceUrl() {
    return announceUrl;
  }

  void setAnnounceUrl(const std::string& url) {
    this->announceUrl = url;
  }

  virtual void announceStart() {}

  virtual void announceSuccess() {}

  virtual void announceFailure() {}

  virtual bool isAllAnnounceFailed() {
    return false;
  }

  virtual void resetAnnounce() {}

  virtual void processAnnounceResponse(const unsigned char* trackerResponse,
                                       size_t trackerResponseLength) {}

  virtual bool noMoreAnnounce() {
    return false;
  }

  virtual void shuffleAnnounce() {
  }

  virtual std::string getPeerId() {
    return peerId;
  }

  virtual void overrideMinInterval(time_t interval) {}

  virtual void setTcpPort(uint16_t port) {}

  void setPeerId(const std::string& peerId) {
    this->peerId = peerId;
  }
};

} // namespace aria2

#endif // D_MOCK_BT_ANNOUNCE_H
