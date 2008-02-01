#ifndef _D_MOCK_DHT_MESSAGE_CALLBACK_H_
#define _D_MOCK_DHT_MESSAGE_CALLBACK_H_

#include "DHTMessageCallback.h"

class MockDHTMessageCallback:public DHTMessageCallback {
public:
  MockDHTMessageCallback() {}

  virtual ~MockDHTMessageCallback() {}

  virtual void onReceived(const DHTMessageHandle& message) {}

  virtual void onTimeout(const DHTNodeHandle& remoteNode) {}
};

#endif // _D_MOCK_DHT_MESSAGE_CALLBACK_H_
