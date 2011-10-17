#include "RpcResponse.h"

#include <cppunit/extensions/HelperMacros.h>

namespace aria2 {

namespace rpc {

class RpcResponseTest:public CppUnit::TestFixture {
  CPPUNIT_TEST_SUITE(RpcResponseTest);
  CPPUNIT_TEST(testToJson);
#ifdef ENABLE_XML_RPC
  CPPUNIT_TEST(testToXml);
#endif // ENABLE_XML_RPC
  CPPUNIT_TEST_SUITE_END();
public:
  void testToJson();
#ifdef ENABLE_XML_RPC
  void testToXml();
#endif // ENABLE_XML_RPC
};

CPPUNIT_TEST_SUITE_REGISTRATION(RpcResponseTest);

void RpcResponseTest::testToJson()
{
  std::vector<RpcResponse> results;
  {
    SharedHandle<List> param = List::g();
    param->append(Integer::g(1));
    SharedHandle<String> id = String::g("9");
    RpcResponse res(0, param, id);
    results.push_back(res);
    std::string s = toJson(res, "", false);
    CPPUNIT_ASSERT_EQUAL(std::string("{\"id\":\"9\","
                                     "\"jsonrpc\":\"2.0\","
                                     "\"result\":[1]}"),
                         s);
    // with callback
    s = toJson(res, "cb", false);
    CPPUNIT_ASSERT_EQUAL(std::string("cb({\"id\":\"9\","
                                     "\"jsonrpc\":\"2.0\","
                                     "\"result\":[1]})"),
                         s);
  }
  {
    // error response
    SharedHandle<Dict> param = Dict::g();
    param->put("code", Integer::g(1));
    param->put("message", "HELLO ERROR");
    RpcResponse res(1, param, Null::g());
    results.push_back(res);
    std::string s = toJson(res, "", false);
    CPPUNIT_ASSERT_EQUAL(std::string("{\"error\":{\"code\":1,"
                                     "\"message\":\"HELLO ERROR\"},"
                                     "\"id\":null,"
                                     "\"jsonrpc\":\"2.0\""
                                     "}"),
                         s);
    // with callback
    s = toJson(res, "cb", false);
    CPPUNIT_ASSERT_EQUAL(std::string("cb({\"error\":{\"code\":1,"
                                     "\"message\":\"HELLO ERROR\"},"
                                     "\"id\":null,"
                                     "\"jsonrpc\":\"2.0\""
                                     "})"),
                         s);
  }
  {
    // batch response
    std::string s = toJsonBatch(results, "", false);
    CPPUNIT_ASSERT_EQUAL(std::string("["
                                     "{\"id\":\"9\","
                                     "\"jsonrpc\":\"2.0\","
                                     "\"result\":[1]},"
                                     "{\"error\":{\"code\":1,"
                                     "\"message\":\"HELLO ERROR\"},"
                                     "\"id\":null,"
                                     "\"jsonrpc\":\"2.0\""
                                     "}"
                                     "]"),
                         s);
    // with callback
    s = toJsonBatch(results, "cb", false);
    CPPUNIT_ASSERT_EQUAL(std::string("cb(["
                                     "{\"id\":\"9\","
                                     "\"jsonrpc\":\"2.0\","
                                     "\"result\":[1]},"
                                     "{\"error\":{\"code\":1,"
                                     "\"message\":\"HELLO ERROR\"},"
                                     "\"id\":null,"
                                     "\"jsonrpc\":\"2.0\""
                                     "}"
                                     "])"),
                         s);
  }
}

#ifdef ENABLE_XML_RPC
void RpcResponseTest::testToXml()
{
  SharedHandle<Dict> param = Dict::g();
  param->put("faultCode", Integer::g(1));
  param->put("faultString", "No such method: make.hamburger");
  RpcResponse res(1, param, Null::g());
  std::string s = toXml(res, false);
  CPPUNIT_ASSERT_EQUAL
    (std::string("<?xml version=\"1.0\"?>"
                 "<methodResponse>"
                 "<fault>"
                 "<value>"
                 "<struct>"
                 "<member>"
                 "<name>faultCode</name><value><int>1</int></value>"
                 "</member>"
                 "<member>"
                 "<name>faultString</name>"
                 "<value>"
                 "<string>No such method: make.hamburger</string>"
                 "</value>"
                 "</member>"
                 "</struct>"
                 "</value>"
                 "</fault>"
                 "</methodResponse>"),
     s);
}
#endif // ENABLE_XML_RPC

} // namespace rpc

} // namespace aria2
