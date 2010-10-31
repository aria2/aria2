#ifndef D_MOCK_DHT_MESSAGE_CALLBACK_H
#define D_MOCK_DHT_MESSAGE_CALLBACK_H

#include "DHTMessageCallback.h"

namespace aria2 {

class MockDHTMessageCallback:public DHTMessageCallback {
public:
  MockDHTMessageCallback() {}

  virtual ~MockDHTMessageCallback() {}

  virtual void visit(const DHTAnnouncePeerReplyMessage* message) {}

  virtual void visit(const DHTFindNodeReplyMessage* message) {}

  virtual void visit(const DHTGetPeersReplyMessage* message) {}

  virtual void visit(const DHTPingReplyMessage* message) {}

  virtual void onTimeout(const SharedHandle<DHTNode>& remoteNode) {}
};

} // namespace aria2

#endif // D_MOCK_DHT_MESSAGE_CALLBACK_H
