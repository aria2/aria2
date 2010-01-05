#include "UTMetadataRejectExtensionMessage.h"

#include <iostream>

#include <cppunit/extensions/HelperMacros.h>

#include "BtConstants.h"
#include "DlAbortEx.h"

namespace aria2 {

class UTMetadataRejectExtensionMessageTest:public CppUnit::TestFixture {

  CPPUNIT_TEST_SUITE(UTMetadataRejectExtensionMessageTest);
  CPPUNIT_TEST(testGetExtensionMessageID);
  CPPUNIT_TEST(testGetBencodedReject);
  CPPUNIT_TEST(testToString);
  CPPUNIT_TEST(testDoReceivedAction);
  CPPUNIT_TEST_SUITE_END();
public:
  void testGetExtensionMessageID();
  void testGetBencodedReject();
  void testToString();
  void testDoReceivedAction();
};


CPPUNIT_TEST_SUITE_REGISTRATION(UTMetadataRejectExtensionMessageTest);

void UTMetadataRejectExtensionMessageTest::testGetExtensionMessageID()
{
  UTMetadataRejectExtensionMessage msg(1);
  CPPUNIT_ASSERT_EQUAL((uint8_t)1, msg.getExtensionMessageID());
}

void UTMetadataRejectExtensionMessageTest::testGetBencodedReject()
{
  UTMetadataRejectExtensionMessage msg(1);
  msg.setIndex(1);
  CPPUNIT_ASSERT_EQUAL
    (std::string("d8:msg_typei2e5:piecei1ee"), msg.getPayload());
}

void UTMetadataRejectExtensionMessageTest::testToString()
{
  UTMetadataRejectExtensionMessage msg(1);
  msg.setIndex(100);
  CPPUNIT_ASSERT_EQUAL(std::string("ut_metadata reject piece=100"),
                       msg.toString());
}

void UTMetadataRejectExtensionMessageTest::testDoReceivedAction()
{
  UTMetadataRejectExtensionMessage msg(1);
  msg.setIndex(0);
  try {
    msg.doReceivedAction();
    CPPUNIT_FAIL("exception must be thrown.");
  } catch(DlAbortEx& e) {
    // success
  }
}

} // namespace aria2
