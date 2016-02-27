#ifndef D_MOCK_DHT_MESSAGE_CALLBACK_H
#define D_MOCK_DHT_MESSAGE_CALLBACK_H

#include "DHTMessageCallback.h"

namespace aria2 {

class MockDHTMessageCallback : public DHTMessageCallback {
public:
  MockDHTMessageCallback() {}

  virtual ~MockDHTMessageCallback() {}

  virtual void visit(const DHTAnnouncePeerReplyMessage* message) CXX11_OVERRIDE
  {
  }

  virtual void visit(const DHTFindNodeReplyMessage* message) CXX11_OVERRIDE {}

  virtual void visit(const DHTGetPeersReplyMessage* message) CXX11_OVERRIDE {}

  virtual void visit(const DHTPingReplyMessage* message) CXX11_OVERRIDE {}

  virtual void
  onTimeout(const std::shared_ptr<DHTNode>& remoteNode) CXX11_OVERRIDE
  {
  }
};

} // namespace aria2

#endif // D_MOCK_DHT_MESSAGE_CALLBACK_H
