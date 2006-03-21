#include "Dictionary.h"
#include "Data.h"
#include <string>
#include <cppunit/extensions/HelperMacros.h>

using namespace std;

class DictionaryTest:public CppUnit::TestFixture {

  CPPUNIT_TEST_SUITE(DictionaryTest);
  CPPUNIT_TEST(testGet);
  CPPUNIT_TEST_SUITE_END();
private:

public:
  void setUp() {
  }

  void testGet();
};


CPPUNIT_TEST_SUITE_REGISTRATION( DictionaryTest );

void DictionaryTest::testGet() {
  Dictionary d;
  Data* data1 = new Data("aria2", 5);
  d.put("app_name", data1);
  Data* data2 = new Data("linux", 5);
  d.put("platform", data2);
  Data* dataGot = (Data*)d.get("app_name");
  CPPUNIT_ASSERT(dataGot != NULL);
  CPPUNIT_ASSERT_EQUAL(string("aria2"), dataGot->toString());
}

