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

class DHTMessageTrackerTest:public CppUnit::TestFixture {

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
  SharedHandle<DHTNode> localNode(new DHTNode());
  SharedHandle<DHTRoutingTable> routingTable(new DHTRoutingTable(localNode));
  SharedHandle<MockDHTMessageFactory> factory(new MockDHTMessageFactory());
  factory->setLocalNode(localNode);

  SharedHandle<MockDHTMessage> m1(new MockDHTMessage(localNode,
                                                     SharedHandle<DHTNode>(new DHTNode())));
  SharedHandle<MockDHTMessage> m2(new MockDHTMessage(localNode,
                                                     SharedHandle<DHTNode>(new DHTNode())));
  SharedHandle<MockDHTMessage> m3(new MockDHTMessage(localNode,
                                                     SharedHandle<DHTNode>(new DHTNode())));

  m1->getRemoteNode()->setIPAddress("192.168.0.1");
  m1->getRemoteNode()->setPort(6881);
  m2->getRemoteNode()->setIPAddress("192.168.0.2");
  m2->getRemoteNode()->setPort(6882);
  m3->getRemoteNode()->setIPAddress("192.168.0.3");
  m3->getRemoteNode()->setPort(6883);

  DHTMessageTracker tracker;
  tracker.setRoutingTable(routingTable);
  tracker.setMessageFactory(factory);
  tracker.addMessage(m1, DHT_MESSAGE_TIMEOUT);
  tracker.addMessage(m2, DHT_MESSAGE_TIMEOUT);
  tracker.addMessage(m3, DHT_MESSAGE_TIMEOUT);

  {
    Dict resDict;
    resDict.put("t", m2->getTransactionID());
    
    std::pair<SharedHandle<DHTMessage>, SharedHandle<DHTMessageCallback> > p =
      tracker.messageArrived(&resDict, m2->getRemoteNode()->getIPAddress(),
                             m2->getRemoteNode()->getPort());
    SharedHandle<DHTMessage> reply = p.first;

    CPPUNIT_ASSERT(reply);
    CPPUNIT_ASSERT(!tracker.getEntryFor(m2));
    CPPUNIT_ASSERT_EQUAL((size_t)2, tracker.countEntry());
  }
  {
    Dict resDict;
    resDict.put("t", m3->getTransactionID());

    std::pair<SharedHandle<DHTMessage>, SharedHandle<DHTMessageCallback> > p =
      tracker.messageArrived(&resDict, m3->getRemoteNode()->getIPAddress(),
                             m3->getRemoteNode()->getPort());
    SharedHandle<DHTMessage> reply = p.first;

    CPPUNIT_ASSERT(reply);
    CPPUNIT_ASSERT(!tracker.getEntryFor(m3));
    CPPUNIT_ASSERT_EQUAL((size_t)1, tracker.countEntry());
  }
  {
    Dict resDict;
    resDict.put("t", m1->getTransactionID());

    std::pair<SharedHandle<DHTMessage>, SharedHandle<DHTMessageCallback> > p =
      tracker.messageArrived(&resDict, "192.168.1.100", 6889);
    SharedHandle<DHTMessage> reply = p.first;

    CPPUNIT_ASSERT(!reply);
  }
}

void DHTMessageTrackerTest::testHandleTimeout()
{
}

} // namespace aria2
