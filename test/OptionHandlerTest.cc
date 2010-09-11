#include "OptionHandlerImpl.h"

#include <cppunit/extensions/HelperMacros.h>

#include "prefs.h"
#include "Exception.h"

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
  CPPUNIT_TEST(testHttpProxyOptionHandler);
  CPPUNIT_TEST(testHttpProxyUserOptionHandler);
  CPPUNIT_TEST(testHttpProxyPasswdOptionHandler);
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
  void testHttpProxyOptionHandler();
  void testHttpProxyUserOptionHandler();
  void testHttpProxyPasswdOptionHandler();
};


CPPUNIT_TEST_SUITE_REGISTRATION( OptionHandlerTest );

void OptionHandlerTest::testNullOptionHandler()
{
  NullOptionHandler handler;
  CPPUNIT_ASSERT(handler.canHandle("foo"));
  Option option;
  handler.parse(option, "bar");
  CPPUNIT_ASSERT(!option.defined("bar"));
}

void OptionHandlerTest::testBooleanOptionHandler()
{
  BooleanOptionHandler handler("foo");
  CPPUNIT_ASSERT(handler.canHandle("foo"));
  CPPUNIT_ASSERT(!handler.canHandle("foobar"));
  Option option;
  handler.parse(option, A2_V_TRUE);
  CPPUNIT_ASSERT_EQUAL(std::string(A2_V_TRUE), option.get("foo"));
  handler.parse(option, A2_V_FALSE);
  CPPUNIT_ASSERT_EQUAL(std::string(A2_V_FALSE), option.get("foo"));
  try {
    handler.parse(option, "hello");
    CPPUNIT_FAIL("exception must be thrown.");
  } catch(Exception& e) {}
  CPPUNIT_ASSERT_EQUAL(std::string("true,false"),
                       handler.createPossibleValuesString());
}

void OptionHandlerTest::testNumberOptionHandler()
{
  NumberOptionHandler handler("foo");
  CPPUNIT_ASSERT(handler.canHandle("foo"));
  CPPUNIT_ASSERT(!handler.canHandle("foobar"));
  Option option;
  handler.parse(option, "0");
  CPPUNIT_ASSERT_EQUAL(std::string("0"), option.get("foo"));
  CPPUNIT_ASSERT_EQUAL(std::string("*-*"),
                       handler.createPossibleValuesString());
}

void OptionHandlerTest::testNumberOptionHandler_min()
{
  NumberOptionHandler handler("foo", "", "", 1);
  Option option;
  handler.parse(option, "1");
  CPPUNIT_ASSERT_EQUAL(std::string("1"), option.get("foo"));
  try {
    handler.parse(option, "0");
    CPPUNIT_FAIL("exception must be thrown.");
  } catch(Exception& e) {}
  CPPUNIT_ASSERT_EQUAL(std::string("1-*"),
                       handler.createPossibleValuesString());
}

void OptionHandlerTest::testNumberOptionHandler_max()
{
  NumberOptionHandler handler("foo", "", "", -1, 100);
  Option option;
  handler.parse(option, "100");
  CPPUNIT_ASSERT_EQUAL(std::string("100"), option.get("foo"));
  try {
    handler.parse(option, "101");
    CPPUNIT_FAIL("exception must be thrown.");
  } catch(Exception& e) {}
  CPPUNIT_ASSERT_EQUAL(std::string("*-100"),
                       handler.createPossibleValuesString());
}

void OptionHandlerTest::testNumberOptionHandler_min_max()
{
  NumberOptionHandler handler("foo", "", "", 1, 100);
  Option option;
  handler.parse(option, "1");
  CPPUNIT_ASSERT_EQUAL(std::string("1"), option.get("foo"));
  handler.parse(option, "100");
  CPPUNIT_ASSERT_EQUAL(std::string("100"), option.get("foo"));
  try {
    handler.parse(option, "0");
    CPPUNIT_FAIL("exception must be thrown.");
  } catch(Exception& e) {}
  try {
    handler.parse(option, "101");
    CPPUNIT_FAIL("exception must be thrown.");
  } catch(Exception& e) {}
  CPPUNIT_ASSERT_EQUAL(std::string("1-100"),
                       handler.createPossibleValuesString());
}

