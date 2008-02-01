#ifndef _D_MOCK_DHT_TASK_H_
#define _D_MOCK_DHT_TASK_H_

#include "DHTTask.h"
#include "DHTNode.h"

class MockDHTTask:public DHTTask {
public:
  DHTNodeHandle _remoteNode;

  MockDHTTask(const DHTNodeHandle& remoteNode):_remoteNode(remoteNode) {}

  virtual ~MockDHTTask() {}

  virtual void startup() {}

  virtual bool finished()
  {
    return false;
  }
};

#endif // _D_MOCK_DHT_TASK_H_
