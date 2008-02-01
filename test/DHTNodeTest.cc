#include "DHTNode.h"
#include "Exception.h"
#include "Util.h"
#include <cppunit/extensions/HelperMacros.h>

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
  cerr << Util::toHex(node.getID(), DHT_ID_LENGTH) << endl;
}
