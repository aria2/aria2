#ifndef _D_MOCK_DHT_MESSAGE_FACTORY_H_
#define _D_MOCK_DHT_MESSAGE_FACTORY_H_

#include "DHTMessageFactory.h"
#include "DHTNode.h"
#include "MockDHTMessage.h"
#include "bencode.h"

namespace aria2 {

class MockDHTMessageFactory:public DHTMessageFactory {
protected:
  SharedHandle<DHTNode> _localNode;
public:
  MockDHTMessageFactory() {}

  virtual ~MockDHTMessageFactory() {}

  virtual SharedHandle<DHTMessage>
  createQueryMessage(const bencode::BDE& dict,
		     const std::string& ipaddr, uint16_t port)
  {
    return SharedHandle<DHTMessage>();
  }

  virtual SharedHandle<DHTMessage>
  createResponseMessage(const std::string& messageType,
			const bencode::BDE& dict,
			const std::string& ipaddr, uint16_t port)
  {
    SharedHandle<DHTNode> remoteNode(new DHTNode());
    // TODO At this point, removeNode's ID is random.
    remoteNode->setIPAddress(ipaddr);
    remoteNode->setPort(port);
    SharedHandle<MockDHTMessage> m
      (new MockDHTMessage(_localNode, remoteNode, dict["t"].s()));
    m->setReply(true);
    return m;
  }

  virtual SharedHandle<DHTMessage>
  createPingMessage(const SharedHandle<DHTNode>& remoteNode,
		    const std::string& transactionID = "")
  {
    return SharedHandle<DHTMessage>();
  }

  virtual SharedHandle<DHTMessage>
  createPingReplyMessage(const SharedHandle<DHTNode>& remoteNode,
			 const unsigned char* remoteNodeID,
			 const std::string& transactionID)
  {
    return SharedHandle<DHTMessage>();
  }

  virtual SharedHandle<DHTMessage>
  createFindNodeMessage(const SharedHandle<DHTNode>& remoteNode,
			const unsigned char* targetNodeID,
			const std::string& transactionID = "")
  {
    return SharedHandle<DHTMessage>();
  }

  virtual SharedHandle<DHTMessage>
  createFindNodeReplyMessage(const SharedHandle<DHTNode>& remoteNode,
			     const std::deque<SharedHandle<DHTNode> >& closestKNodes,
			     const std::string& transactionID)
  {
    return SharedHandle<DHTMessage>();
  }

  virtual SharedHandle<DHTMessage>
  createGetPeersMessage(const SharedHandle<DHTNode>& remoteNode,
			const unsigned char* infoHash,
			const std::string& transactionID)
  {
    return SharedHandle<DHTMessage>();
  }

  virtual SharedHandle<DHTMessage>
  createGetPeersReplyMessage(const SharedHandle<DHTNode>& remoteNode,
			     const std::deque<SharedHandle<DHTNode> >& closestKNodes,
			     const std::string& token,
			     const std::string& transactionID)
  {
    return SharedHandle<DHTMessage>();
  }


  virtual SharedHandle<DHTMessage>
  createGetPeersReplyMessage(const SharedHandle<DHTNode>& remoteNode,
			     const std::deque<SharedHandle<Peer> >& peers,
			     const std::string& token,
			     const std::string& transactionID)
  {
    return SharedHandle<DHTMessage>();
  }
  
  virtual SharedHandle<DHTMessage>
  createAnnouncePeerMessage(const SharedHandle<DHTNode>& remoteNode,
			    const unsigned char* infoHash,
			    uint16_t tcpPort,
			    const std::string& token,
			    const std::string& transactionID = "")
  {
    return SharedHandle<DHTMessage>();
  }

  virtual SharedHandle<DHTMessage>
  createAnnouncePeerReplyMessage(const SharedHandle<DHTNode>& remoteNode,
				 const std::string& transactionID)
  {
    return SharedHandle<DHTMessage>();
  }

  virtual SharedHandle<DHTMessage>
  createUnknownMessage(const unsigned char* data, size_t length,
		       const std::string& ipaddr, uint16_t port)
  {
    return SharedHandle<DHTMessage>();
  }

  void setLocalNode(const SharedHandle<DHTNode>& node)
  {
    _localNode = node;
  }
};

} // namespace aria2

#endif // _D_MOCK_DHT_MESSAGE_FACTORY_H_
