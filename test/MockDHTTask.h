#ifndef D_MOCK_DHT_TASK_H
#define D_MOCK_DHT_TASK_H

#include "DHTTask.h"

#include <cstring>

#include "DHTNode.h"
#include "DHTConstants.h"

namespace aria2 {

class MockDHTTask:public DHTTask {
public:
  SharedHandle<DHTNode> remoteNode_;

  unsigned char targetID_[DHT_ID_LENGTH];

  bool finished_;

  MockDHTTask(const SharedHandle<DHTNode>& remoteNode):
    remoteNode_(remoteNode),
    finished_(false) {}

  virtual ~MockDHTTask() {}

  virtual void startup() {}

  virtual bool finished()
  {
    return finished_;
  }

  void setTargetID(const unsigned char* targetID)
  {
    memcpy(targetID_, targetID, DHT_ID_LENGTH);
  }
};

} // namespace aria2

#endif // D_MOCK_DHT_TASK_H
