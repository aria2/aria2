#include "XmlRpcRequestParserController.h"

#include <cppunit/extensions/HelperMacros.h>

namespace aria2 {

namespace xmlrpc {

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
  controller.setCurrentFrameValue(BDE::dict());
  controller.pushFrame();
  controller.setCurrentFrameValue(BDE("Hello, aria2"));
  controller.setCurrentFrameName("greeting");
  controller.popStructFrame();
  const BDE& structValue = controller.getCurrentFrameValue();
  CPPUNIT_ASSERT_EQUAL((size_t)1, structValue.size());
  CPPUNIT_ASSERT_EQUAL(std::string("Hello, aria2"),
                       structValue["greeting"].s());
}

void XmlRpcRequestParserControllerTest::testPopStructFrame_noName()
{
  XmlRpcRequestParserController controller;
  controller.setCurrentFrameValue(BDE::dict());
  controller.pushFrame();
  controller.setCurrentFrameValue(BDE("Hello, aria2"));
  controller.popStructFrame();
  const BDE& structValue = controller.getCurrentFrameValue();
  CPPUNIT_ASSERT(structValue.empty());
}

void XmlRpcRequestParserControllerTest::testPopStructFrame_noValue()
{
  XmlRpcRequestParserController controller;
  controller.setCurrentFrameValue(BDE::dict());
  controller.pushFrame();
  controller.setCurrentFrameName("greeting");
  controller.popStructFrame();
  const BDE& structValue = controller.getCurrentFrameValue();
  CPPUNIT_ASSERT(structValue.empty());
}

void XmlRpcRequestParserControllerTest::testPopArrayFrame()
{
  XmlRpcRequestParserController controller;
  controller.setCurrentFrameValue(BDE::list());
  controller.pushFrame();
  controller.setCurrentFrameValue(BDE(100));
  controller.popArrayFrame();
  const BDE& array = controller.getCurrentFrameValue();
  CPPUNIT_ASSERT_EQUAL((size_t)1, array.size());
  CPPUNIT_ASSERT_EQUAL((BDE::Integer)100, array[0].i());
}

void XmlRpcRequestParserControllerTest::testPopArrayFrame_noValue()
{
  XmlRpcRequestParserController controller;
  controller.setCurrentFrameValue(BDE::list());
  controller.pushFrame();
  controller.popArrayFrame();
  const BDE& array = controller.getCurrentFrameValue();
  CPPUNIT_ASSERT(array.empty());
}

void XmlRpcRequestParserControllerTest::testPopArrayFrame_compound()
{
  XmlRpcRequestParserController controller;

  // We are making following structs. [] = array, {key:value .. } = dict

  // [ { "uris":["http://example.org/aria2","http://aria2.sf.net/"],
  //     "options":{ "timeout":120 } },
  //   [ "jp","us" ] ]

  controller.setCurrentFrameValue(BDE::list());
  controller.pushFrame();

  controller.setCurrentFrameValue(BDE::dict());
  controller.pushFrame();

  controller.setCurrentFrameName("uris");
  controller.setCurrentFrameValue(BDE::list());
  controller.pushFrame();

  controller.setCurrentFrameValue(BDE("http://example.org/aria2"));
  controller.popArrayFrame();
  controller.pushFrame();

  controller.setCurrentFrameValue(BDE("http://aria2.sf.net/"));
  controller.popArrayFrame();

  controller.popStructFrame();
  controller.pushFrame();

  controller.setCurrentFrameName("options");
  controller.setCurrentFrameValue(BDE::dict());
  controller.pushFrame();

  controller.setCurrentFrameName("timeout");
  controller.setCurrentFrameValue(BDE(120));
  controller.popStructFrame();

  controller.popStructFrame();

  controller.popArrayFrame();
  controller.pushFrame();

  controller.setCurrentFrameValue(BDE::list());
  controller.pushFrame();

  controller.setCurrentFrameValue(BDE("jp"));
  controller.popArrayFrame();
  controller.pushFrame();

  controller.setCurrentFrameValue(BDE("us"));
  controller.popArrayFrame();

  controller.popArrayFrame();

  const BDE& result = controller.getCurrentFrameValue();
  CPPUNIT_ASSERT_EQUAL(std::string("http://aria2.sf.net/"),
                       result[0]["uris"][1].s());
  CPPUNIT_ASSERT_EQUAL((BDE::Integer)120, result[0]["options"]["timeout"].i());
  CPPUNIT_ASSERT_EQUAL(std::string("jp"), result[1][0].s());
}

} // namespace xmlrpc

} // namespace aria2
