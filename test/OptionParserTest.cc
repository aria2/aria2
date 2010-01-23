#include "OptionParser.h"

#include <cstring>
#include <sstream>

#include <cppunit/extensions/HelperMacros.h>

#include "OptionHandlerImpl.h"
#include "Exception.h"
#include "util.h"
#include "Option.h"
#include "array_fun.h"

namespace aria2 {

class OptionParserTest:public CppUnit::TestFixture {

  CPPUNIT_TEST_SUITE(OptionParserTest);
  CPPUNIT_TEST(testFindAll);
  CPPUNIT_TEST(testFindByNameSubstring);
  CPPUNIT_TEST(testFindByTag);
  CPPUNIT_TEST(testFindByName);
  CPPUNIT_TEST(testFindByShortName);
  CPPUNIT_TEST(testFindByID);
  CPPUNIT_TEST(testParseDefaultValues);
  CPPUNIT_TEST(testParseArg);
  CPPUNIT_TEST(testParse);
  CPPUNIT_TEST_SUITE_END();
private:
  SharedHandle<OptionParser> _oparser;
public:
  void setUp()
  {
    _oparser.reset(new OptionParser());

    SharedHandle<OptionHandler> alpha
      (new DefaultOptionHandler("alpha", NO_DESCRIPTION, "ALPHA", "",
                                OptionHandler::REQ_ARG, 'A'));
    alpha->addTag("apple");
    _oparser->addOptionHandler(alpha);

    SharedHandle<OptionHandler> bravo(new DefaultOptionHandler("bravo"));
    bravo->addTag("apple");
    bravo->addTag("orange");
    bravo->addTag("pineapple");
    _oparser->addOptionHandler(bravo);

    SharedHandle<DefaultOptionHandler> charlie
      (new DefaultOptionHandler("charlie", NO_DESCRIPTION, "CHARLIE", "",
                                OptionHandler::REQ_ARG, 'C'));
    charlie->hide();
    charlie->addTag("pineapple");
    _oparser->addOptionHandler(charlie);

    SharedHandle<OptionHandler> delta
      (new UnitNumberOptionHandler("delta", NO_DESCRIPTION, "1M", -1, -1, 'D'));
    delta->addTag("pineapple");
    _oparser->addOptionHandler(delta);    
  }

  void tearDown() {}

  void testFindAll();
  void testFindByNameSubstring();
  void testFindByTag();
  void testFindByName();
  void testFindByShortName();
  void testFindByID();
  void testParseDefaultValues();
  void testParseArg();
  void testParse();
};


CPPUNIT_TEST_SUITE_REGISTRATION(OptionParserTest);

void OptionParserTest::testFindAll()
{
  std::vector<SharedHandle<OptionHandler> > res = _oparser->findAll();
  CPPUNIT_ASSERT_EQUAL((size_t)3, res.size());
  CPPUNIT_ASSERT_EQUAL(std::string("alpha"), res[0]->getName());
  CPPUNIT_ASSERT_EQUAL(std::string("bravo"), res[1]->getName());
  CPPUNIT_ASSERT_EQUAL(std::string("delta"), res[2]->getName());
}

void OptionParserTest::testFindByNameSubstring()
{
  std::vector<SharedHandle<OptionHandler> > res =
    _oparser->findByNameSubstring("l");
  CPPUNIT_ASSERT_EQUAL((size_t)2, res.size());
  CPPUNIT_ASSERT_EQUAL(std::string("alpha"), res[0]->getName());
  CPPUNIT_ASSERT_EQUAL(std::string("delta"), res[1]->getName());
}

void OptionParserTest::testFindByTag()
{
  std::vector<SharedHandle<OptionHandler> > res =
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

  SharedHandle<OptionHandler> alpha2 = _oparser->findByName("alpha2");
  CPPUNIT_ASSERT(alpha2.isNull());
}

void OptionParserTest::testFindByShortName()
{
  SharedHandle<OptionHandler> alpha = _oparser->findByShortName('A');
  CPPUNIT_ASSERT(!alpha.isNull());
  CPPUNIT_ASSERT_EQUAL(std::string("alpha"), alpha->getName());

  CPPUNIT_ASSERT(_oparser->findByShortName('C').isNull());
}

void OptionParserTest::testFindByID()
{
  SharedHandle<OptionHandler> alpha = _oparser->findByID(1);
  CPPUNIT_ASSERT(!alpha.isNull());
  CPPUNIT_ASSERT_EQUAL(std::string("alpha"), alpha->getName());

  CPPUNIT_ASSERT(_oparser->findByID(3).isNull());
}

void OptionParserTest::testParseDefaultValues()
{
  Option option;
  _oparser->parseDefaultValues(option);
  CPPUNIT_ASSERT_EQUAL(std::string("ALPHA"), option.get("alpha"));
  CPPUNIT_ASSERT_EQUAL(std::string("1048576"), option.get("delta"));
  CPPUNIT_ASSERT_EQUAL(std::string("CHARLIE"), option.get("charlie"));
  CPPUNIT_ASSERT(!option.defined("bravo"));
}

void OptionParserTest::testParseArg()
{
  Option option;
  char prog[7];
  strncpy(prog, "aria2c", sizeof(prog));

  char optionAlpha[3];
  strncpy(optionAlpha, "-A", sizeof(optionAlpha));
  char argAlpha[6];
  strncpy(argAlpha, "ALPHA", sizeof(argAlpha));
  char optionBravo[8];
  strncpy(optionBravo, "--bravo", sizeof(optionBravo));
  char argBravo[6];
  strncpy(argBravo, "BRAVO", sizeof(argBravo));

  char nonopt1[8];
  strncpy(nonopt1, "nonopt1", sizeof(nonopt1));
  char nonopt2[8];
  strncpy(nonopt2, "nonopt2", sizeof(nonopt2));

  char* const argv[] = { prog, optionAlpha, argAlpha, optionBravo, argBravo,
                         nonopt1, nonopt2 };
  int argc = arrayLength(argv);

  std::stringstream s;
  std::deque<std::string> nonopts;

  _oparser->parseArg(s, nonopts, argc, argv);

  CPPUNIT_ASSERT_EQUAL(std::string("alpha=ALPHA\n"
                                   "bravo=BRAVO\n"), s.str());

  CPPUNIT_ASSERT_EQUAL((size_t)2, nonopts.size());
  CPPUNIT_ASSERT_EQUAL(std::string("nonopt1"), nonopts[0]);
  CPPUNIT_ASSERT_EQUAL(std::string("nonopt2"), nonopts[1]);
}

void OptionParserTest::testParse()
{
  Option option;
  std::istringstream in("alpha=Hello\n"
                        "UNKNOWN=x\n"
                        "\n"
                        "bravo=World");
  _oparser->parse(option, in);
  CPPUNIT_ASSERT_EQUAL
    ((ptrdiff_t)2, std::distance(option.begin(), option.end()));
  CPPUNIT_ASSERT_EQUAL(std::string("Hello"), option.get("alpha"));
  CPPUNIT_ASSERT_EQUAL(std::string("World"), option.get("bravo"));
}

} // namespace aria2
