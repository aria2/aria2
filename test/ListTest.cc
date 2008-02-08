#include "List.h"
#include "Data.h"
#include <string>
#include <cppunit/extensions/HelperMacros.h>

namespace aria2 {

class ListTest:public CppUnit::TestFixture {

  CPPUNIT_TEST_SUITE(ListTest);
  CPPUNIT_TEST(testAdd);
  CPPUNIT_TEST_SUITE_END();
private:

public:
  void setUp() {
  }

  void testAdd();
};


CPPUNIT_TEST_SUITE_REGISTRATION( ListTest );

void ListTest::testAdd() {
  List l;
  Data* data1 = new Data("usr", 3);
  l.add(data1);
  Data* data2 = new Data("local", 5);
  l.add(data2);
  CPPUNIT_ASSERT_EQUAL(2, (int)l.getList().size());
}

} // namespace aria2
