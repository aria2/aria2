#include "DHTMessageTrackerEntry.h"
#include "Exception.h"
#include "Util.h"
#include "MockDHTMessage.h"
#include "DHTNode.h"
#include "MetaEntry.h"
#include "DHTMessageCallback.h"
#include <cppunit/extensions/HelperMacros.h>

class DHTMessageTrackerEntryTest:public CppUnit::TestFixture {

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
  DHTNodeHandle localNode = new DHTNode();
  try {
    MockDHTMessageHandle msg1 = new MockDHTMessage(localNode, new DHTNode());
    MockDHTMessageHandle msg2 = new MockDHTMessage(localNode, new DHTNode());
    
    DHTMessageTrackerEntry entry(msg1, 30);
    
    CPPUNIT_ASSERT(entry.match(msg1->getTransactionID(),
			       msg1->getRemoteNode()->getIPAddress(),
			       msg1->getRemoteNode()->getPort()));
    CPPUNIT_ASSERT(!entry.match(msg2->getTransactionID(),
				msg2->getRemoteNode()->getIPAddress(),
				msg2->getRemoteNode()->getPort()));
  } catch(Exception* e) {
    cerr << *e << endl;
    CPPUNIT_FAIL("exception thrown.");
  }
}

void DHTMessageTrackerEntryTest::testHandleTimeout()
{
}
