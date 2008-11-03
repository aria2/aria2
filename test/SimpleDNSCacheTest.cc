#include "DNSCache.h"

#include <iostream>

#include <cppunit/extensions/HelperMacros.h>

#include "Exception.h"
#include "Util.h"

namespace aria2 {

class SimpleDNSCacheTest:public CppUnit::TestFixture {

  CPPUNIT_TEST_SUITE(SimpleDNSCacheTest);
  CPPUNIT_TEST(testFind);
  CPPUNIT_TEST_SUITE_END();
public:
  void testFind();
};


CPPUNIT_TEST_SUITE_REGISTRATION(SimpleDNSCacheTest);

void SimpleDNSCacheTest::testFind()
{
  SimpleDNSCache cache;
  cache.put("host1", "192.168.0.1");
  cache.put("host2", "192.168.1.2");

  CPPUNIT_ASSERT_EQUAL(std::string("192.168.0.1"), cache.find("host1"));
  CPPUNIT_ASSERT_EQUAL(std::string("192.168.1.2"), cache.find("host2"));
  CPPUNIT_ASSERT_EQUAL(std::string(""), cache.find("host3"));
}

} // namespace aria2
