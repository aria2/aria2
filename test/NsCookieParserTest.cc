#include "NsCookieParser.h"

#include <iostream>
#include <limits>

#include <cppunit/extensions/HelperMacros.h>

#include "RecoverableException.h"
#include "util.h"
#include "Cookie.h"

namespace aria2 {

class NsCookieParserTest : public CppUnit::TestFixture {

  CPPUNIT_TEST_SUITE(NsCookieParserTest);
  CPPUNIT_TEST(testParse);
  CPPUNIT_TEST(testParse_fileNotFound);
  CPPUNIT_TEST_SUITE_END();

public:
  void setUp() {}

  void tearDown() {}

  void testParse();
  void testParse_fileNotFound();
};

CPPUNIT_TEST_SUITE_REGISTRATION(NsCookieParserTest);

void NsCookieParserTest::testParse()
{
  NsCookieParser parser;
  time_t now = 0;
  auto cookies = parser.parse(A2_TEST_DIR "/nscookietest.txt", now);
  CPPUNIT_ASSERT_EQUAL((size_t)5, cookies.size());

  auto c = cookies[0].get();
  CPPUNIT_ASSERT_EQUAL(std::string("JSESSIONID"), c->getName());
  CPPUNIT_ASSERT_EQUAL(std::string("123456789"), c->getValue());
  CPPUNIT_ASSERT_EQUAL((time_t)INT32_MAX, c->getExpiryTime());
  CPPUNIT_ASSERT(c->getPersistent());
  CPPUNIT_ASSERT_EQUAL(std::string("localhost"), c->getDomain());
  CPPUNIT_ASSERT(c->getHostOnly());
  CPPUNIT_ASSERT_EQUAL(std::string("/"), c->getPath());
  CPPUNIT_ASSERT(c->getSecure());

  c = cookies[1].get();
  CPPUNIT_ASSERT_EQUAL(std::string("user"), c->getName());
  CPPUNIT_ASSERT_EQUAL(std::string("me"), c->getValue());
  CPPUNIT_ASSERT_EQUAL((time_t)1000, c->getExpiryTime());
  CPPUNIT_ASSERT(c->getPersistent());
  CPPUNIT_ASSERT_EQUAL(std::string("expired"), c->getDomain());
  CPPUNIT_ASSERT(c->getHostOnly());
  CPPUNIT_ASSERT_EQUAL(std::string("/"), c->getPath());
  CPPUNIT_ASSERT(!c->getSecure());

  c = cookies[2].get();
  CPPUNIT_ASSERT_EQUAL(std::string("passwd"), c->getName());
  CPPUNIT_ASSERT_EQUAL(std::string("secret"), c->getValue());
  CPPUNIT_ASSERT_EQUAL(std::numeric_limits<time_t>::max(), c->getExpiryTime());
  CPPUNIT_ASSERT(!c->getPersistent());
  CPPUNIT_ASSERT_EQUAL(std::string("192.168.0.1"), c->getDomain());
  CPPUNIT_ASSERT(c->getHostOnly());
  CPPUNIT_ASSERT_EQUAL(std::string("/cgi-bin"), c->getPath());
  CPPUNIT_ASSERT(!c->getSecure());

  c = cookies[3].get();
  CPPUNIT_ASSERT_EQUAL(std::string("TAX"), c->getName());
  CPPUNIT_ASSERT_EQUAL(std::string("1000"), c->getValue());
  CPPUNIT_ASSERT_EQUAL((time_t)1463304912, c->getExpiryTime());
  CPPUNIT_ASSERT(c->getPersistent());
  CPPUNIT_ASSERT_EQUAL(std::string("something"), c->getDomain());
  CPPUNIT_ASSERT(c->getHostOnly());
  CPPUNIT_ASSERT_EQUAL(std::string("/"), c->getPath());
  CPPUNIT_ASSERT(!c->getSecure());

  c = cookies[4].get();
  CPPUNIT_ASSERT_EQUAL(std::string("novalue"), c->getName());
  CPPUNIT_ASSERT_EQUAL(std::string(""), c->getValue());
  CPPUNIT_ASSERT_EQUAL((time_t)INT32_MAX, c->getExpiryTime());
  CPPUNIT_ASSERT(c->getPersistent());
  CPPUNIT_ASSERT_EQUAL(std::string("example.org"), c->getDomain());
  CPPUNIT_ASSERT(!c->getHostOnly());
  CPPUNIT_ASSERT_EQUAL(std::string("/"), c->getPath());
  CPPUNIT_ASSERT(!c->getSecure());
}

void NsCookieParserTest::testParse_fileNotFound()
{
  NsCookieParser parser;
  try {
    time_t now = 0;
    parser.parse("fileNotFound", now);
    CPPUNIT_FAIL("exception must be thrown.");
  }
  catch (RecoverableException& e) {
    // SUCCESS
  }
}

} // namespace aria2
