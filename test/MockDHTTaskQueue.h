#ifndef _D_MOCK_DHT_TASK_QUEUE_H_
#define _D_MOCK_DHT_TASK_QUEUE_H_

#include "DHTTaskQueue.h"

namespace aria2 {

class MockDHTTaskQueue:public DHTTaskQueue {
public:
  std::deque<SharedHandle<DHTTask> > _periodicTaskQueue1;

  std::deque<SharedHandle<DHTTask> > _periodicTaskQueue2;

  std::deque<SharedHandle<DHTTask> > _immediateTaskQueue;

  MockDHTTaskQueue() {}

  virtual ~MockDHTTaskQueue() {}

  virtual void executeTask() {}

  virtual void addPeriodicTask1(const SharedHandle<DHTTask>& task)
  {
    _periodicTaskQueue1.push_back(task);
  }

  virtual void addPeriodicTask2(const SharedHandle<DHTTask>& task)
  {
    _periodicTaskQueue2.push_back(task);
  }

  virtual void addImmediateTask(const SharedHandle<DHTTask>& task)
  {
    _immediateTaskQueue.push_back(task);
  }
};

} // namespace aria2

#endif // _D_MOCK_DHT_TASK_QUEUE_H_
