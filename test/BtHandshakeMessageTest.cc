#include "BtHandshakeMessage.h"
#include "PeerMessageUtil.h"
#include "Util.h"
#include <cppunit/extensions/HelperMacros.h>

using namespace std;

class BtHandshakeMessageTest:public CppUnit::TestFixture {

  CPPUNIT_TEST_SUITE(BtHandshakeMessageTest);
  CPPUNIT_TEST(testCreate);
  CPPUNIT_TEST(testGetMessage);
  CPPUNIT_TEST(testToString);
  CPPUNIT_TEST_SUITE_END();
private:

public:
  void setUp() {
  }

  void testCreate();
  void testGetMessage();
  void testToString();

  static string BTPSTR;
};


string BtHandshakeMessageTest::BTPSTR = "BitTorrent protocol";

CPPUNIT_TEST_SUITE_REGISTRATION(BtHandshakeMessageTest);

void createHandshakeMessageData(unsigned char* msg) {
  msg[0] = 19;
  memcpy(&msg[1], BtHandshakeMessageTest::BTPSTR.c_str(),
	 BtHandshakeMessageTest::BTPSTR.size());
  unsigned char reserved[] = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x04 };
  memcpy(&msg[20], reserved, sizeof(reserved));
  unsigned char infoHash[] = { 0xff, 0xff, 0xff, 0xff, 0xff,
			       0xff, 0xff, 0xff, 0xff, 0xff,
			       0xff, 0xff, 0xff, 0xff, 0xff,
			       0xff, 0xff, 0xff, 0xff, 0xff };
  memcpy(&msg[28], infoHash, sizeof(infoHash));
  unsigned char peerId[] = { 0xf0, 0xf0, 0xf0, 0xf0, 0xf0,
			     0xf0, 0xf0, 0xf0, 0xf0, 0xf0,
			     0xf0, 0xf0, 0xf0, 0xf0, 0xf0,
			     0xf0, 0xf0, 0xf0, 0xf0, 0xf0 };
  memcpy(&msg[48], peerId, sizeof(peerId));
}

void BtHandshakeMessageTest::testCreate() {
  unsigned char msg[68];
  createHandshakeMessageData(msg);
  BtHandshakeMessageHandle message = BtHandshakeMessage::create(&msg[0], sizeof(msg));
  CPPUNIT_ASSERT_EQUAL(INT32_MAX, message->getId());
  CPPUNIT_ASSERT_EQUAL((uint8_t)19, message->getPstrlen());
  CPPUNIT_ASSERT_EQUAL(Util::toHex((const unsigned char*)BTPSTR.c_str(), BTPSTR.size()),
		       Util::toHex(message->getPstr(), BtHandshakeMessage::PSTR_LENGTH));
  CPPUNIT_ASSERT_EQUAL(string("0000000000000004"),
		       Util::toHex(message->getReserved(), BtHandshakeMessage::RESERVED_LENGTH));
  CPPUNIT_ASSERT_EQUAL(string("ffffffffffffffffffffffffffffffffffffffff"),
		       Util::toHex(message->getInfoHash(), INFO_HASH_LENGTH));
  CPPUNIT_ASSERT_EQUAL(string("f0f0f0f0f0f0f0f0f0f0f0f0f0f0f0f0f0f0f0f0"),
		       Util::toHex(message->getPeerId(), PEER_ID_LENGTH));
}

void BtHandshakeMessageTest::testGetMessage() {
  unsigned char infoHash[] = { 0xff, 0xff, 0xff, 0xff, 0xff,
			       0xff, 0xff, 0xff, 0xff, 0xff,
			       0xff, 0xff, 0xff, 0xff, 0xff,
			       0xff, 0xff, 0xff, 0xff, 0xff };
  unsigned char peerId[] = { 0xf0, 0xf0, 0xf0, 0xf0, 0xf0,
			     0xf0, 0xf0, 0xf0, 0xf0, 0xf0,
			     0xf0, 0xf0, 0xf0, 0xf0, 0xf0,
			     0xf0, 0xf0, 0xf0, 0xf0, 0xf0 };

  BtHandshakeMessageHandle msg = new BtHandshakeMessage();
  msg->setInfoHash(infoHash);
  msg->setPeerId(peerId);

  unsigned char data[68];
  createHandshakeMessageData(data);
  CPPUNIT_ASSERT_EQUAL(Util::toHex((const unsigned char*)data, 68),
		       Util::toHex((const unsigned char*)msg->getMessage(), 68));
}

void BtHandshakeMessageTest::testToString() {
  unsigned char infoHash[] = { 0xff, 0xff, 0xff, 0xff, 0xff,
			       0xff, 0xff, 0xff, 0xff, 0xff,
			       0xff, 0xff, 0xff, 0xff, 0xff,
			       0xff, 0xff, 0xff, 0xff, 0xff };
  unsigned char peerId[] = { 0xf0, 0xf0, 0xf0, 0xf0, 0xf0,
			     0xf0, 0xf0, 0xf0, 0xf0, 0xf0,
			     0xf0, 0xf0, 0xf0, 0xf0, 0xf0,
			     0xf0, 0xf0, 0xf0, 0xf0, 0xf0 };

  BtHandshakeMessage msg;
  msg.setInfoHash(infoHash);
  msg.setPeerId(peerId);

  CPPUNIT_ASSERT_EQUAL(string("handshake peerId=%f0%f0%f0%f0%f0%f0%f0%f0%f0%f0%f0%f0%f0%f0%f0%f0%f0%f0%f0%f0, reserved=0000000000000004"), msg.toString());
}
