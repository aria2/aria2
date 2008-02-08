#include "TaggedItem.h"
#include <cppunit/extensions/HelperMacros.h>

namespace aria2 {

class TaggedItemTest:public CppUnit::TestFixture {

  CPPUNIT_TEST_SUITE(TaggedItemTest);
  CPPUNIT_TEST(testHasTag);
  CPPUNIT_TEST(testToTagString);
  CPPUNIT_TEST(testOperatorEqual);
  CPPUNIT_TEST_SUITE_END();
private:

public:
  void setUp() {}

  void testHasTag();
  void testToTagString();
  void testOperatorEqual();
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

  CPPUNIT_ASSERT_EQUAL(std::string("foo,bar"), item.toTagString());
}

void TaggedItemTest::testOperatorEqual()
{
  TaggedItem none("");
  TaggedItem foo("foo");
  TaggedItem foo2("foo");
  TaggedItem bar("bar");
  CPPUNIT_ASSERT(!(none == foo));
  CPPUNIT_ASSERT(!(bar == foo));
  CPPUNIT_ASSERT(foo == foo);
}

} // namespace aria2
