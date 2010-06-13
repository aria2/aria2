#ifndef _D_MOCK_DHT_MESSAGE_H_
#define _D_MOCK_DHT_MESSAGE_H_

#include "DHTMessage.h"
#include "DHTQueryMessage.h"
#include "DHTResponseMessage.h"

#include <deque>

#include "DHTNode.h"
#include "Peer.h"
#include "BDE.h"

namespace aria2 {

class DHTMessageCallback;

class MockDHTMessage:public DHTMessage {
public:
  bool _isReply;

  std::string _messageType;

  std::vector<SharedHandle<DHTNode> > _nodes;

  std::vector<SharedHandle<Peer> > _peers;

  std::string _token;
public:
  MockDHTMessage(const SharedHandle<DHTNode>& localNode,
                 const SharedHandle<DHTNode>& remoteNode,
                 const std::string& messageType = "mock",
                 const std::string& transactionID = ""):
    DHTMessage(localNode, remoteNode, transactionID), _isReply(false), _messageType(messageType) {}
  
  virtual ~MockDHTMessage() {}

  virtual void doReceivedAction() {}

  virtual bool send() { return true; }

  virtual bool isReply() const { return _isReply; }

  void setReply(bool f) { _isReply = f; }

  virtual const std::string& getMessageType() const { return _messageType; }

  virtual std::string toString() const { return "MockDHTMessage"; }
};

class MockDHTQueryMessage:public DHTQueryMessage {
public:
  std::string _messageType;

  std::vector<SharedHandle<DHTNode> > _nodes;

  std::vector<SharedHandle<Peer> > _peers;

  std::string _token;
public:
  MockDHTQueryMessage(const SharedHandle<DHTNode>& localNode,
                      const SharedHandle<DHTNode>& remoteNode,
                      const std::string& messageType = "mock",
                      const std::string& transactionID = ""):
    DHTQueryMessage(localNode, remoteNode, transactionID),
    _messageType(messageType) {}
  
  virtual ~MockDHTQueryMessage() {}

  virtual void doReceivedAction() {}

  virtual bool send() { return true; }

  virtual bool isReply() const { return false; }

  virtual const std::string& getMessageType() const { return _messageType; }

  virtual std::string toString() const { return "MockDHTMessage"; }

  virtual BDE getArgument() { return BDE::dict(); }
};

class MockDHTResponseMessage:public DHTResponseMessage {
public:
  std::string _messageType;

  std::vector<SharedHandle<DHTNode> > _nodes;

  std::vector<SharedHandle<Peer> > _peers;

  std::string _token;
public:
  MockDHTResponseMessage(const SharedHandle<DHTNode>& localNode,
                         const SharedHandle<DHTNode>& remoteNode,
                         const std::string& messageType = "mock",
                         const std::string& transactionID = ""):
    DHTResponseMessage(localNode, remoteNode, transactionID),
    _messageType(messageType) {}
  
  virtual ~MockDHTResponseMessage() {}

  virtual void doReceivedAction() {}

  virtual bool send() { return true; }

  virtual bool isReply() const { return true; }

  virtual const std::string& getMessageType() const { return _messageType; }

  virtual std::string toString() const { return "MockDHTMessage"; }

  virtual BDE getResponse() { return BDE::dict(); }

  virtual void accept(DHTMessageCallback* callback) {}
};

} // namespace aria2

#endif // _D_MOCK_DHT_MESSAGE_H_
