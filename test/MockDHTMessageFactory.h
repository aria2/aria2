#ifndef _D_MOCK_DHT_MESSAGE_FACTORY_H_
#define _D_MOCK_DHT_MESSAGE_FACTORY_H_

#include "DHTMessageFactory.h"
#include "DHTNode.h"
#include "MockDHTMessage.h"
#include "Dictionary.h"
#include "Data.h"

class MockDHTMessageFactory:public DHTMessageFactory {
private:
  DHTNodeHandle _localNode;
public:
  MockDHTMessageFactory() {}

  virtual ~MockDHTMessageFactory() {}

  virtual DHTMessageHandle
  createQueryMessage(const Dictionary* d,
		     const string& ipaddr, uint16_t port)
  {
    return 0;
  }

  virtual DHTMessageHandle
  createResponseMessage(const string& messageType,
			const Dictionary* d,
			const DHTNodeHandle& remoteNode)
  {
    MockDHTMessageHandle m = new MockDHTMessage(_localNode, remoteNode,
						reinterpret_cast<const Data*>(d->get("t"))->toString());
    m->setReply(true);
    return m;
  }

  virtual DHTMessageHandle
  createPingMessage(const DHTNodeHandle& remoteNode,
		    const string& transactionID = "")
  {
    return 0;
  }

  virtual DHTMessageHandle
  createPingReplyMessage(const DHTNodeHandle& remoteNode,
			 const unsigned char* remoteNodeID,
			 const string& transactionID)
  {
    return 0;
  }

  virtual DHTMessageHandle
  createFindNodeMessage(const DHTNodeHandle& remoteNode,
			const unsigned char* targetNodeID,
			const string& transactionID = "")
  {
    return 0;
  }

  virtual DHTMessageHandle
  createFindNodeReplyMessage(const DHTNodeHandle& remoteNode,
			     const DHTNodes& closestKNodes,
			     const string& transactionID)
  {
    return 0;
  }

  virtual DHTMessageHandle
  createGetPeersMessage(const DHTNodeHandle& remoteNode,
			const unsigned char* infoHash,
			const string& transactionID)
  {
    return 0;
  }

  virtual DHTMessageHandle
  createGetPeersReplyMessage(const DHTNodeHandle& remoteNode,
			     const DHTNodes& closestKNodes,
			     const string& transactionID,
			     const string& token = "")
  {
    return 0;
  }

  virtual DHTMessageHandle
  createGetPeersReplyMessage(const DHTNodeHandle& remoteNode,
			     const Peers& peers,
			     const string& transactionID,
			     const string& token = "")
  {
    return 0;
  }
  
  virtual DHTMessageHandle
  createAnnouncePeerMessage(const DHTNodeHandle& remoteNode,
			    const unsigned char* infoHash,
			    uint16_t tcpPort,
			    const string& token,
			    const string& transactionID = "")
  {
    return 0;
  }

  virtual DHTMessageHandle
  createAnnouncePeerReplyMessage(const DHTNodeHandle& remoteNode,
				 const string& transactionID)
  {
    return 0;
  }

  virtual DHTMessageHandle
  createUnknownMessage(const char* data, size_t length, const string& ipaddr,
		       uint16_t port)
  {
    return 0;
  }

  void setLocalNode(const DHTNodeHandle& node)
  {
    _localNode = node;
  }
};

typedef SharedHandle<MockDHTMessageFactory> MockDHTMessageFactoryHandle;
#endif // _D_MOCK_DHT_MESSAGE_FACTORY_H_
