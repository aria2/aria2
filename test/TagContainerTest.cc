#include "TagContainer.h"
#include "TaggedItem.h"
#include <cppunit/extensions/HelperMacros.h>

namespace aria2 {

class TagContainerTest:public CppUnit::TestFixture {

  CPPUNIT_TEST_SUITE(TagContainerTest);
  CPPUNIT_TEST(testSearch);
  CPPUNIT_TEST(testNameMatch);
  CPPUNIT_TEST_SUITE_END();
private:
public:
  void setUp() {}

  void testSearch();
  void testNameMatch();
};


CPPUNIT_TEST_SUITE_REGISTRATION(TagContainerTest);

void TagContainerTest::testSearch()
{
  TaggedItemHandle items[] = {
    SharedHandle<TaggedItem>(new TaggedItem("alpha")),
    SharedHandle<TaggedItem>(new TaggedItem("bravo")),
    SharedHandle<TaggedItem>(new TaggedItem("charlie"))
  };
  items[0]->addTag("foo");
  items[1]->addTag("foo");
  items[1]->addTag("bar");
  items[2]->addTag("foo");

  TagContainer tc(TaggedItems(&items[0], &items[3]));
  
  {
    TaggedItems res = tc.search("bar");
    CPPUNIT_ASSERT_EQUAL((size_t)1, res.size());
    CPPUNIT_ASSERT_EQUAL(std::string("bravo"), res[0]->getName());
    CPPUNIT_ASSERT_EQUAL(std::string("foo,bar"), res[0]->toTagString());
  }
  {
    TaggedItems res = tc.nameMatchForward("ch");
    CPPUNIT_ASSERT_EQUAL((size_t)1, res.size());
    CPPUNIT_ASSERT_EQUAL(std::string("charlie"), res[0]->getName());
    CPPUNIT_ASSERT_EQUAL(std::string("foo"), res[0]->toTagString());
  }
}

void TagContainerTest::testNameMatch()
{
  TaggedItemHandle items[] = {
    SharedHandle<TaggedItem>(new TaggedItem("alpha")),
    SharedHandle<TaggedItem>(new TaggedItem("bravo")),
    SharedHandle<TaggedItem>(new TaggedItem("charlie")),
    SharedHandle<TaggedItem>(new TaggedItem("bravo"))
  };
  items[1]->addTag("foo");
  TagContainer tc(TaggedItems(&items[0], &items[3]));
  {
    TaggedItemHandle item = tc.nameMatch("bravo");
    CPPUNIT_ASSERT_EQUAL(std::string("bravo"), item->getName());
    CPPUNIT_ASSERT_EQUAL(std::string("foo"), item->toTagString());
  }
  {
    CPPUNIT_ASSERT(tc.nameMatch("delta").isNull());
  }
}

} // namespace aria2