void OptionHandlerTest::testUnitNumberOptionHandler()
{
  UnitNumberOptionHandler handler("foo");
  CPPUNIT_ASSERT(handler.canHandle("foo"));
  CPPUNIT_ASSERT(!handler.canHandle("foobar"));
  Option option;
  handler.parse(option, "4294967296");
  CPPUNIT_ASSERT_EQUAL(std::string("4294967296"), option.get("foo"));
  handler.parse(option, "4096M");
  CPPUNIT_ASSERT_EQUAL(std::string("4294967296"), option.get("foo"));
  handler.parse(option, "4096K");
  CPPUNIT_ASSERT_EQUAL(std::string("4194304"), option.get("foo"));
  try {
    handler.parse(option, "K");
    CPPUNIT_FAIL("exception must be thrown.");
  } catch(Exception& e) {}
  try {
    handler.parse(option, "M");
  } catch(Exception& e) {}
  try {
    handler.parse(option, "");
    CPPUNIT_FAIL("exception must be thrown.");
  } catch(Exception& e) {}
}

void OptionHandlerTest::testParameterOptionHandler_1argInit()
{
  ParameterOptionHandler handler("foo", "", "", "value1");
  CPPUNIT_ASSERT(handler.canHandle("foo"));
  CPPUNIT_ASSERT(!handler.canHandle("foobar"));
  Option option;
  handler.parse(option, "value1");
  CPPUNIT_ASSERT_EQUAL(std::string("value1"), option.get("foo"));
  try {
    handler.parse(option, "value3");
    CPPUNIT_FAIL("exception must be thrown.");
  } catch(Exception& e) {}
  CPPUNIT_ASSERT_EQUAL(std::string("value1"),
                       handler.createPossibleValuesString());
}

void OptionHandlerTest::testParameterOptionHandler_2argsInit()
{
  ParameterOptionHandler handler("foo", "", "", "value1", "value2");
  CPPUNIT_ASSERT(handler.canHandle("foo"));
  CPPUNIT_ASSERT(!handler.canHandle("foobar"));
  Option option;
  handler.parse(option, "value1");
  CPPUNIT_ASSERT_EQUAL(std::string("value1"), option.get("foo"));
  handler.parse(option, "value2");
  CPPUNIT_ASSERT_EQUAL(std::string("value2"), option.get("foo"));
  try {
    handler.parse(option, "value3");
    CPPUNIT_FAIL("exception must be thrown.");
  } catch(Exception& e) {}
  CPPUNIT_ASSERT_EQUAL(std::string("value1,value2"),
                       handler.createPossibleValuesString());
}

void OptionHandlerTest::testParameterOptionHandler_listInit()
{
  std::vector<std::string> validValues;
  validValues.push_back("value1");
  validValues.push_back("value2");

  ParameterOptionHandler handler("foo", "", "", validValues);
  CPPUNIT_ASSERT(handler.canHandle("foo"));
  CPPUNIT_ASSERT(!handler.canHandle("foobar"));
  Option option;
  handler.parse(option, "value1");
  CPPUNIT_ASSERT_EQUAL(std::string("value1"), option.get("foo"));
  handler.parse(option, "value2");
  CPPUNIT_ASSERT_EQUAL(std::string("value2"), option.get("foo"));
  try {
    handler.parse(option, "value3");
    CPPUNIT_FAIL("exception must be thrown.");
  } catch(Exception& e) {}
  CPPUNIT_ASSERT_EQUAL(std::string("value1,value2"),
                       handler.createPossibleValuesString());
}

void OptionHandlerTest::testDefaultOptionHandler()
{
  DefaultOptionHandler handler("foo");
  CPPUNIT_ASSERT(handler.canHandle("foo"));
  CPPUNIT_ASSERT(!handler.canHandle("foobar"));
  Option option;
  handler.parse(option, "bar");
  CPPUNIT_ASSERT_EQUAL(std::string("bar"), option.get("foo"));
  handler.parse(option, "");
  CPPUNIT_ASSERT_EQUAL(std::string(""), option.get("foo"));
  CPPUNIT_ASSERT_EQUAL(std::string(""), handler.createPossibleValuesString());

  handler.addTag("apple");
  CPPUNIT_ASSERT_EQUAL(std::string("apple"), handler.toTagString());
  handler.addTag("orange");
  CPPUNIT_ASSERT_EQUAL(std::string("apple, orange"), handler.toTagString());
  CPPUNIT_ASSERT(handler.hasTag("apple"));
  CPPUNIT_ASSERT(handler.hasTag("orange"));
  CPPUNIT_ASSERT(!handler.hasTag("pineapple"));
}

