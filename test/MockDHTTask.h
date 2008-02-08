#ifndef _D_MOCK_DHT_TASK_H_
#define _D_MOCK_DHT_TASK_H_

#include "DHTTask.h"
#include "DHTNode.h"

namespace aria2 {

class MockDHTTask:public DHTTask {
public:
  SharedHandle<DHTNode> _remoteNode;

  MockDHTTask(const SharedHandle<DHTNode>& remoteNode):_remoteNode(remoteNode) {}

  virtual ~MockDHTTask() {}

  virtual void startup() {}

  virtual bool finished()
  {
    return false;
  }
};

} // namespace aria2

#endif // _D_MOCK_DHT_TASK_H_
