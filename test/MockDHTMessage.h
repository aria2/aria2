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

class MockDHTMessage : public DHTMessage {
public:
  bool isReply_;

  std::string messageType_;

  std::vector<std::shared_ptr<DHTNode>> nodes_;

  std::vector<std::shared_ptr<Peer>> peers_;

  std::string token_;

public:
  MockDHTMessage(const std::shared_ptr<DHTNode>& localNode,
                 const std::shared_ptr<DHTNode>& remoteNode,
                 const std::string& messageType = "mock",
                 const std::string& transactionID = "")
      : DHTMessage(localNode, remoteNode, transactionID),
        isReply_(false),
        messageType_(messageType)
  {
  }

  virtual ~MockDHTMessage() {}

  virtual void doReceivedAction() CXX11_OVERRIDE {}

  virtual bool send() CXX11_OVERRIDE { return true; }

  virtual bool isReply() const CXX11_OVERRIDE { return isReply_; }

  void setReply(bool f) { isReply_ = f; }

  virtual const std::string& getMessageType() const CXX11_OVERRIDE
  {
    return messageType_;
  }

  virtual std::string toString() const CXX11_OVERRIDE
  {
    return "MockDHTMessage";
  }
};

class MockDHTQueryMessage : public DHTQueryMessage {
public:
  std::string messageType_;

  std::vector<std::shared_ptr<DHTNode>> nodes_;

  std::vector<std::shared_ptr<Peer>> peers_;

  std::string token_;

public:
  MockDHTQueryMessage(const std::shared_ptr<DHTNode>& localNode,
                      const std::shared_ptr<DHTNode>& remoteNode,
                      const std::string& messageType = "mock",
                      const std::string& transactionID = "")
      : DHTQueryMessage(localNode, remoteNode, transactionID),
        messageType_(messageType)
  {
  }

  virtual ~MockDHTQueryMessage() {}

  virtual void doReceivedAction() CXX11_OVERRIDE {}

  virtual bool send() CXX11_OVERRIDE { return true; }

  virtual bool isReply() const CXX11_OVERRIDE { return false; }

  virtual const std::string& getMessageType() const CXX11_OVERRIDE
  {
    return messageType_;
  }

  virtual std::string toString() const CXX11_OVERRIDE
  {
    return "MockDHTMessage";
  }

  virtual std::unique_ptr<Dict> getArgument() CXX11_OVERRIDE
  {
    return Dict::g();
  }
};

class MockDHTResponseMessage : public DHTResponseMessage {
public:
  std::string messageType_;

  std::vector<std::shared_ptr<DHTNode>> nodes_;

  std::vector<std::shared_ptr<Peer>> peers_;

  std::string token_;

public:
  MockDHTResponseMessage(const std::shared_ptr<DHTNode>& localNode,
                         const std::shared_ptr<DHTNode>& remoteNode,
                         const std::string& messageType = "mock",
                         const std::string& transactionID = "")
      : DHTResponseMessage(localNode, remoteNode, transactionID),
        messageType_(messageType)
  {
  }

  virtual ~MockDHTResponseMessage() {}

  virtual void doReceivedAction() CXX11_OVERRIDE {}

  virtual bool send() CXX11_OVERRIDE { return true; }

  virtual bool isReply() const CXX11_OVERRIDE { return true; }

  virtual const std::string& getMessageType() const CXX11_OVERRIDE
  {
    return messageType_;
  }

  virtual std::string toString() const CXX11_OVERRIDE
  {
    return "MockDHTMessage";
  }

  virtual std::unique_ptr<Dict> getResponse() CXX11_OVERRIDE
  {
    return Dict::g();
  }

  virtual void accept(DHTMessageCallback* callback) CXX11_OVERRIDE {}
};

} // namespace aria2

#endif // D_MOCK_DHT_MESSAGE_H
