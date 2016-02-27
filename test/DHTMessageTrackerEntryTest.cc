#include "DHTMessageTrackerEntry.h"

#include <iostream>

#include <cppunit/extensions/HelperMacros.h>

#include "Exception.h"
#include "util.h"
#include "MockDHTMessage.h"
#include "DHTNode.h"
#include "DHTMessageCallback.h"

namespace aria2 {

class DHTMessageTrackerEntryTest : public CppUnit::TestFixture {

  CPPUNIT_TEST_SUITE(DHTMessageTrackerEntryTest);
  CPPUNIT_TEST(testMatch);
  CPPUNIT_TEST(testHandleTimeout);
  CPPUNIT_TEST_SUITE_END();

public:
  void setUp() {}

  void tearDown() {}

  void testMatch();

  void testHandleTimeout();
};

CPPUNIT_TEST_SUITE_REGISTRATION(DHTMessageTrackerEntryTest);

void DHTMessageTrackerEntryTest::testMatch()
{
  auto localNode = std::make_shared<DHTNode>();
  try {
    auto node1 = std::make_shared<DHTNode>();
    auto msg1 = make_unique<MockDHTMessage>(localNode, node1);
    auto node2 = std::make_shared<DHTNode>();
    auto msg2 = make_unique<MockDHTMessage>(localNode, node2);

    DHTMessageTrackerEntry entry(msg1->getRemoteNode(),
                                 msg1->getTransactionID(),
                                 msg1->getMessageType(), 30_s);

    CPPUNIT_ASSERT(entry.match(msg1->getTransactionID(),
                               msg1->getRemoteNode()->getIPAddress(),
                               msg1->getRemoteNode()->getPort()));
    CPPUNIT_ASSERT(!entry.match(msg2->getTransactionID(),
                                msg2->getRemoteNode()->getIPAddress(),
                                msg2->getRemoteNode()->getPort()));
  }
  catch (Exception& e) {
    CPPUNIT_FAIL(e.stackTrace());
  }
}

void DHTMessageTrackerEntryTest::testHandleTimeout() {}

} // namespace aria2
