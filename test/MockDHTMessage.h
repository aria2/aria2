#ifndef _D_MOCK_DHT_MESSAGE_H_
#define _D_MOCK_DHT_MESSAGE_H_

#include "DHTMessage.h"

class MockDHTMessage:public DHTMessage {
private:
  bool _isReply;

  string _messageType;
public:
  MockDHTMessage(const DHTNodeHandle& localNode,
		 const DHTNodeHandle& remoteNode,
		 const string& transactionID = ""):
    DHTMessage(localNode, remoteNode, transactionID), _isReply(false), _messageType("mock") {}
  
  virtual ~MockDHTMessage() {}

  virtual void doReceivedAction() {}

  virtual void send() {}

  virtual bool isReply() const { return _isReply; }

  void setReply(bool f) { _isReply = f; }

  virtual string getMessageType() const { return _messageType; }

  virtual string toString() const { return "MockDHTMessage"; }

  virtual void validate() const {}
};

typedef SharedHandle<MockDHTMessage> MockDHTMessageHandle;
#endif // _D_MOCK_DHT_MESSAGE_H_
