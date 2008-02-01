#include "DHTMessageTracker.h"
#include "Exception.h"
#include "Util.h"
#include "MockDHTMessage.h"
#include "MockDHTMessageCallback.h"
#include "DHTNode.h"
#include "MetaEntry.h"
#include "DHTMessageTrackerEntry.h"
#include "DHTRoutingTable.h"
#include "MockDHTMessageFactory.h"
#include "Dictionary.h"
#include "Data.h"
#include <cppunit/extensions/HelperMacros.h>

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

    virtual void onReceived(const DHTMessageHandle& message)
    {
      ++_countOnRecivedCalled;
    }
  };

  typedef SharedHandle<MockDHTMessageCallback2> MockDHTMessageCallback2Handle;
};


CPPUNIT_TEST_SUITE_REGISTRATION(DHTMessageTrackerTest);

void DHTMessageTrackerTest::testMessageArrived()
{
  DHTNodeHandle localNode = new DHTNode();
  DHTRoutingTableHandle routingTable = new DHTRoutingTable(localNode);
  MockDHTMessageFactoryHandle factory = new MockDHTMessageFactory();
  factory->setLocalNode(localNode);

  MockDHTMessageHandle m1 = new MockDHTMessage(localNode, new DHTNode());
  MockDHTMessageHandle m2 = new MockDHTMessage(localNode, new DHTNode());
  MockDHTMessageHandle m3 = new MockDHTMessage(localNode, new DHTNode());

  m1->getRemoteNode()->setIPAddress("192.168.0.1");
  m1->getRemoteNode()->setPort(6881);
  m2->getRemoteNode()->setIPAddress("192.168.0.2");
  m2->getRemoteNode()->setPort(6882);
  m3->getRemoteNode()->setIPAddress("192.168.0.3");
  m3->getRemoteNode()->setPort(6883);

  MockDHTMessageCallback2Handle c2 = new MockDHTMessageCallback2();
  
  DHTMessageTracker tracker;
  tracker.setRoutingTable(routingTable);
  tracker.setMessageFactory(factory);
  tracker.addMessage(m1);
  tracker.addMessage(m2, c2);
  tracker.addMessage(m3);

  {
    SharedHandle<Dictionary> res = new Dictionary();
    res->put("t", new Data(m2->getTransactionID()));
    
    std::pair<DHTMessageHandle, DHTMessageCallbackHandle> p =
      tracker.messageArrived(res.get(), m2->getRemoteNode()->getIPAddress(), m2->getRemoteNode()->getPort());
    DHTMessageHandle reply = p.first;

    CPPUNIT_ASSERT(!reply.isNull());
    CPPUNIT_ASSERT_EQUAL((uint32_t)0, c2->_countOnRecivedCalled);
    CPPUNIT_ASSERT(tracker.getEntryFor(m2).isNull());
    CPPUNIT_ASSERT_EQUAL((size_t)2, tracker.countEntry());
  }
  {
    SharedHandle<Dictionary> res = new Dictionary();
    res->put("t", new Data(m3->getTransactionID()));

    std::pair<DHTMessageHandle, DHTMessageCallbackHandle> p = tracker.messageArrived(res.get(), m3->getRemoteNode()->getIPAddress(), m3->getRemoteNode()->getPort());
    DHTMessageHandle reply = p.first;

    CPPUNIT_ASSERT(!reply.isNull());
    CPPUNIT_ASSERT(tracker.getEntryFor(m3).isNull());
    CPPUNIT_ASSERT_EQUAL((size_t)1, tracker.countEntry());
  }
  {
    SharedHandle<Dictionary> res = new Dictionary();
    res->put("t", new Data(m1->getTransactionID()));

    std::pair<DHTMessageHandle, DHTMessageCallbackHandle> p = tracker.messageArrived(res.get(), "192.168.1.100", 6889);
    DHTMessageHandle reply = p.first;

    CPPUNIT_ASSERT(reply.isNull());
  }
}

void DHTMessageTrackerTest::testHandleTimeout()
{
}
