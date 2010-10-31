#ifndef D_MOCK_DHT_TASK_QUEUE_H
#define D_MOCK_DHT_TASK_QUEUE_H

#include "DHTTaskQueue.h"

namespace aria2 {

class MockDHTTaskQueue:public DHTTaskQueue {
public:
  std::deque<SharedHandle<DHTTask> > periodicTaskQueue1_;

  std::deque<SharedHandle<DHTTask> > periodicTaskQueue2_;

  std::deque<SharedHandle<DHTTask> > immediateTaskQueue_;

  MockDHTTaskQueue() {}

  virtual ~MockDHTTaskQueue() {}

  virtual void executeTask() {}

  virtual void addPeriodicTask1(const SharedHandle<DHTTask>& task)
  {
    periodicTaskQueue1_.push_back(task);
  }

  virtual void addPeriodicTask2(const SharedHandle<DHTTask>& task)
  {
    periodicTaskQueue2_.push_back(task);
  }

  virtual void addImmediateTask(const SharedHandle<DHTTask>& task)
  {
    immediateTaskQueue_.push_back(task);
  }
};

} // namespace aria2

#endif // D_MOCK_DHT_TASK_QUEUE_H
