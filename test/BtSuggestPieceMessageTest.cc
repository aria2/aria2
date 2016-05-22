#include "BtSuggestPieceMessage.h"

#include <cstring>
#include <cppunit/extensions/HelperMacros.h>

#include "bittorrent_helper.h"

namespace aria2 {

class BtSuggestPieceMessageTest : public CppUnit::TestFixture {

  CPPUNIT_TEST_SUITE(BtSuggestPieceMessageTest);
  CPPUNIT_TEST(testCreate);
  CPPUNIT_TEST(testCreateMessage);
  CPPUNIT_TEST(testToString);
  CPPUNIT_TEST_SUITE_END();

private:
public:
  void setUp() {}

  void testCreate();
  void testCreateMessage();
  void testToString();
};

CPPUNIT_TEST_SUITE_REGISTRATION(BtSuggestPieceMessageTest);

void BtSuggestPieceMessageTest::testCreate()
{
  unsigned char msg[9];
  bittorrent::createPeerMessageString(msg, sizeof(msg), 5, 13);
  bittorrent::setIntParam(&msg[5], 12345);
  auto pm = BtSuggestPieceMessage::create(&msg[4], 5);
  CPPUNIT_ASSERT(BtSuggestPieceMessage::ID == pm->getId());
  CPPUNIT_ASSERT_EQUAL((size_t)12345, pm->getIndex());

  // case: payload size is wrong
  try {
    unsigned char msg[10];
    bittorrent::createPeerMessageString(msg, sizeof(msg), 6, 13);
    BtSuggestPieceMessage::create(&msg[4], 2);
    CPPUNIT_FAIL("exception must be thrown.");
  }
  catch (...) {
  }
  // case: id is wrong
  try {
    unsigned char msg[9];
    bittorrent::createPeerMessageString(msg, sizeof(msg), 5, 14);
    BtSuggestPieceMessage::create(&msg[4], 1);
    CPPUNIT_FAIL("exception must be thrown.");
  }
  catch (...) {
  }
}

void BtSuggestPieceMessageTest::testCreateMessage()
{
  BtSuggestPieceMessage msg;
  msg.setIndex(12345);
  unsigned char data[9];
  bittorrent::createPeerMessageString(data, sizeof(data), 5, 13);
  bittorrent::setIntParam(&data[5], 12345);
  auto rawmsg = msg.createMessage();
  CPPUNIT_ASSERT_EQUAL((size_t)9, rawmsg.size());
  CPPUNIT_ASSERT(std::equal(std::begin(rawmsg), std::end(rawmsg), data));
}

void BtSuggestPieceMessageTest::testToString()
{
  BtSuggestPieceMessage msg;
  msg.setIndex(12345);

  CPPUNIT_ASSERT_EQUAL(std::string("suggest piece index=12345"),
                       msg.toString());
}

} // namespace aria2
