#include "XmlRpcRequestProcessor.h"

#include <cppunit/extensions/HelperMacros.h>

#include "XmlRpcRequestParserStateMachine.h"

namespace aria2 {

namespace xmlrpc {

class XmlRpcRequestProcessorTest:public CppUnit::TestFixture {

  CPPUNIT_TEST_SUITE(XmlRpcRequestProcessorTest);
  CPPUNIT_TEST(testParseMemory);
  CPPUNIT_TEST(testParseMemory_shouldFail);
  CPPUNIT_TEST_SUITE_END();
public:
  void setUp() {}

  void tearDown() {}

  void testParseMemory();
  void testParseMemory_shouldFail();
};


CPPUNIT_TEST_SUITE_REGISTRATION(XmlRpcRequestProcessorTest);

void XmlRpcRequestProcessorTest::testParseMemory()
{
  XmlRpcRequestProcessor proc;
  XmlRpcRequest req =
    proc.parseMemory("<?xml version=\"1.0\"?>"
                     "<methodCall>"
                     "  <methodName>aria2.addURI</methodName>"
                     "    <params>"
                     "      <param>"
                     "        <value><i4>100</i4></value>"
                     "      </param>"
                     "      <param>"
                     "       <value>"
                     "         <struct>"
                     "           <member>"
                     "             <name>max-count</name>"
                     "             <value><i4>65535</i4></value>"
                     "           </member>"
                     "           <member>"
                     "             <name>seed-ratio</name>"
                     "             <value><double>0.99</double></value>"
                     "           </member>"
                     "         </struct>"
                     "       </value>"
                     "     </param>"
                     "     <param>"
                     "       <value>"
                     "         <array>"
                     "           <data>"
                     "             <value><string>pudding</string></value>"
                     "             <value><base64>aGVsbG8gd29ybGQ=</base64></value>"
                     "           </data>"                    
                     "         </array>"
                     "       </value>"
                     "     </param>"
                     "   </params>"
                     "</methodCall>");

  CPPUNIT_ASSERT_EQUAL(std::string("aria2.addURI"), req.methodName);
  CPPUNIT_ASSERT_EQUAL((size_t)3, req.params.size());
  CPPUNIT_ASSERT_EQUAL((int64_t)100, req.params[0].i());
  CPPUNIT_ASSERT_EQUAL((int64_t)65535, req.params[1]["max-count"].i());
  // Current implementation handles double as string.
  CPPUNIT_ASSERT_EQUAL(std::string("0.99"), req.params[1]["seed-ratio"].s());
  CPPUNIT_ASSERT_EQUAL(std::string("pudding"), req.params[2][0].s());
  CPPUNIT_ASSERT_EQUAL(std::string("hello world"), req.params[2][1].s());
}

void XmlRpcRequestProcessorTest::testParseMemory_shouldFail()
{
  XmlRpcRequestProcessor proc;
  try {
    proc.parseMemory("<methodCall>"
                     "  <methodName>aria2.addURI</methodName>"
                     "    <params>"
                     "      <param>"
                     "        <value><i4>100</i4></value>"
                     "      </param>");
    CPPUNIT_FAIL("exception must be thrown.");
  } catch(RecoverableException& e) {
    // success
  }
}

} // namespace xmlrpc

} // namespace aria2
