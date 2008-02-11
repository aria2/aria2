#ifndef _D_MOCK_DHT_MESSAGE_DISPATCHER_H_
#define _D_MOCK_DHT_MESSAGE_DISPATCHER_H_

#include "DHTMessageDispatcher.h"
#include "DHTMessageCallback.h"
#include "DHTMessage.h"
#include "DHTConstants.h"

namespace aria2 {

class MockDHTMessageDispatcher:public DHTMessageDispatcher {
public:
  class Entry {
  public:
    SharedHandle<DHTMessage> _message;
    time_t _timeout;
    SharedHandle<DHTMessageCallback> _callback;

    Entry(const SharedHandle<DHTMessage>& message, time_t timeout,
	  const SharedHandle<DHTMessageCallback>& callback):
      _message(message),
      _timeout(timeout),
      _callback(callback) {}
  };

  std::deque<Entry> _messageQueue;

public:
  MockDHTMessageDispatcher() {}

  virtual ~MockDHTMessageDispatcher() {}

  virtual void
  addMessageToQueue(const SharedHandle<DHTMessage>& message,
		    time_t timeout,
		    const SharedHandle<DHTMessageCallback>& callback = 0)
  {
    _messageQueue.push_back(Entry(message, timeout, callback));
  }

  virtual void
  addMessageToQueue(const SharedHandle<DHTMessage>& message,
		    const SharedHandle<DHTMessageCallback>& callback = 0)
  {
    _messageQueue.push_back(Entry(message, DHT_MESSAGE_TIMEOUT, callback));
  }
  
  virtual void sendMessages() {}

  virtual size_t countMessageInQueue() const
  {
    return _messageQueue.size();
  }
};

} // namespace aria2

#endif // _D_MOCK_DHT_MESSAGE_DISPATCHER_H_
