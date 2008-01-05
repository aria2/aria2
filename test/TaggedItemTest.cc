#include "TaggedItem.h"
#include <cppunit/extensions/HelperMacros.h>

class TaggedItemTest:public CppUnit::TestFixture {

  CPPUNIT_TEST_SUITE(TaggedItemTest);
  CPPUNIT_TEST(testHasTag);
  CPPUNIT_TEST(testToTagString);
  CPPUNIT_TEST_SUITE_END();
private:

public:
  void setUp() {}

  void testHasTag();
  void testToTagString();
};


CPPUNIT_TEST_SUITE_REGISTRATION(TaggedItemTest);

void TaggedItemTest::testHasTag()
{
  TaggedItem item("alpha");
  item.addTag("foo");
  item.addTag("bar");

  CPPUNIT_ASSERT(item.hasTag("bar"));
  CPPUNIT_ASSERT(!item.hasTag("boo"));
}

void TaggedItemTest::testToTagString()
{
  TaggedItem item("alpha");
  item.addTag("foo");
  item.addTag("bar");

  CPPUNIT_ASSERT_EQUAL(string("foo,bar"), item.toTagString());
}
