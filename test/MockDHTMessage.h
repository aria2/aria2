#ifndef _D_MOCK_DHT_MESSAGE_H_
#define _D_MOCK_DHT_MESSAGE_H_

#include "DHTMessage.h"

namespace aria2 {

class MockDHTMessage:public DHTMessage {
private:
  bool _isReply;

  std::string _messageType;
public:
  MockDHTMessage(const SharedHandle<DHTNode>& localNode,
		 const SharedHandle<DHTNode>& remoteNode,
		 const std::string& transactionID = ""):
    DHTMessage(localNode, remoteNode, transactionID), _isReply(false), _messageType("mock") {}
  
  virtual ~MockDHTMessage() {}

  virtual void doReceivedAction() {}

  virtual void send() {}

  virtual bool isReply() const { return _isReply; }

  void setReply(bool f) { _isReply = f; }

  virtual std::string getMessageType() const { return _messageType; }

  virtual std::string toString() const { return "MockDHTMessage"; }

  virtual void validate() const {}
};

} // namespace aria2

#endif // _D_MOCK_DHT_MESSAGE_H_
