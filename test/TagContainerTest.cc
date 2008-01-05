#include "TagContainer.h"
#include "TaggedItem.h"
#include <cppunit/extensions/HelperMacros.h>

using namespace std;

class TagContainerTest:public CppUnit::TestFixture {

  CPPUNIT_TEST_SUITE(TagContainerTest);
  CPPUNIT_TEST(testSearch);
  CPPUNIT_TEST_SUITE_END();
private:

public:
  void setUp() {}

  void testSearch();
};


CPPUNIT_TEST_SUITE_REGISTRATION(TagContainerTest);

void TagContainerTest::testSearch()
{
  TaggedItemHandle items[] = {
    new TaggedItem("alpha"),
    new TaggedItem("bravo"),
    new TaggedItem("charlie"),
  };
  items[0]->addTag("foo");
  items[1]->addTag("foo");
  items[1]->addTag("bar");
  items[2]->addTag("foo");

  TagContainer tc(TaggedItems(&items[0], &items[3]));
  
  {
    TaggedItems res = tc.search("bar");
    CPPUNIT_ASSERT_EQUAL((size_t)1, res.size());
    CPPUNIT_ASSERT_EQUAL(string("bravo"), res[0]->getName());
    CPPUNIT_ASSERT_EQUAL(string("foo,bar"), res[0]->toTagString());
  }
  {
    TaggedItems res = tc.nameMatchForward("ch");
    CPPUNIT_ASSERT_EQUAL((size_t)1, res.size());
    CPPUNIT_ASSERT_EQUAL(string("charlie"), res[0]->getName());
    CPPUNIT_ASSERT_EQUAL(string("foo"), res[0]->toTagString());
  }
}
