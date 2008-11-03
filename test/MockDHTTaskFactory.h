#ifndef _D_MOCK_DHT_TASK_FACTORY_H_
#define _D_MOCK_DHT_TASK_FACTORY_H_

#include "DHTTaskFactory.h"

namespace aria2 {

class MockDHTTaskFactory:public DHTTaskFactory {
public:
  virtual ~MockDHTTaskFactory() {}

  virtual SharedHandle<DHTTask>
  createPingTask(const SharedHandle<DHTNode>& remoteNode,
		 size_t numRetry = 0)
  {
    return SharedHandle<DHTTask>();
  }

  virtual SharedHandle<DHTTask>
  createGetIDTask(const SharedHandle<DHTNode>& remoteNode,
		  size_t numRetry = 0)
  {
    return SharedHandle<DHTTask>();
  }

  virtual SharedHandle<DHTTask>
  createNodeLookupTask(const unsigned char* targetID)
  {
    return SharedHandle<DHTTask>();
  }

  virtual SharedHandle<DHTTask> createBucketRefreshTask()
  {
    return SharedHandle<DHTTask>();
  }

  virtual SharedHandle<DHTTask>
  createPeerLookupTask(const SharedHandle<BtContext>& ctx,
		       const SharedHandle<BtRuntime>& btRuntime,
		       const SharedHandle<PeerStorage>& peerStorage)
  {
    return SharedHandle<DHTTask>();
  }
  
  virtual SharedHandle<DHTTask>
  createPeerAnnounceTask(const unsigned char* infoHash)
  {
    return SharedHandle<DHTTask>();
  }

  virtual SharedHandle<DHTTask>
  createReplaceNodeTask(const SharedHandle<DHTBucket>& bucket,
			const SharedHandle<DHTNode>& newNode)
  {
    return SharedHandle<DHTTask>();
  }
};

} // namespace aria2

#endif // _D_MOCK_DHT_TASK_FACTORY_H_
