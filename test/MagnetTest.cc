#include "magnet.h"

#include <iostream>

#include <cppunit/extensions/HelperMacros.h>

namespace aria2 {

namespace magnet {

class MagnetTest:public CppUnit::TestFixture {

  CPPUNIT_TEST_SUITE(MagnetTest);
  CPPUNIT_TEST(testParse);
  CPPUNIT_TEST_SUITE_END();
public:
  void testParse();
};


CPPUNIT_TEST_SUITE_REGISTRATION(MagnetTest);

void MagnetTest::testParse()
{
  BDE r = parse
    ("magnet:?xt=urn:btih:248d0a1cd08284299de78d5c1ed359bb46717d8c&dn=aria2"
     "&tr=http%3A%2F%2Ftracker1&tr=http://tracker2");
  CPPUNIT_ASSERT_EQUAL
    (std::string("urn:btih:248d0a1cd08284299de78d5c1ed359bb46717d8c"),
     r["xt"][0].s());
  CPPUNIT_ASSERT_EQUAL(std::string("aria2"), r["dn"][0].s());
  CPPUNIT_ASSERT_EQUAL(std::string("http://tracker1"), r["tr"][0].s());
  CPPUNIT_ASSERT_EQUAL(std::string("http://tracker2"), r["tr"][1].s());

  CPPUNIT_ASSERT(parse("http://localhost").isNone());
}

} // namespace magnet

} // namespace aria2
