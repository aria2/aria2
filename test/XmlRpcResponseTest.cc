#include "XmlRpcResponse.h"

#include <cppunit/extensions/HelperMacros.h>

namespace aria2 {

namespace xmlrpc {

class XmlRpcResponseTest:public CppUnit::TestFixture {
  CPPUNIT_TEST_SUITE(XmlRpcResponseTest);
  CPPUNIT_TEST(testToJson);
  CPPUNIT_TEST_SUITE_END();
public:
  void testToJson();
};

CPPUNIT_TEST_SUITE_REGISTRATION(XmlRpcResponseTest);

void XmlRpcResponseTest::testToJson()
{
  std::vector<XmlRpcResponse> results;
  {
    SharedHandle<List> param = List::g();
    param->append(Integer::g(1));
    SharedHandle<String> id = String::g("9");
    XmlRpcResponse res(0, param, id);
    results.push_back(res);
    std::string s = res.toJson("", false);
    CPPUNIT_ASSERT_EQUAL(std::string("{\"id\":\"9\","
                                     "\"jsonrpc\":\"2.0\","
                                     "\"result\":[1]}"),
                         s);
  }
  {
    // error response
    SharedHandle<Dict> param = Dict::g();
    param->put("code", Integer::g(1));
    param->put("message", "HELLO ERROR");
    XmlRpcResponse res(1, param, Null::g());
    results.push_back(res);
    std::string s = res.toJson("", false);
    CPPUNIT_ASSERT_EQUAL(std::string("{\"error\":{\"code\":1,"
                                     "\"message\":\"HELLO ERROR\"},"
                                     "\"id\":null,"
                                     "\"jsonrpc\":\"2.0\""
                                     "}"),
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
  }
}

} // namespace xmlrpc

} // namespace aria2
