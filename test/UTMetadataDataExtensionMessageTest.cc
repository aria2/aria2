#include "UTMetadataDataExtensionMessage.h"

#include <iostream>

#include <cppunit/extensions/HelperMacros.h>

#include "BtConstants.h"

namespace aria2 {

class UTMetadataDataExtensionMessageTest:public CppUnit::TestFixture {

  CPPUNIT_TEST_SUITE(UTMetadataDataExtensionMessageTest);
  CPPUNIT_TEST(testGetExtensionMessageID);
  CPPUNIT_TEST(testGetBencodedData);
  CPPUNIT_TEST(testToString);
  CPPUNIT_TEST(testDoReceivedAction);
  CPPUNIT_TEST_SUITE_END();
public:
  void testGetExtensionMessageID();
  void testGetBencodedData();
  void testToString();
  void testDoReceivedAction();
};


CPPUNIT_TEST_SUITE_REGISTRATION(UTMetadataDataExtensionMessageTest);

void UTMetadataDataExtensionMessageTest::testGetExtensionMessageID()
{
  UTMetadataDataExtensionMessage msg(1);
  CPPUNIT_ASSERT_EQUAL((uint8_t)1, msg.getExtensionMessageID());
}

void UTMetadataDataExtensionMessageTest::testGetBencodedData()
{
  std::string data(METADATA_PIECE_SIZE, '0');

  UTMetadataDataExtensionMessage msg(1);
  msg.setIndex(1);
  msg.setTotalSize(data.size());
  msg.setData(data);
  CPPUNIT_ASSERT_EQUAL
    (std::string("d8:msg_typei1e5:piecei1e10:total_sizei16384ee16384:")+data,
     msg.getBencodedData());
}

void UTMetadataDataExtensionMessageTest::testToString()
{
  UTMetadataDataExtensionMessage msg(1);
  msg.setIndex(100);
  CPPUNIT_ASSERT_EQUAL(std::string("ut_metadata data piece=100"),
		       msg.toString());
}

void UTMetadataDataExtensionMessageTest::testDoReceivedAction()
{
}

} // namespace aria2
