#include "HandshakeExtensionMessage.h"

#include <iostream>

#include <cppunit/extensions/HelperMacros.h>

#include "Peer.h"
#include "Exception.h"
#include "FileEntry.h"
#include "DownloadContext.h"
#include "bittorrent_helper.h"
#include "Option.h"
#include "RequestGroup.h"

namespace aria2 {

class HandshakeExtensionMessageTest : public CppUnit::TestFixture {

  CPPUNIT_TEST_SUITE(HandshakeExtensionMessageTest);
  CPPUNIT_TEST(testGetExtensionMessageID);
  CPPUNIT_TEST(testGetExtensionName);
  CPPUNIT_TEST(testGetBencodedData);
  CPPUNIT_TEST(testToString);
  CPPUNIT_TEST(testDoReceivedAction);
  CPPUNIT_TEST(testCreate);
  CPPUNIT_TEST(testCreate_stringnum);
  CPPUNIT_TEST_SUITE_END();

public:
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
  CPPUNIT_ASSERT_EQUAL(std::string("handshake"),
                       std::string(msg.getExtensionName()));
}

void HandshakeExtensionMessageTest::testGetBencodedData()
{
  HandshakeExtensionMessage msg;
  msg.setClientVersion("aria2");
  msg.setTCPPort(6889);
  msg.setExtension(ExtensionMessageRegistry::UT_PEX, 1);
  msg.setExtension(ExtensionMessageRegistry::UT_METADATA, 2);
  msg.setMetadataSize(1_k);
  CPPUNIT_ASSERT_EQUAL(std::string("d"
                                   "1:md11:ut_metadatai2e6:ut_pexi1ee"
                                   "13:metadata_sizei1024e"
                                   "1:pi6889e"
                                   "1:v5:aria2"
                                   "e"),
                       msg.getPayload());

  msg.setMetadataSize(0);
  CPPUNIT_ASSERT(msg.getPayload().find("metadata_size") == std::string::npos);
}

void HandshakeExtensionMessageTest::testToString()
{
  HandshakeExtensionMessage msg;
  msg.setClientVersion("aria2");
  msg.setTCPPort(6889);
  msg.setExtension(ExtensionMessageRegistry::UT_PEX, 1);
  msg.setExtension(ExtensionMessageRegistry::UT_METADATA, 2);
  msg.setMetadataSize(1_k);
  CPPUNIT_ASSERT_EQUAL(
      std::string("handshake client=aria2, tcpPort=6889, metadataSize=1024,"
                  " ut_metadata=2, ut_pex=1"),
      msg.toString());
}

void HandshakeExtensionMessageTest::testDoReceivedAction()
{
  auto dctx = std::make_shared<DownloadContext>(METADATA_PIECE_SIZE, 0);
  auto op = std::make_shared<Option>();
  RequestGroup rg(GroupId::create(), op);
  rg.setDownloadContext(dctx);

  dctx->setAttribute(CTX_ATTR_BT, make_unique<TorrentAttribute>());
  dctx->markTotalLengthIsUnknown();

  auto peer = std::make_shared<Peer>("192.168.0.1", 0);
  peer->allocateSessionResource(1_k, 1_m);
  HandshakeExtensionMessage msg;
  msg.setClientVersion("aria2");
  msg.setTCPPort(6889);
  msg.setExtension(ExtensionMessageRegistry::UT_PEX, 1);
  msg.setExtension(ExtensionMessageRegistry::UT_METADATA, 3);
  msg.setMetadataSize(1_k);
  msg.setPeer(peer);
  msg.setDownloadContext(dctx.get());

  msg.doReceivedAction();

  CPPUNIT_ASSERT_EQUAL((uint16_t)6889, peer->getPort());
  CPPUNIT_ASSERT_EQUAL((uint8_t)1, peer->getExtensionMessageID(
                                       ExtensionMessageRegistry::UT_PEX));
  CPPUNIT_ASSERT_EQUAL((uint8_t)3, peer->getExtensionMessageID(
                                       ExtensionMessageRegistry::UT_METADATA));
  CPPUNIT_ASSERT(peer->isSeeder());
  auto attrs = bittorrent::getTorrentAttrs(dctx);
  CPPUNIT_ASSERT_EQUAL((size_t)1_k, attrs->metadataSize);
  CPPUNIT_ASSERT_EQUAL((int64_t)1_k, dctx->getTotalLength());
  CPPUNIT_ASSERT(dctx->knowsTotalLength());

  // See Peer is not marked as seeder if !attrs->metadata.empty()
  peer->allocateSessionResource(1_k, 1_m);
  attrs->metadataSize = 1_k;
  attrs->metadata = std::string('0', attrs->metadataSize);
  msg.doReceivedAction();
  CPPUNIT_ASSERT(!peer->isSeeder());
}

void HandshakeExtensionMessageTest::testCreate()
{
  std::string in =
      "0d1:pi6881e1:v5:aria21:md5:a2dhti2e6:ut_pexi1ee13:metadata_sizei1024ee";
  std::shared_ptr<HandshakeExtensionMessage> m(
      HandshakeExtensionMessage::create(
          reinterpret_cast<const unsigned char*>(in.c_str()), in.size()));
  CPPUNIT_ASSERT_EQUAL(std::string("aria2"), m->getClientVersion());
  CPPUNIT_ASSERT_EQUAL((uint16_t)6881, m->getTCPPort());
  CPPUNIT_ASSERT_EQUAL(
      (uint8_t)1, m->getExtensionMessageID(ExtensionMessageRegistry::UT_PEX));
  CPPUNIT_ASSERT_EQUAL((size_t)1_k, m->getMetadataSize());
  try {
    // bad payload format
    std::string in = "011:hello world";
    HandshakeExtensionMessage::create(
        reinterpret_cast<const unsigned char*>(in.c_str()), in.size());
    CPPUNIT_FAIL("exception must be thrown.");
  }
  catch (Exception& e) {
    std::cerr << e.stackTrace() << std::endl;
  }
  try {
    // malformed dencoded message
    std::string in = "011:hello";
    HandshakeExtensionMessage::create(
        reinterpret_cast<const unsigned char*>(in.c_str()), in.size());
    CPPUNIT_FAIL("exception must be thrown.");
  }
  catch (Exception& e) {
    std::cerr << e.stackTrace() << std::endl;
  }
  try {
    // 0 length data
    std::string in = "";
    HandshakeExtensionMessage::create(
        reinterpret_cast<const unsigned char*>(in.c_str()), in.size());
    CPPUNIT_FAIL("exception must be thrown.");
  }
  catch (Exception& e) {
    std::cerr << e.stackTrace() << std::endl;
  }
}

void HandshakeExtensionMessageTest::testCreate_stringnum()
{
  std::string in = "0d1:p4:68811:v5:aria21:md6:ut_pex1:1ee";
  std::shared_ptr<HandshakeExtensionMessage> m(
      HandshakeExtensionMessage::create(
          reinterpret_cast<const unsigned char*>(in.c_str()), in.size()));
  CPPUNIT_ASSERT_EQUAL(std::string("aria2"), m->getClientVersion());
  // port number in string is not allowed
  CPPUNIT_ASSERT_EQUAL((uint16_t)0, m->getTCPPort());
  // extension ID in string is not allowed
  CPPUNIT_ASSERT_EQUAL(
      (uint8_t)0, m->getExtensionMessageID(ExtensionMessageRegistry::UT_PEX));
}

} // namespace aria2
