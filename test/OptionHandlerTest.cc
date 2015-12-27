#include "OptionHandlerImpl.h"

#include <cppunit/extensions/HelperMacros.h>

#include "Option.h"
#include "prefs.h"
#include "Exception.h"
#include "help_tags.h"

namespace aria2 {

class OptionHandlerTest : public CppUnit::TestFixture {

  CPPUNIT_TEST_SUITE(OptionHandlerTest);
  CPPUNIT_TEST(testBooleanOptionHandler);
  CPPUNIT_TEST(testNumberOptionHandler);
  CPPUNIT_TEST(testNumberOptionHandler_min);
  CPPUNIT_TEST(testNumberOptionHandler_max);
  CPPUNIT_TEST(testNumberOptionHandler_min_max);
  CPPUNIT_TEST(testUnitNumberOptionHandler);
  CPPUNIT_TEST(testParameterOptionHandler);
  CPPUNIT_TEST(testDefaultOptionHandler);
  CPPUNIT_TEST(testFloatNumberOptionHandler);
  CPPUNIT_TEST(testFloatNumberOptionHandler_min);
  CPPUNIT_TEST(testFloatNumberOptionHandler_max);
  CPPUNIT_TEST(testFloatNumberOptionHandler_min_max);
  CPPUNIT_TEST(testHttpProxyOptionHandler);
  CPPUNIT_TEST(testDeprecatedOptionHandler);
  CPPUNIT_TEST_SUITE_END();

public:
  void testBooleanOptionHandler();
  void testNumberOptionHandler();
  void testNumberOptionHandler_min();
  void testNumberOptionHandler_max();
  void testNumberOptionHandler_min_max();
  void testUnitNumberOptionHandler();
  void testParameterOptionHandler();
  void testDefaultOptionHandler();
  void testFloatNumberOptionHandler();
  void testFloatNumberOptionHandler_min();
  void testFloatNumberOptionHandler_max();
  void testFloatNumberOptionHandler_min_max();
  void testHttpProxyOptionHandler();
  void testDeprecatedOptionHandler();
};

CPPUNIT_TEST_SUITE_REGISTRATION(OptionHandlerTest);

void OptionHandlerTest::testBooleanOptionHandler()
{
  BooleanOptionHandler handler(PREF_DAEMON);
  Option option;
  handler.parse(option, A2_V_TRUE);
  CPPUNIT_ASSERT_EQUAL(std::string(A2_V_TRUE), option.get(PREF_DAEMON));
  handler.parse(option, A2_V_FALSE);
  CPPUNIT_ASSERT_EQUAL(std::string(A2_V_FALSE), option.get(PREF_DAEMON));
  try {
    handler.parse(option, "hello");
    CPPUNIT_FAIL("exception must be thrown.");
  }
  catch (Exception& e) {
  }
  CPPUNIT_ASSERT_EQUAL(std::string("true, false"),
                       handler.createPossibleValuesString());
}

void OptionHandlerTest::testNumberOptionHandler()
{
  NumberOptionHandler handler(PREF_TIMEOUT);
  Option option;
  handler.parse(option, "0");
  CPPUNIT_ASSERT_EQUAL(std::string("0"), option.get(PREF_TIMEOUT));
  CPPUNIT_ASSERT_EQUAL(std::string("*-*"),
                       handler.createPossibleValuesString());
}

void OptionHandlerTest::testNumberOptionHandler_min()
{
  NumberOptionHandler handler(PREF_TIMEOUT, "", "", 1);
  Option option;
  handler.parse(option, "1");
  CPPUNIT_ASSERT_EQUAL(std::string("1"), option.get(PREF_TIMEOUT));
  try {
    handler.parse(option, "0");
    CPPUNIT_FAIL("exception must be thrown.");
  }
  catch (Exception& e) {
  }
  CPPUNIT_ASSERT_EQUAL(std::string("1-*"),
                       handler.createPossibleValuesString());
}

void OptionHandlerTest::testNumberOptionHandler_max()
{
  NumberOptionHandler handler(PREF_TIMEOUT, "", "", -1, 100);
  Option option;
  handler.parse(option, "100");
  CPPUNIT_ASSERT_EQUAL(std::string("100"), option.get(PREF_TIMEOUT));
  try {
    handler.parse(option, "101");
    CPPUNIT_FAIL("exception must be thrown.");
  }
  catch (Exception& e) {
  }
  CPPUNIT_ASSERT_EQUAL(std::string("*-100"),
                       handler.createPossibleValuesString());
}

void OptionHandlerTest::testNumberOptionHandler_min_max()
{
  NumberOptionHandler handler(PREF_TIMEOUT, "", "", 1, 100);
  Option option;
  handler.parse(option, "1");
  CPPUNIT_ASSERT_EQUAL(std::string("1"), option.get(PREF_TIMEOUT));
  handler.parse(option, "100");
  CPPUNIT_ASSERT_EQUAL(std::string("100"), option.get(PREF_TIMEOUT));
  try {
    handler.parse(option, "0");
    CPPUNIT_FAIL("exception must be thrown.");
  }
  catch (Exception& e) {
  }
  try {
    handler.parse(option, "101");
    CPPUNIT_FAIL("exception must be thrown.");
  }
  catch (Exception& e) {
  }
  CPPUNIT_ASSERT_EQUAL(std::string("1-100"),
                       handler.createPossibleValuesString());
}

void OptionHandlerTest::testUnitNumberOptionHandler()
{
  UnitNumberOptionHandler handler(PREF_TIMEOUT);
  Option option;
  handler.parse(option, "4294967296");
  CPPUNIT_ASSERT_EQUAL(std::string("4294967296"), option.get(PREF_TIMEOUT));
  handler.parse(option, "4096M");
  CPPUNIT_ASSERT_EQUAL(std::string("4294967296"), option.get(PREF_TIMEOUT));
  handler.parse(option, "4096K");
  CPPUNIT_ASSERT_EQUAL(std::string("4194304"), option.get(PREF_TIMEOUT));
  try {
    handler.parse(option, "K");
    CPPUNIT_FAIL("exception must be thrown.");
  }
  catch (Exception& e) {
  }
  try {
    handler.parse(option, "M");
  }
  catch (Exception& e) {
  }
  try {
    handler.parse(option, "");
    CPPUNIT_FAIL("exception must be thrown.");
  }
  catch (Exception& e) {
  }
}

void OptionHandlerTest::testParameterOptionHandler()
{
  ParameterOptionHandler handler(PREF_TIMEOUT, "", "", {"value1", "value2"});
  Option option;
  handler.parse(option, "value1");
  CPPUNIT_ASSERT_EQUAL(std::string("value1"), option.get(PREF_TIMEOUT));
  handler.parse(option, "value2");
  CPPUNIT_ASSERT_EQUAL(std::string("value2"), option.get(PREF_TIMEOUT));
  try {
    handler.parse(option, "value3");
    CPPUNIT_FAIL("exception must be thrown.");
  }
  catch (Exception& e) {
  }
  CPPUNIT_ASSERT_EQUAL(std::string("value1, value2"),
                       handler.createPossibleValuesString());
}

void OptionHandlerTest::testDefaultOptionHandler()
{
  DefaultOptionHandler handler(PREF_TIMEOUT);
  Option option;
  handler.parse(option, "bar");
  CPPUNIT_ASSERT_EQUAL(std::string("bar"), option.get(PREF_TIMEOUT));
  handler.parse(option, "");
  CPPUNIT_ASSERT_EQUAL(std::string(""), option.get(PREF_TIMEOUT));
  CPPUNIT_ASSERT_EQUAL(std::string(""), handler.createPossibleValuesString());

  handler.addTag(TAG_ADVANCED);
  CPPUNIT_ASSERT_EQUAL(std::string("#advanced"), handler.toTagString());
  handler.addTag(TAG_BASIC);
  CPPUNIT_ASSERT_EQUAL(std::string("#basic, #advanced"), handler.toTagString());
  CPPUNIT_ASSERT(handler.hasTag(TAG_ADVANCED));
  CPPUNIT_ASSERT(handler.hasTag(TAG_BASIC));
  CPPUNIT_ASSERT(!handler.hasTag(TAG_HTTP));
}

void OptionHandlerTest::testFloatNumberOptionHandler()
{
  FloatNumberOptionHandler handler(PREF_TIMEOUT);
  Option option;
  handler.parse(option, "1.0");
  CPPUNIT_ASSERT_EQUAL(std::string("1.0"), option.get(PREF_TIMEOUT));
  CPPUNIT_ASSERT_EQUAL(std::string("*-*"),
                       handler.createPossibleValuesString());
}

void OptionHandlerTest::testFloatNumberOptionHandler_min()
{
  FloatNumberOptionHandler handler(PREF_TIMEOUT, "", "", 0.0);
  Option option;
  handler.parse(option, "0.0");
  CPPUNIT_ASSERT_EQUAL(std::string("0.0"), option.get(PREF_TIMEOUT));
  try {
    handler.parse(option, "-0.1");
    CPPUNIT_FAIL("exception must be thrown.");
  }
  catch (Exception& e) {
  }
  CPPUNIT_ASSERT_EQUAL(std::string("0.0-*"),
                       handler.createPossibleValuesString());
}

void OptionHandlerTest::testFloatNumberOptionHandler_max()
{
  FloatNumberOptionHandler handler(PREF_TIMEOUT, "", "", -1, 10.0);
  Option option;
  handler.parse(option, "10.0");
  CPPUNIT_ASSERT_EQUAL(std::string("10.0"), option.get(PREF_TIMEOUT));
  try {
    handler.parse(option, "10.1");
    CPPUNIT_FAIL("exception must be thrown.");
  }
  catch (Exception& e) {
  }
  CPPUNIT_ASSERT_EQUAL(std::string("*-10.0"),
                       handler.createPossibleValuesString());
}

void OptionHandlerTest::testFloatNumberOptionHandler_min_max()
{
  FloatNumberOptionHandler handler(PREF_TIMEOUT, "", "", 0.0, 10.0);
  Option option;
  handler.parse(option, "0.0");
  CPPUNIT_ASSERT_EQUAL(std::string("0.0"), option.get(PREF_TIMEOUT));
  handler.parse(option, "10.0");
  CPPUNIT_ASSERT_EQUAL(std::string("10.0"), option.get(PREF_TIMEOUT));
  try {
    handler.parse(option, "-0.1");
    CPPUNIT_FAIL("exception must be thrown.");
  }
  catch (Exception& e) {
  }
  try {
    handler.parse(option, "10.1");
    CPPUNIT_FAIL("exception must be thrown.");
  }
  catch (Exception& e) {
  }
  CPPUNIT_ASSERT_EQUAL(std::string("0.0-10.0"),
                       handler.createPossibleValuesString());
}

void OptionHandlerTest::testHttpProxyOptionHandler()
{
  HttpProxyOptionHandler handler(PREF_HTTP_PROXY, "", "");
  Option option;
  handler.parse(option, "proxy:65535");
  CPPUNIT_ASSERT_EQUAL(std::string("http://proxy:65535/"),
                       option.get(PREF_HTTP_PROXY));

  handler.parse(option, "http://proxy:8080");
  CPPUNIT_ASSERT_EQUAL(std::string("http://proxy:8080/"),
                       option.get(PREF_HTTP_PROXY));

  handler.parse(option, "");
  CPPUNIT_ASSERT(option.defined(PREF_HTTP_PROXY));
  CPPUNIT_ASSERT(option.blank(PREF_HTTP_PROXY));

  try {
    handler.parse(option, "http://bar:65536");
    CPPUNIT_FAIL("exception must be thrown.");
  }
  catch (Exception& e) {
  }
  CPPUNIT_ASSERT_EQUAL(std::string("[http://][USER:PASSWORD@]HOST[:PORT]"),
                       handler.createPossibleValuesString());

  handler.parse(option, "http://user%40:passwd%40@proxy:8080");
  CPPUNIT_ASSERT_EQUAL(std::string("http://user%40:passwd%40@proxy:8080/"),
                       option.get(PREF_HTTP_PROXY));

  handler.parse(option, "http://[::1]:8080");
  CPPUNIT_ASSERT_EQUAL(std::string("http://[::1]:8080/"),
                       option.get(PREF_HTTP_PROXY));
}

void OptionHandlerTest::testDeprecatedOptionHandler()
{
  {
    DeprecatedOptionHandler handler(new DefaultOptionHandler(PREF_TIMEOUT));
    Option option;
    handler.parse(option, "foo");
    CPPUNIT_ASSERT(!option.defined(PREF_TIMEOUT));
  }
  {
    DefaultOptionHandler dir(PREF_DIR);
    DeprecatedOptionHandler handler(new DefaultOptionHandler(PREF_TIMEOUT),
                                    &dir);
    Option option;
    handler.parse(option, "foo");
    CPPUNIT_ASSERT(!option.defined(PREF_TIMEOUT));
    CPPUNIT_ASSERT_EQUAL(std::string("foo"), option.get(PREF_DIR));
  }
}

} // namespace aria2
