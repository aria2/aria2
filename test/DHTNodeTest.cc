#include "DHTNode.h"
#include "Exception.h"
#include "util.h"
#include <iostream>
#include <cppunit/extensions/HelperMacros.h>

namespace aria2 {

class DHTNodeTest:public CppUnit::TestFixture {

  CPPUNIT_TEST_SUITE(DHTNodeTest);
  CPPUNIT_TEST(testGenerateID);
  CPPUNIT_TEST_SUITE_END();
public:
  void setUp() {}

  void tearDown() {}

  void testGenerateID();
};


CPPUNIT_TEST_SUITE_REGISTRATION(DHTNodeTest);

void DHTNodeTest::testGenerateID()
{
  DHTNode node;
  std::cerr << util::toHex(node.getID(), DHT_ID_LENGTH) << std::endl;
}

} // namespace aria2
