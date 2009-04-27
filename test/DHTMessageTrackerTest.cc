#include "DHTMessageTracker.h"

#include <cppunit/extensions/HelperMacros.h>

#include "Exception.h"
#include "Util.h"
#include "MockDHTMessage.h"
#include "MockDHTMessageCallback.h"
#include "DHTNode.h"
#include "DHTMessageTrackerEntry.h"
#include "DHTRoutingTable.h"
#include "MockDHTMessageFactory.h"
#include "bencode.h"

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

  class MockDHTMessageCallback2:public MockDHTMessageCallback {
  public:
    uint32_t _countOnRecivedCalled;

    MockDHTMessageCallback2():_countOnRecivedCalled(0) {}

    virtual void onReceived(const SharedHandle<DHTMessage>& message)
    {
      ++_countOnRecivedCalled;
    }
  };
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

  SharedHandle<MockDHTMessageCallback2> c2(new MockDHTMessageCallback2());
  
  DHTMessageTracker tracker;
  tracker.setRoutingTable(routingTable);
  tracker.setMessageFactory(factory);
  tracker.addMessage(m1);
  tracker.addMessage(m2, c2);
  tracker.addMessage(m3);

  {
    BDE resDict = BDE::dict();
    resDict["t"] = m2->getTransactionID();
    
    std::pair<SharedHandle<DHTMessage>, SharedHandle<DHTMessageCallback> > p =
      tracker.messageArrived(resDict, m2->getRemoteNode()->getIPAddress(),
			     m2->getRemoteNode()->getPort());
    SharedHandle<DHTMessage> reply = p.first;

    CPPUNIT_ASSERT(!reply.isNull());
    CPPUNIT_ASSERT_EQUAL((uint32_t)0, c2->_countOnRecivedCalled);
    CPPUNIT_ASSERT(tracker.getEntryFor(m2).isNull());
    CPPUNIT_ASSERT_EQUAL((size_t)2, tracker.countEntry());
  }
  {
    BDE resDict = BDE::dict();
    resDict["t"] = m3->getTransactionID();

    std::pair<SharedHandle<DHTMessage>, SharedHandle<DHTMessageCallback> > p =
      tracker.messageArrived(resDict, m3->getRemoteNode()->getIPAddress(),
			     m3->getRemoteNode()->getPort());
    SharedHandle<DHTMessage> reply = p.first;

    CPPUNIT_ASSERT(!reply.isNull());
    CPPUNIT_ASSERT(tracker.getEntryFor(m3).isNull());
    CPPUNIT_ASSERT_EQUAL((size_t)1, tracker.countEntry());
  }
  {
    BDE resDict = BDE::dict();
    resDict["t"] = m1->getTransactionID();

    std::pair<SharedHandle<DHTMessage>, SharedHandle<DHTMessageCallback> > p =
      tracker.messageArrived(resDict, "192.168.1.100", 6889);
    SharedHandle<DHTMessage> reply = p.first;

    CPPUNIT_ASSERT(reply.isNull());
  }
}

void DHTMessageTrackerTest::testHandleTimeout()
{
}

} // namespace aria2
