#ifndef _D_MOCK_DHT_MESSAGE_H_
#define _D_MOCK_DHT_MESSAGE_H_

#include "DHTMessage.h"
#include "DHTNode.h"
#include "Peer.h"
#include <deque>

namespace aria2 {

class MockDHTMessage:public DHTMessage {
public:
  bool _isReply;

  std::string _messageType;

  std::deque<SharedHandle<DHTNode> > _nodes;

  std::deque<SharedHandle<Peer> > _peers;

  std::string _token;
public:
  MockDHTMessage(const SharedHandle<DHTNode>& localNode,
		 const SharedHandle<DHTNode>& remoteNode,
		 const std::string& messageType = "mock",
		 const std::string& transactionID = ""):
    DHTMessage(localNode, remoteNode, transactionID), _isReply(false), _messageType(messageType) {}
  
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
