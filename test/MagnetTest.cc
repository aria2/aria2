#include "magnet.h"

#include <iostream>

#include <cppunit/extensions/HelperMacros.h>

namespace aria2 {

namespace magnet {

class MagnetTest : public CppUnit::TestFixture {

  CPPUNIT_TEST_SUITE(MagnetTest);
  CPPUNIT_TEST(testParse);
  CPPUNIT_TEST_SUITE_END();

public:
  void testParse();
};

CPPUNIT_TEST_SUITE_REGISTRATION(MagnetTest);

namespace {
const std::string& nthStr(const ValueBase* v, size_t index)
{
  return downcast<String>(downcast<List>(v)->get(index))->s();
}
} // namespace

void MagnetTest::testParse()
{
  auto r = parse(
      "magnet:?xt=urn:btih:248d0a1cd08284299de78d5c1ed359bb46717d8c&dn=aria2"
      "&tr=http%3A%2F%2Ftracker1&tr=http://tracker2");
  CPPUNIT_ASSERT_EQUAL(
      std::string("urn:btih:248d0a1cd08284299de78d5c1ed359bb46717d8c"),
      nthStr(r->get("xt"), 0));
  CPPUNIT_ASSERT_EQUAL(std::string("aria2"), nthStr(r->get("dn"), 0));
  CPPUNIT_ASSERT_EQUAL(std::string("http://tracker1"), nthStr(r->get("tr"), 0));
  CPPUNIT_ASSERT_EQUAL(std::string("http://tracker2"), nthStr(r->get("tr"), 1));
  CPPUNIT_ASSERT(!parse("http://localhost"));
}

} // namespace magnet

} // namespace aria2
