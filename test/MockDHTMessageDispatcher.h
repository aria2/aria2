#ifndef D_MOCK_DHT_MESSAGE_DISPATCHER_H
#define D_MOCK_DHT_MESSAGE_DISPATCHER_H

#include "DHTMessageDispatcher.h"
#include "DHTMessageCallback.h"
#include "DHTMessage.h"
#include "DHTConstants.h"

namespace aria2 {

class MockDHTMessageDispatcher:public DHTMessageDispatcher {
public:
  class Entry {
  public:
    SharedHandle<DHTMessage> message_;
    time_t timeout_;
    SharedHandle<DHTMessageCallback> callback_;

    Entry(const SharedHandle<DHTMessage>& message, time_t timeout,
          const SharedHandle<DHTMessageCallback>& callback):
      message_(message),
      timeout_(timeout),
      callback_(callback) {}
  };

  std::deque<Entry> messageQueue_;

public:
  MockDHTMessageDispatcher() {}

  virtual ~MockDHTMessageDispatcher() {}

  virtual void
  addMessageToQueue(const SharedHandle<DHTMessage>& message,
                    time_t timeout,
                    const SharedHandle<DHTMessageCallback>& callback =
                    SharedHandle<DHTMessageCallback>())
  {
    messageQueue_.push_back(Entry(message, timeout, callback));
  }

  virtual void
  addMessageToQueue(const SharedHandle<DHTMessage>& message,
                    const SharedHandle<DHTMessageCallback>& callback =
                    SharedHandle<DHTMessageCallback>())
  {
    messageQueue_.push_back(Entry(message, DHT_MESSAGE_TIMEOUT, callback));
  }
  
  virtual void sendMessages() {}

  virtual size_t countMessageInQueue() const
  {
    return messageQueue_.size();
  }
};

} // namespace aria2

#endif // D_MOCK_DHT_MESSAGE_DISPATCHER_H
