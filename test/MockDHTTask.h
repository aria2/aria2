#ifndef _D_MOCK_DHT_TASK_H_
#define _D_MOCK_DHT_TASK_H_

#include "DHTTask.h"
#include "DHTNode.h"
#include "DHTConstants.h"
#include <cstring>

namespace aria2 {

class MockDHTTask:public DHTTask {
public:
  SharedHandle<DHTNode> _remoteNode;

  unsigned char _targetID[DHT_ID_LENGTH];

  MockDHTTask(const SharedHandle<DHTNode>& remoteNode):_remoteNode(remoteNode) {}

  virtual ~MockDHTTask() {}

  virtual void startup() {}

  virtual bool finished()
  {
    return false;
  }

  void setTargetID(const unsigned char* targetID)
  {
    memcpy(_targetID, targetID, DHT_ID_LENGTH);
  }
};

} // namespace aria2

#endif // _D_MOCK_DHT_TASK_H_
