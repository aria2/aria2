#include "XmlRpcRequestProcessor.h"

#include <cppunit/extensions/HelperMacros.h>

#include "XmlRpcRequestParserStateMachine.h"
#include "RecoverableException.h"

namespace aria2 {

namespace rpc {

class XmlRpcRequestProcessorTest:public CppUnit::TestFixture {

  CPPUNIT_TEST_SUITE(XmlRpcRequestProcessorTest);
  CPPUNIT_TEST(testParseMemory);
  CPPUNIT_TEST(testParseMemory_shouldFail);
  CPPUNIT_TEST(testParseMemory_withoutStringTag);
  CPPUNIT_TEST_SUITE_END();
public:
  void setUp() {}

  void tearDown() {}

  void testParseMemory();
  void testParseMemory_shouldFail();
  void testParseMemory_withoutStringTag();
};


CPPUNIT_TEST_SUITE_REGISTRATION(XmlRpcRequestProcessorTest);

void XmlRpcRequestProcessorTest::testParseMemory()
{
  XmlRpcRequestProcessor proc;
  RpcRequest req =
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
  CPPUNIT_ASSERT_EQUAL((size_t)3, req.params->size());
  CPPUNIT_ASSERT_EQUAL((Integer::ValueType)100,
                       asInteger(req.params->get(0))->i());
  const Dict* dict = asDict(req.params->get(1));
  CPPUNIT_ASSERT_EQUAL((Integer::ValueType)65535,
                       asInteger(dict->get("max-count"))->i());
  // Current implementation handles double as string.
  CPPUNIT_ASSERT_EQUAL(std::string("0.99"),
                       asString(dict->get("seed-ratio"))->s());
  const List* list = asList(req.params->get(2));
  CPPUNIT_ASSERT_EQUAL(std::string("pudding"), asString(list->get(0))->s());
  CPPUNIT_ASSERT_EQUAL(std::string("hello world"), asString(list->get(1))->s());
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
  {
    RpcRequest req =
      proc.parseMemory("<methodCall>"
                       "  <methodName>aria2.addURI</methodName>"
                     "    <params>"
                     "    </params>"
                       "</methodCall>");
    CPPUNIT_ASSERT(req.params);
  }
  try {
    RpcRequest req =
      proc.parseMemory("<methodCall>"
                       "  <methodName>aria2.addURI</methodName>"
                       "</methodCall>");
    CPPUNIT_FAIL("exception must be thrown.");
  } catch(RecoverableException& e) {
    // success
  }
}

void XmlRpcRequestProcessorTest::testParseMemory_withoutStringTag()
{
  XmlRpcRequestProcessor proc;
  RpcRequest req =
    proc.parseMemory("<?xml version=\"1.0\"?>"
                     "<methodCall>"
                     "  <methodName>aria2.addUri</methodName>"
                     "    <params>"
                     "      <param>"
                     "        <value>http://aria2.sourceforge.net</value>"
                     "      </param>"
                     "      <param>"
                     "        <value>http://aria2.<foo/>sourceforge.net</value>"
                     "      </param>"
                     "      <param>"
                     "       <value>"
                     "         <struct>"
                     "           <member>"
                     "             <name>hello</name>"
                     "             <value>world</value>"
                     "           </member>"
                     "         </struct>"
                     "       </value>"
                     "     </param>"
                     "     <param>"
                     "       <value>"
                     "         <array>"
                     "           <data>"
                     "             <value>apple</value>"
                     "             <value>banana</value>"
                     "             <value><string>lemon</string>peanuts</value>"
                     "           </data>"
                     "         </array>"
                     "       </value>"
                     "     </param>"
                     "   </params>"
                     "</methodCall>");

  CPPUNIT_ASSERT_EQUAL((size_t)4, req.params->size());
  CPPUNIT_ASSERT_EQUAL(std::string("http://aria2.sourceforge.net"),
                       asString(req.params->get(0))->s());
  CPPUNIT_ASSERT_EQUAL(std::string("http://aria2.sourceforge.net"),
                       asString(req.params->get(1))->s());
  const Dict* dict = asDict(req.params->get(2));
  CPPUNIT_ASSERT_EQUAL(std::string("world"),
                       asString(dict->get("hello"))->s());
  const List* list = asList(req.params->get(3));
  CPPUNIT_ASSERT_EQUAL(std::string("apple"), asString(list->get(0))->s());
  CPPUNIT_ASSERT_EQUAL(std::string("banana"), asString(list->get(1))->s());
  CPPUNIT_ASSERT_EQUAL(std::string("lemon"), asString(list->get(2))->s());
}

} // namespace rpc

} // namespace aria2
