#include "HandshakeExtensionMessage.h"
#include "Peer.h"
#include "MockBtContext.h"
#include "Exception.h"
#include "FileEntry.h"
#include <cppunit/extensions/HelperMacros.h>

namespace aria2 {

class HandshakeExtensionMessageTest:public CppUnit::TestFixture {

  CPPUNIT_TEST_SUITE(HandshakeExtensionMessageTest);
  CPPUNIT_TEST(testGetExtensionMessageID);
  CPPUNIT_TEST(testGetExtensionName);
  CPPUNIT_TEST(testGetBencodedData);
  CPPUNIT_TEST(testToString);
  CPPUNIT_TEST(testDoReceivedAction);
  CPPUNIT_TEST(testCreate);
  CPPUNIT_TEST(testCreate_stringnum);
  CPPUNIT_TEST_SUITE_END();
private:
  SharedHandle<BtContext> _btContext;
public:
  HandshakeExtensionMessageTest():_btContext(0) {}

  void setUp() {}

  void tearDown() {}

  void testGetExtensionMessageID();
  void testGetExtensionName();
  void testGetBencodedData();
  void testToString();
  void testDoReceivedAction();
  void testCreate();
  void testCreate_stringnum();
};


CPPUNIT_TEST_SUITE_REGISTRATION(HandshakeExtensionMessageTest);

void HandshakeExtensionMessageTest::testGetExtensionMessageID()
{
  HandshakeExtensionMessage msg;
  CPPUNIT_ASSERT_EQUAL((uint8_t)0, msg.getExtensionMessageID());
}

void HandshakeExtensionMessageTest::testGetExtensionName()
{
  HandshakeExtensionMessage msg;
  CPPUNIT_ASSERT_EQUAL(std::string("handshake"), msg.getExtensionName());
}

void HandshakeExtensionMessageTest::testGetBencodedData()
{
  HandshakeExtensionMessage msg;
  msg.setClientVersion("aria2");
  msg.setTCPPort(6889);
  msg.setExtension("ut_pex", 1);
  msg.setExtension("a2_dht", 2);
  CPPUNIT_ASSERT_EQUAL(std::string("d1:v5:aria21:pi6889e1:md6:a2_dhti2e6:ut_pexi1eee"), msg.getBencodedData());
}

void HandshakeExtensionMessageTest::testToString()
{
  HandshakeExtensionMessage msg;
  msg.setClientVersion("aria2");
  msg.setTCPPort(6889);
  msg.setExtension("ut_pex", 1);
  msg.setExtension("a2_dht", 2);
  CPPUNIT_ASSERT_EQUAL(std::string("handshake client=aria2, tcpPort=6889, a2_dht=2, ut_pex=1"), msg.toString());
}

void HandshakeExtensionMessageTest::testDoReceivedAction()
{
  SharedHandle<Peer> peer = new Peer("192.168.0.1", 0);
  HandshakeExtensionMessage msg;
  msg.setClientVersion("aria2");
  msg.setTCPPort(6889);
  msg.setExtension("ut_pex", 1);
  msg.setExtension("a2_dht", 2);
  msg.setPeer(peer);
  msg.setBtContext(_btContext);

  msg.doReceivedAction();

  CPPUNIT_ASSERT_EQUAL((uint16_t)6889, peer->port);
  CPPUNIT_ASSERT_EQUAL((uint8_t)1, peer->getExtensionMessageID("ut_pex"));
  CPPUNIT_ASSERT_EQUAL((uint8_t)2, peer->getExtensionMessageID("a2_dht"));
}

void HandshakeExtensionMessageTest::testCreate()
{
  std::string in = "0d1:pi6881e1:v5:aria21:md6:ut_pexi1eee";
  SharedHandle<HandshakeExtensionMessage> m =
    HandshakeExtensionMessage::create(in.c_str(), in.size());
  CPPUNIT_ASSERT_EQUAL(std::string("aria2"), m->getClientVersion());
  CPPUNIT_ASSERT_EQUAL((uint16_t)6881, m->getTCPPort());
  CPPUNIT_ASSERT_EQUAL((uint8_t)1, m->getExtensionMessageID("ut_pex"));

  try {
    // bad payload format
    std::string in = "011:hello world";
    HandshakeExtensionMessage::create(in.c_str(), in.size());
    CPPUNIT_FAIL("exception must be thrown.");
  } catch(Exception* e) {
    std::cerr << *e << std::endl;
    delete e;
  }
  try {
    // malformed dencoded message
    std::string in = "011:hello";
    HandshakeExtensionMessage::create(in.c_str(), in.size());
    CPPUNIT_FAIL("exception must be thrown.");
  } catch(Exception* e) {
    std::cerr << *e << std::endl;
    delete e;
  }
  try {
    // 0 length data
    std::string in = "";
    HandshakeExtensionMessage::create(in.c_str(), in.size());
    CPPUNIT_FAIL("exception must be thrown.");
  } catch(Exception* e) {
    std::cerr << *e << std::endl;
    delete e;
  }    
}

void HandshakeExtensionMessageTest::testCreate_stringnum()
{
  std::string in = "0d1:p4:68811:v5:aria21:md6:ut_pex1:1ee";
  SharedHandle<HandshakeExtensionMessage> m =
    HandshakeExtensionMessage::create(in.c_str(), in.size());
  CPPUNIT_ASSERT_EQUAL(std::string("aria2"), m->getClientVersion());
  CPPUNIT_ASSERT_EQUAL((uint16_t)6881, m->getTCPPort());
  CPPUNIT_ASSERT_EQUAL((uint8_t)1, m->getExtensionMessageID("ut_pex"));
}

} // namespace aria2
