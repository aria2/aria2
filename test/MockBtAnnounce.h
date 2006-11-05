#ifndef _D_MOCK_BT_ANNOUNCE_H_
#define _D_MOCK_BT_ANNOUNCE_H_

#include "BtAnnounce.h"

class MockBtAnnounce : public BtAnnounce {
private:
  bool announceReady;
  string announceUrl;
  string peerId;
public:
  MockBtAnnounce() {}
  virtual ~MockBtAnnounce() {}

  virtual bool isAnnounceReady() {
    return announceReady;
  }

  void setAnnounceReady(bool flag) {
    this->announceReady = flag;
  }

  virtual string getAnnounceUrl() {
    return announceUrl;
  }

  void setAnnounceUrl(const string& url) {
    this->announceUrl = url;
  }

  virtual void announceStart() {}

  virtual void announceSuccess() {}

  virtual void announceFailure() {}

  virtual bool isAllAnnounceFailed() {
    return false;
  }

  virtual void resetAnnounce() {}

  virtual void processAnnounceResponse(const char* trackerResponse,
				       size_t trackerResponseLength) {}

  virtual bool noMoreAnnounce() {
    return false;
  }

  virtual void shuffleAnnounce() {
  }

  virtual string getPeerId() {
    return peerId;
  }

  void setPeerId(const string& peerId) {
    this->peerId = peerId;
  }
};

#endif // _D_MOCK_BT_ANNOUNCE_H_
