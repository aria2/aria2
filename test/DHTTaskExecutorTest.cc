#include "DHTTaskExecutor.h"

#include <cppunit/extensions/HelperMacros.h>

#include "MockDHTTask.h"
#include "array_fun.h"

namespace aria2 {

class DHTTaskExecutorTest : public CppUnit::TestFixture {

  CPPUNIT_TEST_SUITE(DHTTaskExecutorTest);
  CPPUNIT_TEST(testUpdate);
  CPPUNIT_TEST_SUITE_END();

public:
  void testUpdate();
};

CPPUNIT_TEST_SUITE_REGISTRATION(DHTTaskExecutorTest);

void DHTTaskExecutorTest::testUpdate()
{
  std::shared_ptr<DHTNode> rn;
  DHTTaskExecutor tex(2);
  std::shared_ptr<MockDHTTask> tasks[] = {
      std::shared_ptr<MockDHTTask>(new MockDHTTask(rn)),
      std::shared_ptr<MockDHTTask>(new MockDHTTask(rn)),
      std::shared_ptr<MockDHTTask>(new MockDHTTask(rn)),
      std::shared_ptr<MockDHTTask>(new MockDHTTask(rn))};
  tasks[1]->finished_ = true;
  for (size_t i = 0; i < arraySize(tasks); ++i) {
    tex.addTask(tasks[i]);
  }
  CPPUNIT_ASSERT_EQUAL((size_t)0, tex.getExecutingTaskSize());
  CPPUNIT_ASSERT_EQUAL((size_t)4, tex.getQueueSize());
  tex.update();
  CPPUNIT_ASSERT_EQUAL((size_t)2, tex.getExecutingTaskSize());
  CPPUNIT_ASSERT_EQUAL((size_t)1, tex.getQueueSize());
  tasks[0]->finished_ = true;
  tasks[2]->finished_ = true;
  tasks[3]->finished_ = true;
  tex.update();
  CPPUNIT_ASSERT_EQUAL((size_t)0, tex.getExecutingTaskSize());
  CPPUNIT_ASSERT_EQUAL((size_t)0, tex.getQueueSize());
}

} // namespace aria2