void OptionHandlerTest::testFloatNumberOptionHandler()
{
  FloatNumberOptionHandler handler("foo");
  CPPUNIT_ASSERT(handler.canHandle("foo"));
  CPPUNIT_ASSERT(!handler.canHandle("foobar"));
  Option option;
  handler.parse(option, "1.0");
  CPPUNIT_ASSERT_EQUAL(std::string("1.0"), option.get("foo"));
  CPPUNIT_ASSERT_EQUAL(std::string("*-*"),
                       handler.createPossibleValuesString());
}

void OptionHandlerTest::testFloatNumberOptionHandler_min()
{
  FloatNumberOptionHandler handler("foo", "", "", 0.0);
  Option option;
  handler.parse(option, "0.0");
  CPPUNIT_ASSERT_EQUAL(std::string("0.0"), option.get("foo"));
  try {
    handler.parse(option, "-0.1");
    CPPUNIT_FAIL("exception must be thrown.");
  } catch(Exception& e) {}
  CPPUNIT_ASSERT_EQUAL(std::string("0.0-*"),
                       handler.createPossibleValuesString());
}

void OptionHandlerTest::testFloatNumberOptionHandler_max()
{
  FloatNumberOptionHandler handler("foo", "", "", -1, 10.0);
  Option option;
  handler.parse(option, "10.0");
  CPPUNIT_ASSERT_EQUAL(std::string("10.0"), option.get("foo"));
  try {
    handler.parse(option, "10.1");
    CPPUNIT_FAIL("exception must be thrown.");
  } catch(Exception& e) {}
  CPPUNIT_ASSERT_EQUAL(std::string("*-10.0"),
                       handler.createPossibleValuesString());
}

void OptionHandlerTest::testFloatNumberOptionHandler_min_max()
{
  FloatNumberOptionHandler handler("foo", "", "", 0.0, 10.0);
  Option option;
  handler.parse(option, "0.0");
  CPPUNIT_ASSERT_EQUAL(std::string("0.0"), option.get("foo"));
  handler.parse(option, "10.0");
  CPPUNIT_ASSERT_EQUAL(std::string("10.0"), option.get("foo"));
  try {
    handler.parse(option, "-0.1");
    CPPUNIT_FAIL("exception must be thrown.");
  } catch(Exception& e) {}
  try {
    handler.parse(option, "10.1");
    CPPUNIT_FAIL("exception must be thrown.");
  } catch(Exception& e) {}
  CPPUNIT_ASSERT_EQUAL(std::string("0.0-10.0"),
                       handler.createPossibleValuesString());
}

void OptionHandlerTest::testHttpProxyOptionHandler()
{
  HttpProxyOptionHandler handler(PREF_HTTP_PROXY, "", "");
  CPPUNIT_ASSERT(handler.canHandle(PREF_HTTP_PROXY));
  CPPUNIT_ASSERT(!handler.canHandle("foobar"));
  Option option;
  handler.parse(option, "proxy:65535");
  CPPUNIT_ASSERT_EQUAL(std::string("http://proxy:65535"),
                       option.get(PREF_HTTP_PROXY));

  handler.parse(option, "http://proxy:8080");
  CPPUNIT_ASSERT_EQUAL(std::string("http://proxy:8080"),
                       option.get(PREF_HTTP_PROXY));

  handler.parse(option, "");
  CPPUNIT_ASSERT(option.defined(PREF_HTTP_PROXY));
  CPPUNIT_ASSERT(option.blank(PREF_HTTP_PROXY));

  try {
    handler.parse(option, "http://bar:65536");
    CPPUNIT_FAIL("exception must be thrown.");
  } catch(Exception& e) {}
  CPPUNIT_ASSERT_EQUAL(std::string("[http://][USER:PASSWORD@]HOST[:PORT]"),
                       handler.createPossibleValuesString());

  handler.parse(option, "http://user%40:passwd%40@proxy:8080");
  CPPUNIT_ASSERT_EQUAL(std::string("http://user%40:passwd%40@proxy:8080"),
                       option.get(PREF_HTTP_PROXY));

  option.put(PREF_HTTP_PROXY_USER, "proxy@user");
  handler.parse(option, "http://proxy:8080");
  CPPUNIT_ASSERT_EQUAL(std::string("http://proxy%40user@proxy:8080"),
                       option.get(PREF_HTTP_PROXY));

  option.put(PREF_HTTP_PROXY_PASSWD, "proxy@passwd");
  handler.parse(option, "http://proxy:8080");
  CPPUNIT_ASSERT_EQUAL
    (std::string("http://proxy%40user:proxy%40passwd@proxy:8080"),
     option.get(PREF_HTTP_PROXY));

  handler.parse(option, "http://user:passwd@proxy:8080");
  CPPUNIT_ASSERT_EQUAL(std::string("http://user:passwd@proxy:8080"),
                       option.get(PREF_HTTP_PROXY));

  option.put(PREF_HTTP_PROXY_USER, "");
  handler.parse(option, "http://proxy:8080");
  CPPUNIT_ASSERT_EQUAL(std::string("http://:proxy%40passwd@proxy:8080"),
                       option.get(PREF_HTTP_PROXY));

  option.put(PREF_HTTP_PROXY_PASSWD, "");
  handler.parse(option, "http://proxy:8080");
  CPPUNIT_ASSERT_EQUAL(std::string("http://:@proxy:8080"),
                       option.get(PREF_HTTP_PROXY));
}

