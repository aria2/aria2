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
    std::shared_ptr<DHTMessage> message_;
    time_t timeout_;
    std::shared_ptr<DHTMessageCallback> callback_;

    Entry(const std::shared_ptr<DHTMessage>& message, time_t timeout,
          const std::shared_ptr<DHTMessageCallback>& callback):
      message_(message),
      timeout_(timeout),
      callback_(callback) {}
  };

  std::deque<Entry> messageQueue_;

public:
  MockDHTMessageDispatcher() {}

  virtual ~MockDHTMessageDispatcher() {}

  virtual void
  addMessageToQueue(const std::shared_ptr<DHTMessage>& message,
                    time_t timeout,
                    const std::shared_ptr<DHTMessageCallback>& callback =
                    std::shared_ptr<DHTMessageCallback>())
  {
    messageQueue_.push_back(Entry(message, timeout, callback));
  }

  virtual void
  addMessageToQueue(const std::shared_ptr<DHTMessage>& message,
                    const std::shared_ptr<DHTMessageCallback>& callback =
                    std::shared_ptr<DHTMessageCallback>())
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
