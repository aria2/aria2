#include "XmlRpcRequestParserController.h"

#include <cppunit/extensions/HelperMacros.h>

namespace aria2 {

namespace rpc {

class XmlRpcRequestParserControllerTest:public CppUnit::TestFixture {

  CPPUNIT_TEST_SUITE(XmlRpcRequestParserControllerTest);
  CPPUNIT_TEST(testPopStructFrame);
  CPPUNIT_TEST(testPopStructFrame_noName);
  CPPUNIT_TEST(testPopStructFrame_noValue);
  CPPUNIT_TEST(testPopArrayFrame);
  CPPUNIT_TEST(testPopArrayFrame_noValue);
  CPPUNIT_TEST(testPopArrayFrame_compound);
  CPPUNIT_TEST_SUITE_END();
public:
  void setUp() {}

  void tearDown() {}

  void testPopStructFrame();
  void testPopStructFrame_noName();
  void testPopStructFrame_noValue();
  void testPopArrayFrame();
  void testPopArrayFrame_noValue();
  void testPopArrayFrame_compound();
};


CPPUNIT_TEST_SUITE_REGISTRATION(XmlRpcRequestParserControllerTest);

void XmlRpcRequestParserControllerTest::testPopStructFrame()
{
  XmlRpcRequestParserController controller;
  controller.setCurrentFrameValue(Dict::g());
  controller.pushFrame();
  controller.setCurrentFrameValue(String::g("Hello, aria2"));
  controller.setCurrentFrameName("greeting");
  controller.popStructFrame();
  const Dict* structValue = downcast<Dict>(controller.getCurrentFrameValue());
  CPPUNIT_ASSERT_EQUAL((size_t)1, structValue->size());
  CPPUNIT_ASSERT_EQUAL(std::string("Hello, aria2"),
                       downcast<String>(structValue->get("greeting"))->s());
}

void XmlRpcRequestParserControllerTest::testPopStructFrame_noName()
{
  XmlRpcRequestParserController controller;
  controller.setCurrentFrameValue(Dict::g());
  controller.pushFrame();
  controller.setCurrentFrameValue(String::g("Hello, aria2"));
  controller.popStructFrame();
  const Dict* structValue = downcast<Dict>(controller.getCurrentFrameValue());
  CPPUNIT_ASSERT(structValue->empty());
}

void XmlRpcRequestParserControllerTest::testPopStructFrame_noValue()
{
  XmlRpcRequestParserController controller;
  controller.setCurrentFrameValue(Dict::g());
  controller.pushFrame();
  controller.setCurrentFrameName("greeting");
  controller.popStructFrame();
  const Dict* structValue = downcast<Dict>(controller.getCurrentFrameValue());
  CPPUNIT_ASSERT(structValue->empty());
}

void XmlRpcRequestParserControllerTest::testPopArrayFrame()
{
  XmlRpcRequestParserController controller;
  controller.setCurrentFrameValue(List::g());
  controller.pushFrame();
  controller.setCurrentFrameValue(Integer::g(100));
  controller.popArrayFrame();
  const List* array = downcast<List>(controller.getCurrentFrameValue());
  CPPUNIT_ASSERT_EQUAL((size_t)1, array->size());
  CPPUNIT_ASSERT_EQUAL((Integer::ValueType)100, downcast<Integer>(array->get(0))->i());
}

void XmlRpcRequestParserControllerTest::testPopArrayFrame_noValue()
{
  XmlRpcRequestParserController controller;
  controller.setCurrentFrameValue(List::g());
  controller.pushFrame();
  controller.popArrayFrame();
  const List* array = downcast<List>(controller.getCurrentFrameValue());
  CPPUNIT_ASSERT(array->empty());
}

void XmlRpcRequestParserControllerTest::testPopArrayFrame_compound()
{
  XmlRpcRequestParserController controller;

  // We are making following structs. [] = array, {key:value .. } = dict

  // [ { "uris":["http://example.org/aria2","http://aria2.sf.net/"],
  //     "options":{ "timeout":120 } },
  //   [ "jp","us" ] ]

  controller.setCurrentFrameValue(List::g());
  controller.pushFrame();

  controller.setCurrentFrameValue(Dict::g());
  controller.pushFrame();

  controller.setCurrentFrameName("uris");
  controller.setCurrentFrameValue(List::g());
  controller.pushFrame();

  controller.setCurrentFrameValue(String::g("http://example.org/aria2"));
  controller.popArrayFrame();
  controller.pushFrame();

  controller.setCurrentFrameValue(String::g("http://aria2.sf.net/"));
  controller.popArrayFrame();

  controller.popStructFrame();
  controller.pushFrame();

  controller.setCurrentFrameName("options");
  controller.setCurrentFrameValue(Dict::g());
  controller.pushFrame();

  controller.setCurrentFrameName("timeout");
  controller.setCurrentFrameValue(Integer::g(120));
  controller.popStructFrame();

  controller.popStructFrame();

  controller.popArrayFrame();
  controller.pushFrame();

  controller.setCurrentFrameValue(List::g());
  controller.pushFrame();

  controller.setCurrentFrameValue(String::g("jp"));
  controller.popArrayFrame();
  controller.pushFrame();

  controller.setCurrentFrameValue(String::g("us"));
  controller.popArrayFrame();

  controller.popArrayFrame();

  const List* result = downcast<List>(controller.getCurrentFrameValue());
  const Dict* dict = downcast<Dict>(result->get(0));
  const List* uris = downcast<List>(dict->get("uris"));
  const Dict* options = downcast<Dict>(dict->get("options"));
  const List* countryList = downcast<List>(result->get(1));
  CPPUNIT_ASSERT_EQUAL(std::string("http://aria2.sf.net/"),
                       downcast<String>(uris->get(1))->s());
  CPPUNIT_ASSERT_EQUAL((Integer::ValueType)120,
                       downcast<Integer>(options->get("timeout"))->i());
  CPPUNIT_ASSERT_EQUAL(std::string("jp"), downcast<String>(countryList->get(0))->s());
}

} // namespace rpc

} // namespace aria2
