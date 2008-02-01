#ifndef _D_MOCK_DHT_TASK_QUEUE_H_
#define _D_MOCK_DHT_TASK_QUEUE_H_

#include "DHTTaskQueue.h"

class MockDHTTaskQueue:public DHTTaskQueue {
public:
  DHTTasks _periodicTaskQueue1;

  DHTTasks _periodicTaskQueue2;

  DHTTasks _immediateTaskQueue;

  MockDHTTaskQueue() {}

  virtual ~MockDHTTaskQueue() {}

  virtual void executeTask() {}

  virtual void addPeriodicTask1(const DHTTaskHandle& task)
  {
    _periodicTaskQueue1.push_back(task);
  }

  virtual void addPeriodicTask2(const DHTTaskHandle& task)
  {
    _periodicTaskQueue2.push_back(task);
  }

  virtual void addImmediateTask(const DHTTaskHandle& task)
  {
    _immediateTaskQueue.push_back(task);
  }
};

#endif // _D_MOCK_DHT_TASK_QUEUE_H_
