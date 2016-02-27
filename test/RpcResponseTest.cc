#include "RpcResponse.h"

#include <cppunit/extensions/HelperMacros.h>

namespace aria2 {

namespace rpc {

class RpcResponseTest : public CppUnit::TestFixture {
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
    auto param = List::g();
    param->append(Integer::g(1));
    RpcResponse res(0, RpcResponse::AUTHORIZED, std::move(param),
                    String::g("9"));
    results.push_back(std::move(res));
    std::string s = toJson(results.back(), "", false);
    CPPUNIT_ASSERT_EQUAL(std::string("{\"id\":\"9\","
                                     "\"jsonrpc\":\"2.0\","
                                     "\"result\":[1]}"),
                         s);
    // with callback
    s = toJson(results.back(), "cb", false);
    CPPUNIT_ASSERT_EQUAL(std::string("cb({\"id\":\"9\","
                                     "\"jsonrpc\":\"2.0\","
                                     "\"result\":[1]})"),
                         s);
  }
  {
    // error response
    auto param = Dict::g();
    param->put("code", Integer::g(1));
    param->put("message", "HELLO ERROR");
    RpcResponse res(1, RpcResponse::AUTHORIZED, std::move(param), Null::g());
    results.push_back(std::move(res));
    std::string s = toJson(results.back(), "", false);
    CPPUNIT_ASSERT_EQUAL(std::string("{\"id\":null,"
                                     "\"jsonrpc\":\"2.0\","
                                     "\"error\":{\"code\":1,"
                                     "\"message\":\"HELLO ERROR\"}"
                                     "}"),
                         s);
    // with callback
    s = toJson(results.back(), "cb", false);
    CPPUNIT_ASSERT_EQUAL(std::string("cb({\"id\":null,"
                                     "\"jsonrpc\":\"2.0\","
                                     "\"error\":{\"code\":1,"
                                     "\"message\":\"HELLO ERROR\"}"
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
                                     "{\"id\":null,"
                                     "\"jsonrpc\":\"2.0\","
                                     "\"error\":{\"code\":1,"
                                     "\"message\":\"HELLO ERROR\"}"
                                     "}"
                                     "]"),
                         s);
    // with callback
    s = toJsonBatch(results, "cb", false);
    CPPUNIT_ASSERT_EQUAL(std::string("cb(["
                                     "{\"id\":\"9\","
                                     "\"jsonrpc\":\"2.0\","
                                     "\"result\":[1]},"
                                     "{\"id\":null,"
                                     "\"jsonrpc\":\"2.0\","
                                     "\"error\":{\"code\":1,"
                                     "\"message\":\"HELLO ERROR\"}"
                                     "}"
                                     "])"),
                         s);
  }
}

#ifdef ENABLE_XML_RPC
void RpcResponseTest::testToXml()
{
  auto param = Dict::g();
  param->put("faultCode", Integer::g(1));
  param->put("faultString", "No such method: make.hamburger");
  RpcResponse res(1, RpcResponse::AUTHORIZED, std::move(param), Null::g());
  std::string s = toXml(res, false);
  CPPUNIT_ASSERT_EQUAL(
      std::string("<?xml version=\"1.0\"?>"
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