void OptionHandlerTest::testHttpProxyUserOptionHandler()
{
  HttpProxyUserOptionHandler handler(PREF_HTTP_PROXY_USER, "", "");
  Option option;
  handler.parse(option, "proxyuser");
  CPPUNIT_ASSERT_EQUAL(std::string("proxyuser"),
                       option.get(PREF_HTTP_PROXY_USER));

  option.put(PREF_HTTP_PROXY, "http://proxy:8080");
  handler.parse(option, "proxy@user");
  CPPUNIT_ASSERT_EQUAL(std::string("proxy@user"),
                       option.get(PREF_HTTP_PROXY_USER));
  CPPUNIT_ASSERT_EQUAL(std::string("http://proxy%40user@proxy:8080"),
                       option.get(PREF_HTTP_PROXY));

  option.put(PREF_HTTP_PROXY, "http://user@proxy:8080");
  handler.parse(option, "proxyuser");
  CPPUNIT_ASSERT_EQUAL(std::string("http://proxyuser@proxy:8080"),
                       option.get(PREF_HTTP_PROXY));

  option.put(PREF_HTTP_PROXY, "http://user:passwd%40@proxy:8080");
  handler.parse(option, "proxyuser");
  CPPUNIT_ASSERT_EQUAL(std::string("http://proxyuser:passwd%40@proxy:8080"),
                       option.get(PREF_HTTP_PROXY));

  handler.parse(option, "");
  CPPUNIT_ASSERT_EQUAL(std::string("http://:passwd%40@proxy:8080"),
                       option.get(PREF_HTTP_PROXY));
}

void OptionHandlerTest::testHttpProxyPasswdOptionHandler()
{
  HttpProxyPasswdOptionHandler handler(PREF_HTTP_PROXY_PASSWD, "", "");
  Option option;
  handler.parse(option, "proxypasswd");
  CPPUNIT_ASSERT_EQUAL(std::string("proxypasswd"),
                       option.get(PREF_HTTP_PROXY_PASSWD));

  option.put(PREF_HTTP_PROXY, "http://proxy:8080");
  handler.parse(option, "proxy@passwd");
  CPPUNIT_ASSERT_EQUAL(std::string("proxy@passwd"),
                       option.get(PREF_HTTP_PROXY_PASSWD));
  CPPUNIT_ASSERT_EQUAL(std::string("http://:proxy%40passwd@proxy:8080"),
                       option.get(PREF_HTTP_PROXY));

  option.put(PREF_HTTP_PROXY, "http://:pass@proxy:8080");
  handler.parse(option, "proxypasswd");
  CPPUNIT_ASSERT_EQUAL(std::string("http://:proxypasswd@proxy:8080"),
                       option.get(PREF_HTTP_PROXY));

  option.put(PREF_HTTP_PROXY, "http://user%40:pass@proxy:8080");
  handler.parse(option, "proxypasswd");
  CPPUNIT_ASSERT_EQUAL(std::string("http://user%40:proxypasswd@proxy:8080"),
                       option.get(PREF_HTTP_PROXY));

  handler.parse(option, "");
  CPPUNIT_ASSERT_EQUAL(std::string("http://user%40:@proxy:8080"),
                       option.get(PREF_HTTP_PROXY));
  
}

} // namespace aria2
