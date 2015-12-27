#include "DHTMessageTracker.h"

#include <cppunit/extensions/HelperMacros.h>

#include "Exception.h"
#include "util.h"
#include "MockDHTMessage.h"
#include "MockDHTMessageCallback.h"
#include "DHTNode.h"
#include "DHTMessageTrackerEntry.h"
#include "DHTRoutingTable.h"
#include "MockDHTMessageFactory.h"

namespace aria2 {

class DHTMessageTrackerTest : public CppUnit::TestFixture {

  CPPUNIT_TEST_SUITE(DHTMessageTrackerTest);
  CPPUNIT_TEST(testMessageArrived);
  CPPUNIT_TEST(testHandleTimeout);
  CPPUNIT_TEST_SUITE_END();

public:
  void setUp() {}

  void tearDown() {}

  void testMessageArrived();

  void testHandleTimeout();
};

CPPUNIT_TEST_SUITE_REGISTRATION(DHTMessageTrackerTest);

void DHTMessageTrackerTest::testMessageArrived()
{
  auto localNode = std::make_shared<DHTNode>();
  auto routingTable = make_unique<DHTRoutingTable>(localNode);
  auto factory = make_unique<MockDHTMessageFactory>();
  factory->setLocalNode(localNode);

  auto r1 = std::make_shared<DHTNode>();
  r1->setIPAddress("192.168.0.1");
  r1->setPort(6881);
  auto r2 = std::make_shared<DHTNode>();
  r2->setIPAddress("192.168.0.2");
  r2->setPort(6882);
  auto r3 = std::make_shared<DHTNode>();
  r3->setIPAddress("192.168.0.3");
  r3->setPort(6883);

  auto m1 = make_unique<MockDHTMessage>(localNode, r1);
  auto m2 = make_unique<MockDHTMessage>(localNode, r2);
  auto m3 = make_unique<MockDHTMessage>(localNode, r3);

  DHTMessageTracker tracker;
  tracker.setRoutingTable(routingTable.get());
  tracker.setMessageFactory(factory.get());
  tracker.addMessage(m1.get(), DHT_MESSAGE_TIMEOUT);
  tracker.addMessage(m2.get(), DHT_MESSAGE_TIMEOUT);
  tracker.addMessage(m3.get(), DHT_MESSAGE_TIMEOUT);

  {
    Dict resDict;
    resDict.put("t", m2->getTransactionID());

    auto p =
        tracker.messageArrived(&resDict, r2->getIPAddress(), r2->getPort());
    auto& reply = p.first;

    CPPUNIT_ASSERT(reply);
    CPPUNIT_ASSERT(!tracker.getEntryFor(m2.get()));
    CPPUNIT_ASSERT_EQUAL((size_t)2, tracker.countEntry());
  }
  {
    Dict resDict;
    resDict.put("t", m3->getTransactionID());

    auto p =
        tracker.messageArrived(&resDict, r3->getIPAddress(), r3->getPort());
    auto& reply = p.first;

    CPPUNIT_ASSERT(reply);
    CPPUNIT_ASSERT(!tracker.getEntryFor(m3.get()));
    CPPUNIT_ASSERT_EQUAL((size_t)1, tracker.countEntry());
  }
  {
    Dict resDict;
    resDict.put("t", m1->getTransactionID());

    auto p = tracker.messageArrived(&resDict, "192.168.1.100", 6889);
    auto& reply = p.first;

    CPPUNIT_ASSERT(!reply);
  }
}

void DHTMessageTrackerTest::testHandleTimeout() {}

} // namespace aria2
