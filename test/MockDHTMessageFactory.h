#ifndef D_MOCK_DHT_MESSAGE_FACTORY_H
#define D_MOCK_DHT_MESSAGE_FACTORY_H

#include "DHTMessageFactory.h"
#include "DHTNode.h"
#include "MockDHTMessage.h"

namespace aria2 {

class MockDHTMessageFactory:public DHTMessageFactory {
protected:
  SharedHandle<DHTNode> localNode_;
public:
  MockDHTMessageFactory() {}

  virtual ~MockDHTMessageFactory() {}

  virtual SharedHandle<DHTQueryMessage>
  createQueryMessage(const Dict* dict,
                     const std::string& ipaddr, uint16_t port)
  {
    return SharedHandle<DHTQueryMessage>();
  }

  virtual SharedHandle<DHTResponseMessage>
  createResponseMessage(const std::string& messageType,
                        const Dict* dict,
                        const std::string& ipaddr, uint16_t port)
  {
    SharedHandle<DHTNode> remoteNode(new DHTNode());
    // TODO At this point, removeNode's ID is random.
    remoteNode->setIPAddress(ipaddr);
    remoteNode->setPort(port);
    SharedHandle<MockDHTResponseMessage> m
      (new MockDHTResponseMessage(localNode_, remoteNode,
                                  downcast<String>(dict->get("t"))->s()));
    return m;
  }

  virtual SharedHandle<DHTQueryMessage>
  createPingMessage(const SharedHandle<DHTNode>& remoteNode,
                    const std::string& transactionID = "")
  {
    return SharedHandle<DHTQueryMessage>();
  }

  virtual SharedHandle<DHTResponseMessage>
  createPingReplyMessage(const SharedHandle<DHTNode>& remoteNode,
                         const unsigned char* remoteNodeID,
                         const std::string& transactionID)
  {
    return SharedHandle<DHTResponseMessage>();
  }

  virtual SharedHandle<DHTQueryMessage>
  createFindNodeMessage(const SharedHandle<DHTNode>& remoteNode,
                        const unsigned char* targetNodeID,
                        const std::string& transactionID = "")
  {
    return SharedHandle<DHTQueryMessage>();
  }

  virtual SharedHandle<DHTResponseMessage>
  createFindNodeReplyMessage
  (const SharedHandle<DHTNode>& remoteNode,
   const std::vector<SharedHandle<DHTNode> >& closestKNodes,
   const std::string& transactionID)
  {
    return SharedHandle<DHTResponseMessage>();
  }

  virtual SharedHandle<DHTQueryMessage>
  createGetPeersMessage(const SharedHandle<DHTNode>& remoteNode,
                        const unsigned char* infoHash,
                        const std::string& transactionID)
  {
    return SharedHandle<DHTQueryMessage>();
  }

  virtual SharedHandle<DHTResponseMessage>
  createGetPeersReplyMessage
  (const SharedHandle<DHTNode>& remoteNode,
   const std::vector<SharedHandle<DHTNode> >& closestKNodes,
   const std::vector<SharedHandle<Peer> >& peers,
   const std::string& token,
   const std::string& transactionID)
  {
    return SharedHandle<DHTResponseMessage>();
  }
  
  virtual SharedHandle<DHTQueryMessage>
  createAnnouncePeerMessage(const SharedHandle<DHTNode>& remoteNode,
                            const unsigned char* infoHash,
                            uint16_t tcpPort,
                            const std::string& token,
                            const std::string& transactionID = "")
  {
    return SharedHandle<DHTQueryMessage>();
  }

  virtual SharedHandle<DHTResponseMessage>
  createAnnouncePeerReplyMessage(const SharedHandle<DHTNode>& remoteNode,
                                 const std::string& transactionID)
  {
    return SharedHandle<DHTResponseMessage>();
  }

  virtual SharedHandle<DHTMessage>
  createUnknownMessage(const unsigned char* data, size_t length,
                       const std::string& ipaddr, uint16_t port)
  {
    return SharedHandle<DHTMessage>();
  }

  void setLocalNode(const SharedHandle<DHTNode>& node)
  {
    localNode_ = node;
  }
};

} // namespace aria2

#endif // D_MOCK_DHT_MESSAGE_FACTORY_H
