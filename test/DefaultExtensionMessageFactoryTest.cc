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
#include "UTMetadataRequestExtensionMessage.h"
#include "UTMetadataDataExtensionMessage.h"
#include "UTMetadataRejectExtensionMessage.h"
#include "BtRuntime.h"
#include "PieceStorage.h"
#include "RequestGroup.h"
#include "Option.h"

namespace aria2 {

class DefaultExtensionMessageFactoryTest : public CppUnit::TestFixture {

  CPPUNIT_TEST_SUITE(DefaultExtensionMessageFactoryTest);
  CPPUNIT_TEST(testCreateMessage_unknown);
  CPPUNIT_TEST(testCreateMessage_Handshake);
  CPPUNIT_TEST(testCreateMessage_UTPex);
  CPPUNIT_TEST(testCreateMessage_UTMetadataRequest);
  CPPUNIT_TEST(testCreateMessage_UTMetadataData);
  CPPUNIT_TEST(testCreateMessage_UTMetadataReject);
  CPPUNIT_TEST_SUITE_END();

private:
  std::unique_ptr<MockPeerStorage> peerStorage_;
  std::shared_ptr<Peer> peer_;
  std::unique_ptr<DefaultExtensionMessageFactory> factory_;
  std::unique_ptr<ExtensionMessageRegistry> registry_;
  std::unique_ptr<MockBtMessageDispatcher> dispatcher_;
  std::unique_ptr<MockBtMessageFactory> messageFactory_;
  std::shared_ptr<DownloadContext> dctx_;
  std::unique_ptr<RequestGroup> requestGroup_;

public:
  void setUp()
  {
    peerStorage_ = make_unique<MockPeerStorage>();

    peer_ = std::make_shared<Peer>("192.168.0.1", 6969);
    peer_->allocateSessionResource(1_k, 1_m);
    peer_->setExtension(ExtensionMessageRegistry::UT_PEX, 1);

    registry_ = make_unique<ExtensionMessageRegistry>();
    dispatcher_ = make_unique<MockBtMessageDispatcher>();
    messageFactory_ = make_unique<MockBtMessageFactory>();
    dctx_ = std::make_shared<DownloadContext>();
    auto option = std::make_shared<Option>();
    requestGroup_ = make_unique<RequestGroup>(GroupId::create(), option);
    requestGroup_->setDownloadContext(dctx_);

    factory_ = make_unique<DefaultExtensionMessageFactory>();
    factory_->setPeerStorage(peerStorage_.get());
    factory_->setPeer(peer_);
    factory_->setExtensionMessageRegistry(registry_.get());
    factory_->setBtMessageDispatcher(dispatcher_.get());
    factory_->setBtMessageFactory(messageFactory_.get());
    factory_->setDownloadContext(dctx_.get());
  }

  std::string getExtensionMessageID(int key)
  {
    unsigned char id[1] = {registry_->getExtensionMessageID(key)};
    return std::string(&id[0], &id[1]);
  }

  template <typename T>
  std::shared_ptr<T> createMessage(const std::string& data)
  {
    auto m = factory_->createMessage(
        reinterpret_cast<const unsigned char*>(data.c_str()), data.size());
    return std::dynamic_pointer_cast<T>(
        std::shared_ptr<T>{static_cast<T*>(m.release())});
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
  peer_->setExtension(ExtensionMessageRegistry::UT_PEX, 255);

  unsigned char id[1] = {255};

  std::string data = std::string(&id[0], &id[1]);
  try {
    // this test fails because localhost doesn't have extension id = 255.
    factory_->createMessage(
        reinterpret_cast<const unsigned char*>(data.c_str()), data.size());
    CPPUNIT_FAIL("exception must be thrown.");
  }
  catch (Exception& e) {
    std::cerr << e.stackTrace() << std::endl;
  }
}

void DefaultExtensionMessageFactoryTest::testCreateMessage_Handshake()
{
  char id[1] = {0};

  std::string data = std::string(&id[0], &id[1]) + "d1:v5:aria2e";
  auto m = createMessage<HandshakeExtensionMessage>(data);
  CPPUNIT_ASSERT_EQUAL(std::string("aria2"), m->getClientVersion());
}

void DefaultExtensionMessageFactoryTest::testCreateMessage_UTPex()
{
  unsigned char c1[COMPACT_LEN_IPV6];
  unsigned char c2[COMPACT_LEN_IPV6];
  unsigned char c3[COMPACT_LEN_IPV6];
  unsigned char c4[COMPACT_LEN_IPV6];
  bittorrent::packcompact(c1, "192.168.0.1", 6881);
  bittorrent::packcompact(c2, "10.1.1.2", 9999);
  bittorrent::packcompact(c3, "192.168.0.2", 6882);
  bittorrent::packcompact(c4, "10.1.1.3", 10000);

  registry_->setExtensionMessageID(ExtensionMessageRegistry::UT_PEX, 1);

  std::string data = getExtensionMessageID(ExtensionMessageRegistry::UT_PEX) +
                     "d5:added12:" + std::string(&c1[0], &c1[6]) +
                     std::string(&c2[0], &c2[6]) +
                     "7:added.f2:207:dropped12:" + std::string(&c3[0], &c3[6]) +
                     std::string(&c4[0], &c4[6]) + "e";

  auto m = createMessage<UTPexExtensionMessage>(data);
  CPPUNIT_ASSERT_EQUAL(
      registry_->getExtensionMessageID(ExtensionMessageRegistry::UT_PEX),
      m->getExtensionMessageID());
}

void DefaultExtensionMessageFactoryTest::testCreateMessage_UTMetadataRequest()
{
  registry_->setExtensionMessageID(ExtensionMessageRegistry::UT_METADATA, 1);

  std::string data =
      getExtensionMessageID(ExtensionMessageRegistry::UT_METADATA) +
      "d8:msg_typei0e5:piecei1ee";
  auto m = createMessage<UTMetadataRequestExtensionMessage>(data);
  CPPUNIT_ASSERT_EQUAL((size_t)1, m->getIndex());
}

void DefaultExtensionMessageFactoryTest::testCreateMessage_UTMetadataData()
{
  registry_->setExtensionMessageID(ExtensionMessageRegistry::UT_METADATA, 1);

  std::string data =
      getExtensionMessageID(ExtensionMessageRegistry::UT_METADATA) +
      "d8:msg_typei1e5:piecei1e10:total_sizei300ee0000000000";
  auto m = createMessage<UTMetadataDataExtensionMessage>(data);
  CPPUNIT_ASSERT_EQUAL((size_t)1, m->getIndex());
  CPPUNIT_ASSERT_EQUAL((size_t)300, m->getTotalSize());
  CPPUNIT_ASSERT_EQUAL(std::string(10, '0'), m->getData());
}

void DefaultExtensionMessageFactoryTest::testCreateMessage_UTMetadataReject()
{
  registry_->setExtensionMessageID(ExtensionMessageRegistry::UT_METADATA, 1);

  std::string data =
      getExtensionMessageID(ExtensionMessageRegistry::UT_METADATA) +
      "d8:msg_typei2e5:piecei1ee";
  auto m = createMessage<UTMetadataRejectExtensionMessage>(data);
  CPPUNIT_ASSERT_EQUAL((size_t)1, m->getIndex());
}

} // namespace aria2
