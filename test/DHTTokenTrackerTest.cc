#include "DHTTokenTracker.h"
#include "Exception.h"
#include "Util.h"
#include "DHTUtil.h"
#include "DHTConstants.h"
#include <cppunit/extensions/HelperMacros.h>

class DHTTokenTrackerTest:public CppUnit::TestFixture {

  CPPUNIT_TEST_SUITE(DHTTokenTrackerTest);
  CPPUNIT_TEST(testGenerateToken);
  CPPUNIT_TEST_SUITE_END();
public:
  void setUp() {}

  void tearDown() {}

  void testGenerateToken();
};


CPPUNIT_TEST_SUITE_REGISTRATION(DHTTokenTrackerTest);

void DHTTokenTrackerTest::testGenerateToken()
{
  unsigned char infohash[DHT_ID_LENGTH];
  DHTUtil::generateRandomData(reinterpret_cast<char*>(infohash), DHT_ID_LENGTH);
  string ipaddr = "192.168.0.1";
  uint16_t port = 6881;
  
  DHTTokenTracker tracker;
  string token = tracker.generateToken(infohash, ipaddr, port);
  CPPUNIT_ASSERT(tracker.validateToken(token, infohash, ipaddr, port));

  tracker.updateTokenSecret();
  CPPUNIT_ASSERT(tracker.validateToken(token, infohash, ipaddr, port));
  string newtoken = tracker.generateToken(infohash, ipaddr, port);
  tracker.updateTokenSecret();
  CPPUNIT_ASSERT(!tracker.validateToken(token, infohash, ipaddr, port));
  CPPUNIT_ASSERT(tracker.validateToken(newtoken, infohash, ipaddr, port));
}
