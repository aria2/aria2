#include "HelpItem.h"
#include <sstream>
#include <cppunit/extensions/HelperMacros.h>

namespace aria2 {

class HelpItemTest:public CppUnit::TestFixture {

  CPPUNIT_TEST_SUITE(HelpItemTest);
  CPPUNIT_TEST(testOperatorStreamOut);
  CPPUNIT_TEST_SUITE_END();
private:

public:
  void setUp() {}

  void testOperatorStreamOut();
};


CPPUNIT_TEST_SUITE_REGISTRATION(HelpItemTest);

void HelpItemTest::testOperatorStreamOut()
{
  std::string usage =
    " -m, --max-tries=N            Set number of tries. 0 means unlimited.";
  HelpItem item("max-tries", usage, "5");
  item.setAvailableValues("0,5,10");
  item.addTag("basic");
  item.addTag("http");
  item.addTag("ftp");
  
  std::stringstream s;
  s << item;

  CPPUNIT_ASSERT_EQUAL(usage+"\n"+
		       "                              Available Values: 0,5,10\n"
		       "                              Default: 5\n"
		       "                              Tags: basic,http,ftp",
		       s.str());
}

} // namespace aria2
