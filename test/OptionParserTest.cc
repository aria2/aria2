#include "OptionParser.h"

#include <cstring>
#include <sstream>

#include <cppunit/extensions/HelperMacros.h>

#include "OptionHandlerImpl.h"
#include "Exception.h"
#include "util.h"
#include "Option.h"
#include "array_fun.h"
#include "prefs.h"
#include "help_tags.h"

namespace aria2 {

class OptionParserTest : public CppUnit::TestFixture {

  CPPUNIT_TEST_SUITE(OptionParserTest);
  CPPUNIT_TEST(testFindAll);
  CPPUNIT_TEST(testFindByNameSubstring);
  CPPUNIT_TEST(testFindByTag);
  CPPUNIT_TEST(testFind);
  CPPUNIT_TEST(testFindByShortName);
  CPPUNIT_TEST(testFindById);
  CPPUNIT_TEST(testParseDefaultValues);
  CPPUNIT_TEST(testParseArg);
  CPPUNIT_TEST(testParse);
  CPPUNIT_TEST(testParseKeyVals);
  CPPUNIT_TEST_SUITE_END();

private:
  std::shared_ptr<OptionParser> oparser_;

public:
  void setUp()
  {
    oparser_.reset(new OptionParser());

    OptionHandler* timeout(
        new DefaultOptionHandler(PREF_TIMEOUT, NO_DESCRIPTION, "ALPHA", "",
                                 OptionHandler::REQ_ARG, 'A'));
    timeout->addTag(TAG_BASIC);
    timeout->setEraseAfterParse(true);
    oparser_->addOptionHandler(timeout);

    OptionHandler* dir(new DefaultOptionHandler(PREF_DIR));
    dir->addTag(TAG_BASIC);
    dir->addTag(TAG_HTTP);
    dir->addTag(TAG_FILE);
    oparser_->addOptionHandler(dir);

    DefaultOptionHandler* daemon(
        new DefaultOptionHandler(PREF_DAEMON, NO_DESCRIPTION, "CHARLIE", "",
                                 OptionHandler::REQ_ARG, 'C'));
    daemon->hide();
    daemon->addTag(TAG_FILE);
    oparser_->addOptionHandler(daemon);

    OptionHandler* out(new UnitNumberOptionHandler(PREF_OUT, NO_DESCRIPTION,
                                                   "1M", -1, -1, 'D'));
    out->addTag(TAG_FILE);
    oparser_->addOptionHandler(out);
  }

  void tearDown() {}

