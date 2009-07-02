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

  DNSCache _cache;
public:
  void setUp()
  {
    _cache = DNSCache();
    _cache.put("www", "192.168.0.1", 80);
    _cache.put("www", "::1", 80);
    _cache.put("ftp", "192.168.0.1", 21);
    _cache.put("proxy", "192.168.1.2", 8080);
  }

  void testFind();
  void testMarkBad();
  void testPutBadAddr();
  void testRemove();
};


CPPUNIT_TEST_SUITE_REGISTRATION(DNSCacheTest);

void DNSCacheTest::testFind()
{
  CPPUNIT_ASSERT_EQUAL(std::string("192.168.0.1"), _cache.find("www", 80));
  CPPUNIT_ASSERT_EQUAL(std::string("192.168.0.1"), _cache.find("ftp", 21));
  CPPUNIT_ASSERT_EQUAL(std::string("192.168.1.2"), _cache.find("proxy", 8080));
  CPPUNIT_ASSERT_EQUAL(std::string(""), _cache.find("www", 8080));
  CPPUNIT_ASSERT_EQUAL(std::string(""), _cache.find("another", 80));
}

void DNSCacheTest::testMarkBad()
{
  _cache.markBad("www", "192.168.0.1", 80);
  CPPUNIT_ASSERT_EQUAL(std::string("::1"), _cache.find("www", 80));
}

void DNSCacheTest::testPutBadAddr()
{
  _cache.markBad("www", "192.168.0.1", 80);
  _cache.put("www", "192.168.0.1", 80);
  CPPUNIT_ASSERT_EQUAL(std::string("::1"), _cache.find("www", 80));
}

void DNSCacheTest::testRemove()
{
  _cache.remove("www", 80);
  CPPUNIT_ASSERT_EQUAL(std::string(""), _cache.find("www", 80));
}

} // namespace aria2
