#include "DefaultExtensionMessageFactory.h"
#include "Peer.h"
#include "MockBtContext.h"
#include "PeerMessageUtil.h"
#include "HandshakeExtensionMessage.h"
#include "UTPexExtensionMessage.h"
#include "Exception.h"
#include "BtRegistry.h"
#include "BtRuntime.h"
#include "FileEntry.h"
#include <cppunit/extensions/HelperMacros.h>

namespace aria2 {

class DefaultExtensionMessageFactoryTest:public CppUnit::TestFixture {

  CPPUNIT_TEST_SUITE(DefaultExtensionMessageFactoryTest);
  CPPUNIT_TEST(testCreateMessage_unknown);
  CPPUNIT_TEST(testCreateMessage_Handshake);
  CPPUNIT_TEST(testCreateMessage_UTPex);
  CPPUNIT_TEST_SUITE_END();
private:
  SharedHandle<MockBtContext> _btContext;
  SharedHandle<Peer> _peer;
public:
  DefaultExtensionMessageFactoryTest():_btContext(0), _peer(0) {}

  void setUp()
  {
    BtRegistry::unregisterAll();
    SharedHandle<MockBtContext> btContext = new MockBtContext();
    unsigned char infohash[20];
    memset(infohash, 0, sizeof(infohash));
    btContext->setInfoHash(infohash);
    _btContext = btContext;

    SharedHandle<BtRuntime> btRuntime = new BtRuntime();
    BtRegistry::registerBtRuntime(_btContext->getInfoHashAsString(),
				  btRuntime);

    _peer = new Peer("192.168.0.1", 6969);
    _peer->setExtension("ut_pex", 1);
  }

  void tearDown()
  {
    BtRegistry::unregisterAll();
  }

  void testCreateMessage_unknown();
  void testCreateMessage_Handshake();
  void testCreateMessage_UTPex();
};


CPPUNIT_TEST_SUITE_REGISTRATION(DefaultExtensionMessageFactoryTest);

void DefaultExtensionMessageFactoryTest::testCreateMessage_unknown()
{
  DefaultExtensionMessageFactory factory;
  factory.setBtContext(_btContext);
  factory.setPeer(_peer);
  _peer->setExtension("foo", 255);

  char id[1] = { 255 };

  std::string data = std::string(&id[0], &id[1]);
  try {
    // this test fails because localhost doesn't have extension id = 255.
    factory.createMessage(data.c_str(), data.size());
    CPPUNIT_FAIL("exception must be thrown.");
  } catch(Exception* e) {
    std::cerr << *e << std::endl;
    delete e;
  }
}

void DefaultExtensionMessageFactoryTest::testCreateMessage_Handshake()
{
  DefaultExtensionMessageFactory factory;
  factory.setBtContext(_btContext);
  factory.setPeer(_peer);

  char id[1] = { 0 };

  std::string data = std::string(&id[0], &id[1])+"d1:v5:aria2e";
  SharedHandle<HandshakeExtensionMessage> m =
    factory.createMessage(data.c_str(), data.size());
  CPPUNIT_ASSERT_EQUAL(std::string("aria2"), m->getClientVersion());
}

void DefaultExtensionMessageFactoryTest::testCreateMessage_UTPex()
{
  DefaultExtensionMessageFactory factory;
  factory.setBtContext(_btContext);
  factory.setPeer(_peer);
  
  char c1[6];
  char c2[6];
  char c3[6];
  char c4[6];
  PeerMessageUtil::createcompact(c1, "192.168.0.1", 6881);
  PeerMessageUtil::createcompact(c2, "10.1.1.2", 9999);
  PeerMessageUtil::createcompact(c3, "192.168.0.2", 6882);
  PeerMessageUtil::createcompact(c4, "10.1.1.3",10000);

  char id[1] = { factory.getExtensionMessageID("ut_pex") };

  std::string data = std::string(&id[0], &id[1])+"d5:added12:"+
    std::string(&c1[0], &c1[6])+std::string(&c2[0], &c2[6])+
    "7:added.f2:207:dropped12:"+
    std::string(&c3[0], &c3[6])+std::string(&c4[0], &c4[6])+
    "e";

  SharedHandle<UTPexExtensionMessage> m =
    factory.createMessage(data.c_str(), data.size());
  CPPUNIT_ASSERT_EQUAL(factory.getExtensionMessageID("ut_pex"),
		       m->getExtensionMessageID());
}

} // namespace aria2
