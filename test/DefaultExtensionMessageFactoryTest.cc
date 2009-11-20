#include "DefaultExtensionMessageFactory.h"

#include <iostream>

#include <cppunit/extensions/HelperMacros.h>

#include "Peer.h"
#include "MockPeerStorage.h"
#include "bittorrent_helper.h"
#include "HandshakeExtensionMessage.h"
#include "UTPexExtensionMessage.h"
#include "Exception.h"
#include "FileEntry.h"
#include "ExtensionMessageRegistry.h"
#include "DownloadContext.h"
#include "MockBtMessageDispatcher.h"
#include "MockBtMessageFactory.h"
#include "DownloadContext.h"
#include "BtHandshakeMessage.h"
#include "StringFormat.h"
#include "UTMetadataRequestExtensionMessage.h"
#include "UTMetadataDataExtensionMessage.h"
#include "UTMetadataRejectExtensionMessage.h"

namespace aria2 {

class DefaultExtensionMessageFactoryTest:public CppUnit::TestFixture {

  CPPUNIT_TEST_SUITE(DefaultExtensionMessageFactoryTest);
  CPPUNIT_TEST(testCreateMessage_unknown);
  CPPUNIT_TEST(testCreateMessage_Handshake);
  CPPUNIT_TEST(testCreateMessage_UTPex);
  CPPUNIT_TEST(testCreateMessage_UTMetadataRequest);
  CPPUNIT_TEST(testCreateMessage_UTMetadataData);
  CPPUNIT_TEST(testCreateMessage_UTMetadataReject);
  CPPUNIT_TEST_SUITE_END();
private:
  SharedHandle<MockPeerStorage> _peerStorage;
  SharedHandle<Peer> _peer;
  SharedHandle<DefaultExtensionMessageFactory> _factory;
  SharedHandle<ExtensionMessageRegistry> _registry;
  SharedHandle<MockBtMessageDispatcher> _dispatcher;
  SharedHandle<MockBtMessageFactory> _messageFactory;
  SharedHandle<DownloadContext> _dctx;
public:
  void setUp()
  {
    _peerStorage.reset(new MockPeerStorage());

    _peer.reset(new Peer("192.168.0.1", 6969));
    _peer->allocateSessionResource(1024, 1024*1024);
    _peer->setExtension("ut_pex", 1);

    _registry.reset(new ExtensionMessageRegistry());

    _dispatcher.reset(new MockBtMessageDispatcher());

    _messageFactory.reset(new MockBtMessageFactory());

    _dctx.reset(new DownloadContext());

    _factory.reset(new DefaultExtensionMessageFactory());
    _factory->setPeerStorage(_peerStorage);
    _factory->setPeer(_peer);
    _factory->setExtensionMessageRegistry(_registry);
    _factory->setBtMessageDispatcher(_dispatcher);
    _factory->setBtMessageFactory(_messageFactory);
    _factory->setDownloadContext(_dctx);
  }

  std::string getExtensionMessageID(const std::string& name)
  {
    char id[1] = { _registry->getExtensionMessageID(name) };
    return std::string(&id[0], &id[1]);
  }

  template<typename T>
  SharedHandle<T> createMessage(const std::string& data)
  {
    return dynamic_pointer_cast<T>
      (_factory->createMessage
       (reinterpret_cast<const unsigned char*>(data.c_str()), data.size()));
  }

  void testCreateMessage_unknown();
  void testCreateMessage_Handshake();
  void testCreateMessage_UTPex();
  void testCreateMessage_UTMetadataRequest();
  void testCreateMessage_UTMetadataData();
  void testCreateMessage_UTMetadataReject();
};


CPPUNIT_TEST_SUITE_REGISTRATION(DefaultExtensionMessageFactoryTest);

void DefaultExtensionMessageFactoryTest::testCreateMessage_unknown()
{
  _peer->setExtension("foo", 255);

  char id[1] = { 255 };

  std::string data = std::string(&id[0], &id[1]);
  try {
    // this test fails because localhost doesn't have extension id = 255.
    _factory->createMessage
      (reinterpret_cast<const unsigned char*>(data.c_str()), data.size());
    CPPUNIT_FAIL("exception must be thrown.");
  } catch(Exception& e) {
    std::cerr << e.stackTrace() << std::endl;
  }
}

void DefaultExtensionMessageFactoryTest::testCreateMessage_Handshake()
{
  char id[1] = { 0 };

  std::string data = std::string(&id[0], &id[1])+"d1:v5:aria2e";
  SharedHandle<HandshakeExtensionMessage> m =
    createMessage<HandshakeExtensionMessage>(data);
  CPPUNIT_ASSERT_EQUAL(std::string("aria2"), m->getClientVersion());
}

void DefaultExtensionMessageFactoryTest::testCreateMessage_UTPex()
{
  unsigned char c1[6];
  unsigned char c2[6];
  unsigned char c3[6];
  unsigned char c4[6];
  bittorrent::createcompact(c1, "192.168.0.1", 6881);
  bittorrent::createcompact(c2, "10.1.1.2", 9999);
  bittorrent::createcompact(c3, "192.168.0.2", 6882);
  bittorrent::createcompact(c4, "10.1.1.3",10000);

  std::string data = getExtensionMessageID("ut_pex")+"d5:added12:"+
    std::string(&c1[0], &c1[6])+std::string(&c2[0], &c2[6])+
    "7:added.f2:207:dropped12:"+
    std::string(&c3[0], &c3[6])+std::string(&c4[0], &c4[6])+
    "e";

  SharedHandle<UTPexExtensionMessage> m =
    createMessage<UTPexExtensionMessage>(data);
  CPPUNIT_ASSERT_EQUAL(_registry->getExtensionMessageID("ut_pex"),
		       m->getExtensionMessageID());
}

void DefaultExtensionMessageFactoryTest::testCreateMessage_UTMetadataRequest()
{
  std::string data = getExtensionMessageID("ut_metadata")+
    "d8:msg_typei0e5:piecei1ee";
  SharedHandle<UTMetadataRequestExtensionMessage> m =
    createMessage<UTMetadataRequestExtensionMessage>(data);
  CPPUNIT_ASSERT_EQUAL((size_t)1, m->getIndex());
}

void DefaultExtensionMessageFactoryTest::testCreateMessage_UTMetadataData()
{
  std::string data = getExtensionMessageID("ut_metadata")+
    "d8:msg_typei1e5:piecei1e10:total_sizei300ee10:0000000000";
  SharedHandle<UTMetadataDataExtensionMessage> m =
    createMessage<UTMetadataDataExtensionMessage>(data);
  CPPUNIT_ASSERT_EQUAL((size_t)1, m->getIndex());
  CPPUNIT_ASSERT_EQUAL((size_t)300, m->getTotalSize());
  CPPUNIT_ASSERT_EQUAL(std::string(10, '0'), m->getData());
}

void DefaultExtensionMessageFactoryTest::testCreateMessage_UTMetadataReject()
{
  std::string data = getExtensionMessageID("ut_metadata")+
    "d8:msg_typei2e5:piecei1ee";
  SharedHandle<UTMetadataRejectExtensionMessage> m =
    createMessage<UTMetadataRejectExtensionMessage>(data);
  CPPUNIT_ASSERT_EQUAL((size_t)1, m->getIndex());
}

} // namespace aria2
