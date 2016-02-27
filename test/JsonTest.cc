#include "json.h"

#include <cppunit/extensions/HelperMacros.h>

#include "RecoverableException.h"
#include "util.h"
#include "array_fun.h"
#include "base64.h"

namespace aria2 {

class JsonTest : public CppUnit::TestFixture {

  CPPUNIT_TEST_SUITE(JsonTest);
  CPPUNIT_TEST(testEncode);
  CPPUNIT_TEST(testDecodeGetParams);
  CPPUNIT_TEST_SUITE_END();

private:
public:
  void testEncode();
  void testDecodeGetParams();
};

CPPUNIT_TEST_SUITE_REGISTRATION(JsonTest);

void JsonTest::testEncode()
{
  {
    auto dict = Dict::g();
    dict->put("name", String::g("aria2"));
    dict->put("loc", Integer::g(80000));
    auto files = List::g();
    files->append(String::g("aria2c"));
    dict->put("files", std::move(files));
    auto attrs = Dict::g();
    attrs->put("license", String::g("GPL"));
    dict->put("attrs", std::move(attrs));

    CPPUNIT_ASSERT_EQUAL(std::string("{\"attrs\":{\"license\":\"GPL\"},"
                                     "\"files\":[\"aria2c\"],"
                                     "\"loc\":80000,"
                                     "\"name\":\"aria2\"}"),
                         json::encode(dict.get()));
  }
  {
    auto list = List::g();
    list->append("\"\\/\b\f\n\r\t");
    CPPUNIT_ASSERT_EQUAL(std::string("[\"\\\"\\\\\\/\\b\\f\\n\\r\\t\"]"),
                         json::encode(list.get()));
  }
  {
    auto list = List::g();
    std::string s;
    s += 0x1Fu;
    list->append(s);
    CPPUNIT_ASSERT_EQUAL(std::string("[\"\\u001F\"]"),
                         json::encode(list.get()));
  }
  {
    auto list = List::g();
    list->append(Bool::gTrue());
    list->append(Bool::gFalse());
    list->append(Null::g());
    CPPUNIT_ASSERT_EQUAL(std::string("[true,false,null]"),
                         json::encode(list.get()));
  }
}

void JsonTest::testDecodeGetParams()
{
  {
    std::string s = "[1,2,3]";
    std::string param = util::percentEncode(base64::encode(s.begin(), s.end()));
    std::string query = "?params=";
    query += param;
    query += '&';
    query += "method=sum&";
    query += "id=300&";
    query += "jsoncallback=cb";
    json::JsonGetParam gparam = json::decodeGetParams(query);
    CPPUNIT_ASSERT_EQUAL(std::string("{\"method\":\"sum\","
                                     "\"id\":\"300\","
                                     "\"params\":[1,2,3]}"),
                         gparam.request);
    CPPUNIT_ASSERT_EQUAL(std::string("cb"), gparam.callback);
  }
  {
    std::string s = "[{}]";
    std::string query = "?params=";
    query += util::percentEncode(base64::encode(s.begin(), s.end()));
    query += '&';
    query += "jsoncallback=cb";
    json::JsonGetParam gparam = json::decodeGetParams(query);
    CPPUNIT_ASSERT_EQUAL(std::string("[{}]"), gparam.request);
    CPPUNIT_ASSERT_EQUAL(std::string("cb"), gparam.callback);
  }
  {
    std::string query = "?method=sum&id=300";
    json::JsonGetParam gparam = json::decodeGetParams(query);
    CPPUNIT_ASSERT_EQUAL(std::string("{\"method\":\"sum\","
                                     "\"id\":\"300\"}"),
                         gparam.request);
    CPPUNIT_ASSERT_EQUAL(std::string(), gparam.callback);
  }
}

} // namespace aria2