  void testFindAll();
  void testFindByNameSubstring();
  void testFindByTag();
  void testFind();
  void testFindByShortName();
  void testFindById();
  void testParseDefaultValues();
  void testParseArg();
  void testParse();
  void testParseKeyVals();
};

CPPUNIT_TEST_SUITE_REGISTRATION(OptionParserTest);

void OptionParserTest::testFindAll()
{
  std::vector<const OptionHandler*> res = oparser_->findAll();
  CPPUNIT_ASSERT_EQUAL((size_t)3, res.size());
  CPPUNIT_ASSERT_EQUAL(std::string("timeout"), std::string(res[0]->getName()));
  CPPUNIT_ASSERT_EQUAL(std::string("dir"), std::string(res[1]->getName()));
  CPPUNIT_ASSERT_EQUAL(std::string("out"), std::string(res[2]->getName()));
}

void OptionParserTest::testFindByNameSubstring()
{
  std::vector<const OptionHandler*> res = oparser_->findByNameSubstring("i");
  CPPUNIT_ASSERT_EQUAL((size_t)2, res.size());
  CPPUNIT_ASSERT_EQUAL(std::string("timeout"), std::string(res[0]->getName()));
  CPPUNIT_ASSERT_EQUAL(std::string("dir"), std::string(res[1]->getName()));
}

void OptionParserTest::testFindByTag()
{
  std::vector<const OptionHandler*> res = oparser_->findByTag(TAG_FILE);
  CPPUNIT_ASSERT_EQUAL((size_t)2, res.size());
  CPPUNIT_ASSERT_EQUAL(std::string("dir"), std::string(res[0]->getName()));
  CPPUNIT_ASSERT_EQUAL(std::string("out"), std::string(res[1]->getName()));
}

void OptionParserTest::testFind()
{
  const OptionHandler* dir = oparser_->find(PREF_DIR);
  CPPUNIT_ASSERT(dir);
  CPPUNIT_ASSERT_EQUAL(std::string("dir"), std::string(dir->getName()));

  const OptionHandler* daemon = oparser_->find(PREF_DAEMON);
  CPPUNIT_ASSERT(!daemon);

  const OptionHandler* log = oparser_->find(PREF_LOG);
  CPPUNIT_ASSERT(!log);
}

void OptionParserTest::testFindByShortName()
{
  const OptionHandler* timeout = oparser_->findByShortName('A');
  CPPUNIT_ASSERT(timeout);
  CPPUNIT_ASSERT_EQUAL(std::string("timeout"), std::string(timeout->getName()));

  CPPUNIT_ASSERT(!oparser_->findByShortName('C'));
}

void OptionParserTest::testFindById()
{
  const OptionHandler* timeout = oparser_->findById(PREF_TIMEOUT->i);
  CPPUNIT_ASSERT(timeout);
  CPPUNIT_ASSERT_EQUAL(std::string("timeout"), std::string(timeout->getName()));

  CPPUNIT_ASSERT(!oparser_->findById(9999));
}

void OptionParserTest::testParseDefaultValues()
{
  Option option;
  oparser_->parseDefaultValues(option);
  CPPUNIT_ASSERT_EQUAL(std::string("ALPHA"), option.get(PREF_TIMEOUT));
  CPPUNIT_ASSERT_EQUAL(std::string("1048576"), option.get(PREF_OUT));
  CPPUNIT_ASSERT_EQUAL(std::string("CHARLIE"), option.get(PREF_DAEMON));
  CPPUNIT_ASSERT(!option.defined(PREF_DIR));
}

void OptionParserTest::testParseArg()
{
  Option option;
  char prog[7];
  strncpy(prog, "aria2c", sizeof(prog));

  char optionTimeout[3];
  strncpy(optionTimeout, "-A", sizeof(optionTimeout));
  char argTimeout[6];
  strncpy(argTimeout, "ALPHA", sizeof(argTimeout));
  char optionDir[8];
  strncpy(optionDir, "--dir", sizeof(optionDir));
  char argDir[6];
  strncpy(argDir, "BRAVO", sizeof(argDir));

  char nonopt1[8];
  strncpy(nonopt1, "nonopt1", sizeof(nonopt1));
  char nonopt2[8];
  strncpy(nonopt2, "nonopt2", sizeof(nonopt2));

  char* argv[] = {prog,   optionTimeout, argTimeout, optionDir,
                  argDir, nonopt1,       nonopt2};
  int argc = arraySize(argv);

  std::stringstream s;
  std::vector<std::string> nonopts;

  oparser_->parseArg(s, nonopts, argc, argv);

  CPPUNIT_ASSERT_EQUAL(std::string("timeout=ALPHA\n"
                                   "dir=BRAVO\n"),
                       s.str());

  CPPUNIT_ASSERT_EQUAL((size_t)2, nonopts.size());
  CPPUNIT_ASSERT_EQUAL(std::string("nonopt1"), nonopts[0]);
  CPPUNIT_ASSERT_EQUAL(std::string("nonopt2"), nonopts[1]);

  CPPUNIT_ASSERT_EQUAL(std::string("*****"), std::string(argTimeout));
}

void OptionParserTest::testParse()
{
  Option option;
  std::istringstream in("timeout=Hello\n"
                        "UNKNOWN=x\n"
                        "\n"
                        "dir=World");
  oparser_->parse(option, in);
  CPPUNIT_ASSERT_EQUAL(std::string("Hello"), option.get(PREF_TIMEOUT));
  CPPUNIT_ASSERT_EQUAL(std::string("World"), option.get(PREF_DIR));
}

void OptionParserTest::testParseKeyVals()
{
  Option option;
  KeyVals kv;
  kv.push_back(std::make_pair("timeout", "Hello"));
  kv.push_back(std::make_pair("UNKNOWN", "x"));
  kv.push_back(std::make_pair("dir", "World"));
  oparser_->parse(option, kv);
  CPPUNIT_ASSERT_EQUAL(std::string("Hello"), option.get(PREF_TIMEOUT));
  CPPUNIT_ASSERT_EQUAL(std::string("World"), option.get(PREF_DIR));
}

} // namespace aria2
