#ifndef D_MOCK_DHT_MESSAGE_DISPATCHER_H
#define D_MOCK_DHT_MESSAGE_DISPATCHER_H

#include "DHTMessageDispatcher.h"
#include "DHTMessageCallback.h"
#include "DHTMessage.h"
#include "DHTConstants.h"

namespace aria2 {

class MockDHTMessageDispatcher : public DHTMessageDispatcher {
public:
  struct Entry {
    std::unique_ptr<DHTMessage> message_;
    std::chrono::seconds timeout_;
    std::unique_ptr<DHTMessageCallback> callback_;

    Entry(std::unique_ptr<DHTMessage> message, std::chrono::seconds timeout,
          std::unique_ptr<DHTMessageCallback> callback)
        : message_{std::move(message)},
          timeout_{std::move(timeout)},
          callback_{std::move(callback)}
    {
    }
  };

  std::deque<Entry> messageQueue_;

public:
  MockDHTMessageDispatcher() {}

  virtual void
  addMessageToQueue(std::unique_ptr<DHTMessage> message,
                    std::chrono::seconds timeout,
                    std::unique_ptr<DHTMessageCallback> callback =
                        std::unique_ptr<DHTMessageCallback>{}) CXX11_OVERRIDE
  {
    messageQueue_.push_back(
        Entry(std::move(message), std::move(timeout), std::move(callback)));
  }

  virtual void
  addMessageToQueue(std::unique_ptr<DHTMessage> message,
                    std::unique_ptr<DHTMessageCallback> callback =
                        std::unique_ptr<DHTMessageCallback>{}) CXX11_OVERRIDE
  {
    messageQueue_.push_back(
        Entry(std::move(message), DHT_MESSAGE_TIMEOUT, std::move(callback)));
  }

  virtual void sendMessages() CXX11_OVERRIDE {}

  virtual size_t countMessageInQueue() const CXX11_OVERRIDE
  {
    return messageQueue_.size();
  }
};

} // namespace aria2

#endif // D_MOCK_DHT_MESSAGE_DISPATCHER_H
