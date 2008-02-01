#ifndef _D_MOCK_DHT_TASK_FACTORY_H_
#define _D_MOCK_DHT_TASK_FACTORY_H_

#include "DHTTaskFactory.h"

class MockDHTTaskFactory:public DHTTaskFactory {
public:
  virtual ~MockDHTTaskFactory() {}

  virtual DHTTaskHandle createPingTask(const DHTNodeHandle& remoteNode,
				       size_t numRetry = 0)
  {
    return 0;
  }

  virtual DHTTaskHandle createGetIDTask(const DHTNodeHandle& remoteNode,
					size_t numRetry = 0)
  {
    return 0;
  }

  virtual DHTTaskHandle createNodeLookupTask(const unsigned char* targetID)
  {
    return 0;
  }

  virtual DHTTaskHandle createBucketRefreshTask()
  {
    return 0;
  }

  virtual DHTTaskHandle createPeerLookupTask(const BtContextHandle& ctx)
  {
    return 0;
  }
  
  virtual DHTTaskHandle createPeerAnnounceTask(const unsigned char* infoHash)
  {
    return 0;
  }

  virtual DHTTaskHandle createReplaceNodeTask(const DHTBucketHandle& bucket,
					      const DHTNodeHandle& newNode)
  {
    return 0;
  }
};

#endif // _D_MOCK_DHT_TASK_FACTORY_H_
