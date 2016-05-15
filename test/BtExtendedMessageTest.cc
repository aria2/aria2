#include "BtExtendedMessage.h"

#include <cstring>
#include <iostream>

#include <cppunit/extensions/HelperMacros.h>

#include "bittorrent_helper.h"
#include "MockExtensionMessageFactory.h"
#include "Peer.h"
#include "Exception.h"

namespace aria2 {

class BtExtendedMessageTest : public CppUnit::TestFixture {

  CPPUNIT_TEST_SUITE(BtExtendedMessageTest);
  CPPUNIT_TEST(testCreate);
  CPPUNIT_TEST(testCreateMessage);
  CPPUNIT_TEST(testDoReceivedAction);
  CPPUNIT_TEST(testToString);
  CPPUNIT_TEST_SUITE_END();

private:
public:
  void testCreate();
  void testCreateMessage();
  void testDoReceivedAction();
  void testToString();
};

CPPUNIT_TEST_SUITE_REGISTRATION(BtExtendedMessageTest);

void BtExtendedMessageTest::testCreate()
{
  auto peer = std::make_shared<Peer>("192.168.0.1", 6969);
  peer->allocateSessionResource(1_k, 1_m);
  auto exmsgFactory = MockExtensionMessageFactory{};

  // payload:{4:name3:foo}->11bytes
  std::string payload = "4:name3:foo";
  unsigned char msg[17]; // 6+11bytes
  bittorrent::createPeerMessageString((unsigned char*)msg, sizeof(msg), 13, 20);
  msg[5] = 1; // Set dummy extended message ID 1
  memcpy(msg + 6, payload.c_str(), payload.size());
  auto pm = BtExtendedMessage::create(&exmsgFactory, peer, &msg[4], 13);
  CPPUNIT_ASSERT_EQUAL((uint8_t)20, pm->getId());

  // case: payload size is wrong
  try {
    unsigned char msg[5];
    bittorrent::createPeerMessageString(msg, sizeof(msg), 1, 20);
    BtExtendedMessage::create(&exmsgFactory, peer, &msg[4], 1);
    CPPUNIT_FAIL("exception must be thrown.");
  }
  catch (Exception& e) {
    std::cerr << e.stackTrace() << std::endl;
  }
  // case: id is wrong
  try {
    unsigned char msg[6];
    bittorrent::createPeerMessageString(msg, sizeof(msg), 2, 21);
    BtExtendedMessage::create(&exmsgFactory, peer, &msg[4], 2);
    CPPUNIT_FAIL("exception must be thrown.");
  }
  catch (Exception& e) {
    std::cerr << e.stackTrace() << std::endl;
  }
}

void BtExtendedMessageTest::testCreateMessage()
{
  std::string payload = "4:name3:foo";
  uint8_t extendedMessageID = 1;
  BtExtendedMessage msg{make_unique<MockExtensionMessage>(
      "charlie", extendedMessageID, payload, nullptr)};
  unsigned char data[17];
  bittorrent::createPeerMessageString(data, sizeof(data), 13, 20);
  *(data + 5) = extendedMessageID;
  memcpy(data + 6, payload.c_str(), payload.size());
  auto rawmsg = msg.createMessage();
  CPPUNIT_ASSERT_EQUAL((size_t)17, rawmsg.size());
  CPPUNIT_ASSERT(std::equal(std::begin(rawmsg), std::end(rawmsg), data));
}

void BtExtendedMessageTest::testDoReceivedAction()
{
  auto evcheck = MockExtensionMessageEventCheck{};
  BtExtendedMessage msg{
      make_unique<MockExtensionMessage>("charlie", 1, "", &evcheck)};
  msg.doReceivedAction();
  CPPUNIT_ASSERT(evcheck.doReceivedActionCalled);
}

void BtExtendedMessageTest::testToString()
{
  std::string payload = "4:name3:foo";
  uint8_t extendedMessageID = 1;
  BtExtendedMessage msg{make_unique<MockExtensionMessage>(
      "charlie", extendedMessageID, payload, nullptr)};
  CPPUNIT_ASSERT_EQUAL(std::string("extended charlie"), msg.toString());
}

} // namespace aria2
