#include "OptionParser.h"
#include "OptionHandlerImpl.h"
#include "Exception.h"
#include "Util.h"
#include "Option.h"
#include <iostream>
#include <cppunit/extensions/HelperMacros.h>

namespace aria2 {

class OptionParserTest:public CppUnit::TestFixture {

  CPPUNIT_TEST_SUITE(OptionParserTest);
  CPPUNIT_TEST(testFindAll);
  CPPUNIT_TEST(testFindByNameSubstring);
  CPPUNIT_TEST(testFindByTag);
  CPPUNIT_TEST(testFindByName);
  CPPUNIT_TEST(testParseDefaultValues);
  CPPUNIT_TEST_SUITE_END();
private:
  SharedHandle<OptionParser> _oparser;
public:
  void setUp()
  {
    _oparser.reset(new OptionParser());

    SharedHandle<OptionHandler> alpha(new DefaultOptionHandler("alpha",
							       "",
							       "ALPHA"));
    alpha->addTag("apple");
    _oparser->addOptionHandler(alpha);
    SharedHandle<OptionHandler> bravo(new DefaultOptionHandler("bravo"));
    bravo->addTag("apple");
    bravo->addTag("orange");
    bravo->addTag("pineapple");
    _oparser->addOptionHandler(bravo);
    SharedHandle<OptionHandler> charlie(new DefaultOptionHandler("charlie",
								 "",
								 "CHARLIE",
								 "",
								 true));
    charlie->addTag("pineapple");
    _oparser->addOptionHandler(charlie);
    SharedHandle<OptionHandler> delta(new UnitNumberOptionHandler("delta",
								  "",
								  "1M"));
    delta->addTag("pineapple");
    _oparser->addOptionHandler(delta);    
  }

  void tearDown() {}

  void testFindAll();
  void testFindByNameSubstring();
  void testFindByTag();
  void testFindByName();
  void testParseDefaultValues();
};


CPPUNIT_TEST_SUITE_REGISTRATION(OptionParserTest);

void OptionParserTest::testFindAll()
{
  std::deque<SharedHandle<OptionHandler> > res = _oparser->findAll();
  CPPUNIT_ASSERT_EQUAL((size_t)3, res.size());
  CPPUNIT_ASSERT_EQUAL(std::string("alpha"), res[0]->getName());
  CPPUNIT_ASSERT_EQUAL(std::string("bravo"), res[1]->getName());
  CPPUNIT_ASSERT_EQUAL(std::string("delta"), res[2]->getName());
}

void OptionParserTest::testFindByNameSubstring()
{
  std::deque<SharedHandle<OptionHandler> > res =
    _oparser->findByNameSubstring("l");
  CPPUNIT_ASSERT_EQUAL((size_t)2, res.size());
  CPPUNIT_ASSERT_EQUAL(std::string("alpha"), res[0]->getName());
  CPPUNIT_ASSERT_EQUAL(std::string("delta"), res[1]->getName());
}

void OptionParserTest::testFindByTag()
{
  std::deque<SharedHandle<OptionHandler> > res =
    _oparser->findByTag("pineapple");
  CPPUNIT_ASSERT_EQUAL((size_t)2, res.size());
  CPPUNIT_ASSERT_EQUAL(std::string("bravo"), res[0]->getName());
  CPPUNIT_ASSERT_EQUAL(std::string("delta"), res[1]->getName());
}

void OptionParserTest::testFindByName()
{
  SharedHandle<OptionHandler> bravo = _oparser->findByName("bravo");
  CPPUNIT_ASSERT(!bravo.isNull());
  CPPUNIT_ASSERT_EQUAL(std::string("bravo"), bravo->getName());

  SharedHandle<OptionHandler> charlie = _oparser->findByName("charlie");
  CPPUNIT_ASSERT(charlie.isNull());
}

void OptionParserTest::testParseDefaultValues()
{
  Option option;
  _oparser->parseDefaultValues(&option);
  CPPUNIT_ASSERT_EQUAL(std::string("ALPHA"), option.get("alpha"));
  CPPUNIT_ASSERT_EQUAL(std::string("1048576"), option.get("delta"));
  CPPUNIT_ASSERT_EQUAL(std::string("CHARLIE"), option.get("charlie"));
  CPPUNIT_ASSERT(!option.defined("bravo"));
}

} // namespace aria2
