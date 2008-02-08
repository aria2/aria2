#include "OptionHandlerImpl.h"
#include "prefs.h"
#include "Exception.h"
#include <iostream>
#include <cppunit/extensions/HelperMacros.h>

namespace aria2 {

class OptionHandlerTest:public CppUnit::TestFixture {

  CPPUNIT_TEST_SUITE(OptionHandlerTest);
  CPPUNIT_TEST(testNullOptionHandler);
  CPPUNIT_TEST(testBooleanOptionHandler);
  CPPUNIT_TEST(testNumberOptionHandler);
  CPPUNIT_TEST(testNumberOptionHandler_min);
  CPPUNIT_TEST(testNumberOptionHandler_max);
  CPPUNIT_TEST(testNumberOptionHandler_min_max);
  CPPUNIT_TEST(testUnitNumberOptionHandler);
  CPPUNIT_TEST(testParameterOptionHandler_1argInit);
  CPPUNIT_TEST(testParameterOptionHandler_2argsInit);
  CPPUNIT_TEST(testParameterOptionHandler_listInit);
  CPPUNIT_TEST(testDefaultOptionHandler);
  CPPUNIT_TEST(testFloatNumberOptionHandler);
  CPPUNIT_TEST(testFloatNumberOptionHandler_min);
  CPPUNIT_TEST(testFloatNumberOptionHandler_max);
  CPPUNIT_TEST(testFloatNumberOptionHandler_min_max);
  CPPUNIT_TEST(testLogOptionHandler);
  CPPUNIT_TEST(testHttpProxyOptionHandler);
  CPPUNIT_TEST_SUITE_END();
  
public:
  void testNullOptionHandler();
  void testBooleanOptionHandler();
  void testNumberOptionHandler();
  void testNumberOptionHandler_min();
  void testNumberOptionHandler_max();
  void testNumberOptionHandler_min_max();
  void testUnitNumberOptionHandler();
  void testParameterOptionHandler_1argInit();
  void testParameterOptionHandler_2argsInit();
  void testParameterOptionHandler_listInit();
  void testDefaultOptionHandler();
  void testFloatNumberOptionHandler();
  void testFloatNumberOptionHandler_min();
  void testFloatNumberOptionHandler_max();
  void testFloatNumberOptionHandler_min_max();
  void testLogOptionHandler();
  void testHttpProxyOptionHandler();
};


CPPUNIT_TEST_SUITE_REGISTRATION( OptionHandlerTest );

void OptionHandlerTest::testNullOptionHandler()
{
  NullOptionHandler handler;
  CPPUNIT_ASSERT(handler.canHandle("foo"));
  handler.parse(0, "bar");
}

void OptionHandlerTest::testBooleanOptionHandler()
{
  BooleanOptionHandler handler("foo");
  CPPUNIT_ASSERT(handler.canHandle("foo"));
  CPPUNIT_ASSERT(!handler.canHandle("foobar"));
  Option option;
  handler.parse(&option, V_TRUE);
  CPPUNIT_ASSERT_EQUAL(std::string(V_TRUE), option.get("foo"));
  handler.parse(&option, V_FALSE);
  CPPUNIT_ASSERT_EQUAL(std::string(V_FALSE), option.get("foo"));
  try {
    handler.parse(&option, "hello");
    CPPUNIT_FAIL("exception must be thrown.");
  } catch(Exception* e) {
    std::cerr << e->getMsg() << std::endl;
    delete e;
  }
}

void OptionHandlerTest::testNumberOptionHandler()
{
  NumberOptionHandler handler("foo");
  CPPUNIT_ASSERT(handler.canHandle("foo"));
  CPPUNIT_ASSERT(!handler.canHandle("foobar"));
  Option option;
  handler.parse(&option, "0");
  CPPUNIT_ASSERT_EQUAL(std::string("0"), option.get("foo"));
}

void OptionHandlerTest::testNumberOptionHandler_min()
{
  NumberOptionHandler handler("foo", 1);
  Option option;
  handler.parse(&option, "1");
  CPPUNIT_ASSERT_EQUAL(std::string("1"), option.get("foo"));
  try {
    handler.parse(&option, "0");
    CPPUNIT_FAIL("exception must be thrown.");
  } catch(Exception* e) {
    std::cerr << e->getMsg() << std::endl;
    delete e;
  }

}

void OptionHandlerTest::testNumberOptionHandler_max()
{
  NumberOptionHandler handler("foo", -1, 100);
  Option option;
  handler.parse(&option, "100");
  CPPUNIT_ASSERT_EQUAL(std::string("100"), option.get("foo"));
  try {
    handler.parse(&option, "101");
    CPPUNIT_FAIL("exception must be thrown.");
  } catch(Exception* e) {
    std::cerr << e->getMsg() << std::endl;
    delete e;
  }
}

void OptionHandlerTest::testNumberOptionHandler_min_max()
{
  NumberOptionHandler handler("foo", 1, 100);
  Option option;
  handler.parse(&option, "1");
  CPPUNIT_ASSERT_EQUAL(std::string("1"), option.get("foo"));
  handler.parse(&option, "100");
  CPPUNIT_ASSERT_EQUAL(std::string("100"), option.get("foo"));
  try {
    handler.parse(&option, "0");
    CPPUNIT_FAIL("exception must be thrown.");
  } catch(Exception* e) {
    std::cerr << e->getMsg() << std::endl;
    delete e;
  }
  try {
    handler.parse(&option, "101");
    CPPUNIT_FAIL("exception must be thrown.");
  } catch(Exception* e) {
    std::cerr << e->getMsg() << std::endl;
    delete e;
  }
}

void OptionHandlerTest::testUnitNumberOptionHandler()
{
  UnitNumberOptionHandler handler("foo");
  CPPUNIT_ASSERT(handler.canHandle("foo"));
  CPPUNIT_ASSERT(!handler.canHandle("foobar"));
  Option option;
  handler.parse(&option, "4294967296");
  CPPUNIT_ASSERT_EQUAL(std::string("4294967296"), option.get("foo"));
  handler.parse(&option, "4096M");
  CPPUNIT_ASSERT_EQUAL(std::string("4294967296"), option.get("foo"));
  handler.parse(&option, "4096K");
  CPPUNIT_ASSERT_EQUAL(std::string("4194304"), option.get("foo"));
  try {
    handler.parse(&option, "K");
    CPPUNIT_FAIL("exception must be thrown.");
  } catch(Exception* e) {
    std::cerr << *e;
    delete e;
  }
  try {
    handler.parse(&option, "M");
  } catch(Exception* e) {
    std::cerr << *e;
    delete e;
  }
  try {
    handler.parse(&option, "");
    CPPUNIT_FAIL("exception must be thrown.");
  } catch(Exception* e) {
    std::cerr << *e;
    delete e;
  }
}

void OptionHandlerTest::testParameterOptionHandler_1argInit()
{
  ParameterOptionHandler handler("foo", "value1");
  CPPUNIT_ASSERT(handler.canHandle("foo"));
  CPPUNIT_ASSERT(!handler.canHandle("foobar"));
  Option option;
  handler.parse(&option, "value1");
  CPPUNIT_ASSERT_EQUAL(std::string("value1"), option.get("foo"));
  try {
    handler.parse(&option, "value3");
    CPPUNIT_FAIL("exception must be thrown.");
  } catch(Exception* e) {
    std::cerr << e->getMsg() << std::endl;
    delete e;
  }
}

void OptionHandlerTest::testParameterOptionHandler_2argsInit()
{
  ParameterOptionHandler handler("foo", "value1", "value2");
  CPPUNIT_ASSERT(handler.canHandle("foo"));
  CPPUNIT_ASSERT(!handler.canHandle("foobar"));
  Option option;
  handler.parse(&option, "value1");
  CPPUNIT_ASSERT_EQUAL(std::string("value1"), option.get("foo"));
  handler.parse(&option, "value2");
  CPPUNIT_ASSERT_EQUAL(std::string("value2"), option.get("foo"));
  try {
    handler.parse(&option, "value3");
    CPPUNIT_FAIL("exception must be thrown.");
  } catch(Exception* e) {
    std::cerr << e->getMsg() << std::endl;
    delete e;
  }
}

void OptionHandlerTest::testParameterOptionHandler_listInit()
{
  std::deque<std::string> validValues;
  validValues.push_back("value1");
  validValues.push_back("value2");

  ParameterOptionHandler handler("foo", validValues);
  CPPUNIT_ASSERT(handler.canHandle("foo"));
  CPPUNIT_ASSERT(!handler.canHandle("foobar"));
  Option option;
  handler.parse(&option, "value1");
  CPPUNIT_ASSERT_EQUAL(std::string("value1"), option.get("foo"));
  handler.parse(&option, "value2");
  CPPUNIT_ASSERT_EQUAL(std::string("value2"), option.get("foo"));
  try {
    handler.parse(&option, "value3");
    CPPUNIT_FAIL("exception must be thrown.");
  } catch(Exception* e) {
    std::cerr << e->getMsg() << std::endl;
    delete e;
  }
}

void OptionHandlerTest::testDefaultOptionHandler()
{
  DefaultOptionHandler handler("foo");
  CPPUNIT_ASSERT(handler.canHandle("foo"));
  CPPUNIT_ASSERT(!handler.canHandle("foobar"));
  Option option;
  handler.parse(&option, "bar");
  CPPUNIT_ASSERT_EQUAL(std::string("bar"), option.get("foo"));
  handler.parse(&option, "");
  CPPUNIT_ASSERT_EQUAL(std::string(""), option.get("foo"));
}

void OptionHandlerTest::testFloatNumberOptionHandler()
{
  FloatNumberOptionHandler handler("foo");
  CPPUNIT_ASSERT(handler.canHandle("foo"));
  CPPUNIT_ASSERT(!handler.canHandle("foobar"));
  Option option;
  handler.parse(&option, "1.0");
  CPPUNIT_ASSERT_EQUAL(std::string("1.0"), option.get("foo"));
}

void OptionHandlerTest::testFloatNumberOptionHandler_min()
{
  FloatNumberOptionHandler handler("foo", 0.0);
  Option option;
  handler.parse(&option, "0.0");
  CPPUNIT_ASSERT_EQUAL(std::string("0.0"), option.get("foo"));
  try {
    handler.parse(&option, "-0.1");
    CPPUNIT_FAIL("exception must be thrown.");
  } catch(Exception* e) {
    std::cerr << e->getMsg() << std::endl;
    delete e;
  }
}

void OptionHandlerTest::testFloatNumberOptionHandler_max()
{
  FloatNumberOptionHandler handler("foo", -1, 10.0);
  Option option;
  handler.parse(&option, "10.0");
  CPPUNIT_ASSERT_EQUAL(std::string("10.0"), option.get("foo"));
  try {
    handler.parse(&option, "10.1");
    CPPUNIT_FAIL("exception must be thrown.");
  } catch(Exception* e) {
    std::cerr << e->getMsg() << std::endl;
    delete e;
  }
}

void OptionHandlerTest::testFloatNumberOptionHandler_min_max()
{
  FloatNumberOptionHandler handler("foo", 0.0, 10.0);
  Option option;
  handler.parse(&option, "0.0");
  CPPUNIT_ASSERT_EQUAL(std::string("0.0"), option.get("foo"));
  handler.parse(&option, "10.0");
  CPPUNIT_ASSERT_EQUAL(std::string("10.0"), option.get("foo"));
  try {
    handler.parse(&option, "-0.1");
    CPPUNIT_FAIL("exception must be thrown.");
  } catch(Exception* e) {
    std::cerr << e->getMsg() << std::endl;
    delete e;
  }
  try {
    handler.parse(&option, "10.1");
    CPPUNIT_FAIL("exception must be thrown.");
  } catch(Exception* e) {
    std::cerr << e->getMsg() << std::endl;
    delete e;
  }
}

void OptionHandlerTest::testLogOptionHandler()
{
  LogOptionHandler handler("foo");
  CPPUNIT_ASSERT(handler.canHandle("foo"));
  CPPUNIT_ASSERT(!handler.canHandle("foobar"));
  Option option;
  handler.parse(&option, "/tmp/log.txt");
  CPPUNIT_ASSERT_EQUAL(std::string("/tmp/log.txt"), option.get(PREF_LOG));
  CPPUNIT_ASSERT_EQUAL(std::string(""), option.get(PREF_STDOUT_LOG));

  option.clear();
  handler.parse(&option, "-");
  CPPUNIT_ASSERT_EQUAL(std::string(""), option.get(PREF_LOG));
  CPPUNIT_ASSERT_EQUAL(std::string(V_TRUE), option.get(PREF_STDOUT_LOG));
}

void OptionHandlerTest::testHttpProxyOptionHandler()
{
  HttpProxyOptionHandler handler(PREF_HTTP_PROXY,
				 PREF_HTTP_PROXY_HOST,
				 PREF_HTTP_PROXY_PORT);
  CPPUNIT_ASSERT(handler.canHandle(PREF_HTTP_PROXY));
  CPPUNIT_ASSERT(!handler.canHandle("foobar"));
  Option option;
  handler.parse(&option, "bar:80");
  CPPUNIT_ASSERT_EQUAL(std::string("bar:80"), option.get(PREF_HTTP_PROXY));
  CPPUNIT_ASSERT_EQUAL(std::string("bar"), option.get(PREF_HTTP_PROXY_HOST));
  CPPUNIT_ASSERT_EQUAL(std::string("80"), option.get(PREF_HTTP_PROXY_PORT));
  CPPUNIT_ASSERT_EQUAL(std::string(V_TRUE), option.get(PREF_HTTP_PROXY_ENABLED));

  try {
    handler.parse(&option, "bar");
    CPPUNIT_FAIL("exception must be thrown.");
  } catch(Exception* e) {
    std::cerr << e->getMsg() << std::endl;
    delete e;
  }
  try {
    handler.parse(&option, "bar:");
    CPPUNIT_FAIL("exception must be thrown.");
  } catch(Exception* e) {
    std::cerr << e->getMsg() << std::endl;
    delete e;
  }
  try {
    handler.parse(&option, ":");
    CPPUNIT_FAIL("exception must be thrown.");
  } catch(Exception* e) {
    std::cerr << e->getMsg() << std::endl;
    delete e;
  }
  try {
    handler.parse(&option, ":80");
    CPPUNIT_FAIL("exception must be thrown.");
  } catch(Exception* e) {
    std::cerr << e->getMsg() << std::endl;
    delete e;
  }
  try {
    handler.parse(&option, "foo:bar");
    CPPUNIT_FAIL("exception must be thrown.");
  } catch(Exception* e) {
    std::cerr << e->getMsg() << std::endl;
    delete e;
  }
}

} // namespace aria2
