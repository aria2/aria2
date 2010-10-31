#ifndef D_MOCK_DHT_MESSAGE_H
#define D_MOCK_DHT_MESSAGE_H

#include "DHTMessage.h"
#include "DHTQueryMessage.h"
#include "DHTResponseMessage.h"

#include <deque>

#include "DHTNode.h"
#include "Peer.h"

namespace aria2 {

class DHTMessageCallback;

class MockDHTMessage:public DHTMessage {
public:
  bool isReply_;

  std::string messageType_;

  std::vector<SharedHandle<DHTNode> > nodes_;

  std::vector<SharedHandle<Peer> > peers_;

  std::string token_;
public:
  MockDHTMessage(const SharedHandle<DHTNode>& localNode,
                 const SharedHandle<DHTNode>& remoteNode,
                 const std::string& messageType = "mock",
                 const std::string& transactionID = ""):
    DHTMessage(localNode, remoteNode, transactionID), isReply_(false), messageType_(messageType) {}
  
  virtual ~MockDHTMessage() {}

  virtual void doReceivedAction() {}

  virtual bool send() { return true; }

  virtual bool isReply() const { return isReply_; }

  void setReply(bool f) { isReply_ = f; }

  virtual const std::string& getMessageType() const { return messageType_; }

  virtual std::string toString() const { return "MockDHTMessage"; }
};

class MockDHTQueryMessage:public DHTQueryMessage {
public:
  std::string messageType_;

  std::vector<SharedHandle<DHTNode> > nodes_;

  std::vector<SharedHandle<Peer> > peers_;

  std::string token_;
public:
  MockDHTQueryMessage(const SharedHandle<DHTNode>& localNode,
                      const SharedHandle<DHTNode>& remoteNode,
                      const std::string& messageType = "mock",
                      const std::string& transactionID = ""):
    DHTQueryMessage(localNode, remoteNode, transactionID),
    messageType_(messageType) {}
  
  virtual ~MockDHTQueryMessage() {}

  virtual void doReceivedAction() {}

  virtual bool send() { return true; }

  virtual bool isReply() const { return false; }

  virtual const std::string& getMessageType() const { return messageType_; }

  virtual std::string toString() const { return "MockDHTMessage"; }

  virtual SharedHandle<Dict> getArgument() { return Dict::g(); }
};

class MockDHTResponseMessage:public DHTResponseMessage {
public:
  std::string messageType_;

  std::vector<SharedHandle<DHTNode> > nodes_;

  std::vector<SharedHandle<Peer> > peers_;

  std::string token_;
public:
  MockDHTResponseMessage(const SharedHandle<DHTNode>& localNode,
                         const SharedHandle<DHTNode>& remoteNode,
                         const std::string& messageType = "mock",
                         const std::string& transactionID = ""):
    DHTResponseMessage(localNode, remoteNode, transactionID),
    messageType_(messageType) {}
  
  virtual ~MockDHTResponseMessage() {}

  virtual void doReceivedAction() {}

  virtual bool send() { return true; }

  virtual bool isReply() const { return true; }

  virtual const std::string& getMessageType() const { return messageType_; }

  virtual std::string toString() const { return "MockDHTMessage"; }

  virtual SharedHandle<Dict> getResponse() { return Dict::g(); }

  virtual void accept(DHTMessageCallback* callback) {}
};

} // namespace aria2

#endif // D_MOCK_DHT_MESSAGE_H
