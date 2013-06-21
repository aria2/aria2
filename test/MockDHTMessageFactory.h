#ifndef D_MOCK_DHT_MESSAGE_FACTORY_H
#define D_MOCK_DHT_MESSAGE_FACTORY_H

#include "DHTMessageFactory.h"
#include "DHTNode.h"
#include "MockDHTMessage.h"

namespace aria2 {

class MockDHTMessageFactory:public DHTMessageFactory {
protected:
  std::shared_ptr<DHTNode> localNode_;
public:
  MockDHTMessageFactory() {}

  virtual ~MockDHTMessageFactory() {}

  virtual std::shared_ptr<DHTQueryMessage>
  createQueryMessage(const Dict* dict,
                     const std::string& ipaddr, uint16_t port)
  {
    return std::shared_ptr<DHTQueryMessage>();
  }

  virtual std::shared_ptr<DHTResponseMessage>
  createResponseMessage(const std::string& messageType,
                        const Dict* dict,
                        const std::string& ipaddr, uint16_t port)
  {
    std::shared_ptr<DHTNode> remoteNode(new DHTNode());
    // TODO At this point, removeNode's ID is random.
    remoteNode->setIPAddress(ipaddr);
    remoteNode->setPort(port);
    std::shared_ptr<MockDHTResponseMessage> m
      (new MockDHTResponseMessage(localNode_, remoteNode,
                                  downcast<String>(dict->get("t"))->s()));
    return m;
  }

  virtual std::shared_ptr<DHTQueryMessage>
  createPingMessage(const std::shared_ptr<DHTNode>& remoteNode,
                    const std::string& transactionID = "")
  {
    return std::shared_ptr<DHTQueryMessage>();
  }

  virtual std::shared_ptr<DHTResponseMessage>
  createPingReplyMessage(const std::shared_ptr<DHTNode>& remoteNode,
                         const unsigned char* remoteNodeID,
                         const std::string& transactionID)
  {
    return std::shared_ptr<DHTResponseMessage>();
  }

  virtual std::shared_ptr<DHTQueryMessage>
  createFindNodeMessage(const std::shared_ptr<DHTNode>& remoteNode,
                        const unsigned char* targetNodeID,
                        const std::string& transactionID = "")
  {
    return std::shared_ptr<DHTQueryMessage>();
  }

  virtual std::shared_ptr<DHTResponseMessage>
  createFindNodeReplyMessage
  (const std::shared_ptr<DHTNode>& remoteNode,
   const std::vector<std::shared_ptr<DHTNode> >& closestKNodes,
   const std::string& transactionID)
  {
    return std::shared_ptr<DHTResponseMessage>();
  }

  virtual std::shared_ptr<DHTQueryMessage>
  createGetPeersMessage(const std::shared_ptr<DHTNode>& remoteNode,
                        const unsigned char* infoHash,
                        const std::string& transactionID)
  {
    return std::shared_ptr<DHTQueryMessage>();
  }

  virtual std::shared_ptr<DHTResponseMessage>
  createGetPeersReplyMessage
  (const std::shared_ptr<DHTNode>& remoteNode,
   const std::vector<std::shared_ptr<DHTNode> >& closestKNodes,
   const std::vector<std::shared_ptr<Peer> >& peers,
   const std::string& token,
   const std::string& transactionID)
  {
    return std::shared_ptr<DHTResponseMessage>();
  }

  virtual std::shared_ptr<DHTQueryMessage>
  createAnnouncePeerMessage(const std::shared_ptr<DHTNode>& remoteNode,
                            const unsigned char* infoHash,
                            uint16_t tcpPort,
                            const std::string& token,
                            const std::string& transactionID = "")
  {
    return std::shared_ptr<DHTQueryMessage>();
  }

  virtual std::shared_ptr<DHTResponseMessage>
  createAnnouncePeerReplyMessage(const std::shared_ptr<DHTNode>& remoteNode,
                                 const std::string& transactionID)
  {
    return std::shared_ptr<DHTResponseMessage>();
  }

  virtual std::shared_ptr<DHTMessage>
  createUnknownMessage(const unsigned char* data, size_t length,
                       const std::string& ipaddr, uint16_t port)
  {
    return std::shared_ptr<DHTMessage>();
  }

  void setLocalNode(const std::shared_ptr<DHTNode>& node)
  {
    localNode_ = node;
  }
};

} // namespace aria2

#endif // D_MOCK_DHT_MESSAGE_FACTORY_H
