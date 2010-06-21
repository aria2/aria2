#include "DNSCache.h"

#include <cppunit/extensions/HelperMacros.h>

namespace aria2 {

class DNSCacheTest:public CppUnit::TestFixture {

  CPPUNIT_TEST_SUITE(DNSCacheTest);
  CPPUNIT_TEST(testFind);
  CPPUNIT_TEST(testMarkBad);
  CPPUNIT_TEST(testPutBadAddr);
  CPPUNIT_TEST(testRemove);
  CPPUNIT_TEST_SUITE_END();

  DNSCache cache_;
public:
  void setUp()
  {
    cache_ = DNSCache();
    cache_.put("www", "192.168.0.1", 80);
    cache_.put("www", "::1", 80);
    cache_.put("ftp", "192.168.0.1", 21);
    cache_.put("proxy", "192.168.1.2", 8080);
  }

  void testFind();
  void testMarkBad();
  void testPutBadAddr();
  void testRemove();
};


CPPUNIT_TEST_SUITE_REGISTRATION(DNSCacheTest);

void DNSCacheTest::testFind()
{
  CPPUNIT_ASSERT_EQUAL(std::string("192.168.0.1"), cache_.find("www", 80));
  CPPUNIT_ASSERT_EQUAL(std::string("192.168.0.1"), cache_.find("ftp", 21));
  CPPUNIT_ASSERT_EQUAL(std::string("192.168.1.2"), cache_.find("proxy", 8080));
  CPPUNIT_ASSERT_EQUAL(std::string(""), cache_.find("www", 8080));
  CPPUNIT_ASSERT_EQUAL(std::string(""), cache_.find("another", 80));
}

void DNSCacheTest::testMarkBad()
{
  cache_.markBad("www", "192.168.0.1", 80);
  CPPUNIT_ASSERT_EQUAL(std::string("::1"), cache_.find("www", 80));
}

void DNSCacheTest::testPutBadAddr()
{
  cache_.markBad("www", "192.168.0.1", 80);
  cache_.put("www", "192.168.0.1", 80);
  CPPUNIT_ASSERT_EQUAL(std::string("::1"), cache_.find("www", 80));
}

void DNSCacheTest::testRemove()
{
  cache_.remove("www", 80);
  CPPUNIT_ASSERT_EQUAL(std::string(""), cache_.find("www", 80));
}

} // namespace aria2
