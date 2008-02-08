#ifndef _D_MOCK_DHT_TASK_FACTORY_H_
#define _D_MOCK_DHT_TASK_FACTORY_H_

#include "DHTTaskFactory.h"

namespace aria2 {

class MockDHTTaskFactory:public DHTTaskFactory {
public:
  virtual ~MockDHTTaskFactory() {}

  virtual SharedHandle<DHTTask> createPingTask(const SharedHandle<DHTNode>& remoteNode,
				       size_t numRetry = 0)
  {
    return 0;
  }

  virtual SharedHandle<DHTTask> createGetIDTask(const SharedHandle<DHTNode>& remoteNode,
					size_t numRetry = 0)
  {
    return 0;
  }

  virtual SharedHandle<DHTTask> createNodeLookupTask(const unsigned char* targetID)
  {
    return 0;
  }

  virtual SharedHandle<DHTTask> createBucketRefreshTask()
  {
    return 0;
  }

  virtual SharedHandle<DHTTask> createPeerLookupTask(const SharedHandle<BtContext>& ctx)
  {
    return 0;
  }
  
  virtual SharedHandle<DHTTask> createPeerAnnounceTask(const unsigned char* infoHash)
  {
    return 0;
  }

  virtual SharedHandle<DHTTask> createReplaceNodeTask(const SharedHandle<DHTBucket>& bucket,
						      const SharedHandle<DHTNode>& newNode)
  {
    return 0;
  }
};

} // namespace aria2

#endif // _D_MOCK_DHT_TASK_FACTORY_H_
