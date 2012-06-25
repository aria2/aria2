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
  CPPUNIT_ASSERT_EQUAL(std::string("handshake"), msg.getExtensionName());
}

void HandshakeExtensionMessageTest::testGetBencodedData()
{
  HandshakeExtensionMessage msg;
  msg.setClientVersion("aria2");
  msg.setTCPPort(6889);
  msg.setExtension("ut_pex", 1);
  msg.setExtension("a2_dht", 2);
  msg.setMetadataSize(1024);
  CPPUNIT_ASSERT_EQUAL
    (std::string("d"
                 "1:md6:a2_dhti2e6:ut_pexi1ee"
                 "13:metadata_sizei1024e"
                 "1:pi6889e"
                 "1:v5:aria2"
                 "e"), msg.getPayload());

  msg.setMetadataSize(0);
  CPPUNIT_ASSERT
    (msg.getPayload().find("metadata_size") == std::string::npos);
}

void HandshakeExtensionMessageTest::testToString()
{
  HandshakeExtensionMessage msg;
  msg.setClientVersion("aria2");
  msg.setTCPPort(6889);
  msg.setExtension("ut_pex", 1);
  msg.setExtension("a2_dht", 2);
  msg.setMetadataSize(1024);
  CPPUNIT_ASSERT_EQUAL
    (std::string("handshake client=aria2, tcpPort=6889, metadataSize=1024,"
                 " a2_dht=2, ut_pex=1"), msg.toString());
}

void HandshakeExtensionMessageTest::testDoReceivedAction()
{
  SharedHandle<DownloadContext> dctx
    (new DownloadContext(METADATA_PIECE_SIZE, 0));
  SharedHandle<Option> op(new Option());
  RequestGroup rg(op);
  rg.setDownloadContext(dctx);

  SharedHandle<TorrentAttribute> attrs(new TorrentAttribute());
  dctx->setAttribute(bittorrent::BITTORRENT, attrs);
  dctx->markTotalLengthIsUnknown();

  SharedHandle<Peer> peer(new Peer("192.168.0.1", 0));
  peer->allocateSessionResource(1024, 1024*1024);
  HandshakeExtensionMessage msg;
  msg.setClientVersion("aria2");
  msg.setTCPPort(6889);
  msg.setExtension("ut_pex", 1);
  msg.setExtension("a2_dht", 2);
  msg.setExtension("ut_metadata", 3);
  msg.setMetadataSize(1024);
  msg.setPeer(peer);
  msg.setDownloadContext(dctx);

  msg.doReceivedAction();

  CPPUNIT_ASSERT_EQUAL((uint16_t)6889, peer->getPort());
  CPPUNIT_ASSERT_EQUAL((uint8_t)1, peer->getExtensionMessageID("ut_pex"));
  CPPUNIT_ASSERT_EQUAL((uint8_t)2, peer->getExtensionMessageID("a2_dht"));
  CPPUNIT_ASSERT(peer->isSeeder());
  CPPUNIT_ASSERT_EQUAL((size_t)1024, attrs->metadataSize);
  CPPUNIT_ASSERT_EQUAL((int64_t)1024, dctx->getTotalLength());
  CPPUNIT_ASSERT(dctx->knowsTotalLength());

  // See Peer is not marked as seeder if !attrs->metadata.empty()
  peer->allocateSessionResource(1024, 1024*1024);
  attrs->metadataSize = 1024;
  attrs->metadata = std::string('0', attrs->metadataSize);
  msg.doReceivedAction();
  CPPUNIT_ASSERT(!peer->isSeeder());
}

void HandshakeExtensionMessageTest::testCreate()
{
  std::string in = 
    "0d1:pi6881e1:v5:aria21:md6:ut_pexi1ee13:metadata_sizei1024ee";
  SharedHandle<HandshakeExtensionMessage> m =
    HandshakeExtensionMessage::create(reinterpret_cast<const unsigned char*>(in.c_str()),
                                      in.size());
  CPPUNIT_ASSERT_EQUAL(std::string("aria2"), m->getClientVersion());
  CPPUNIT_ASSERT_EQUAL((uint16_t)6881, m->getTCPPort());
  CPPUNIT_ASSERT_EQUAL((uint8_t)1, m->getExtensionMessageID("ut_pex"));
  CPPUNIT_ASSERT_EQUAL((size_t)1024, m->getMetadataSize());
  try {
    // bad payload format
    std::string in = "011:hello world";
    HandshakeExtensionMessage::create(reinterpret_cast<const unsigned char*>(in.c_str()),
                                      in.size());
    CPPUNIT_FAIL("exception must be thrown.");
  } catch(Exception& e) {
    std::cerr << e.stackTrace() << std::endl;
  }
  try {
    // malformed dencoded message
    std::string in = "011:hello";
    HandshakeExtensionMessage::create(reinterpret_cast<const unsigned char*>(in.c_str()),
                                      in.size());
    CPPUNIT_FAIL("exception must be thrown.");
  } catch(Exception& e) {
    std::cerr << e.stackTrace() << std::endl;
  }
  try {
    // 0 length data
    std::string in = "";
    HandshakeExtensionMessage::create(reinterpret_cast<const unsigned char*>(in.c_str()),
                                      in.size());
    CPPUNIT_FAIL("exception must be thrown.");
  } catch(Exception& e) {
    std::cerr << e.stackTrace() << std::endl;
  }    
}

void HandshakeExtensionMessageTest::testCreate_stringnum()
{
  std::string in = "0d1:p4:68811:v5:aria21:md6:ut_pex1:1ee";
  SharedHandle<HandshakeExtensionMessage> m =
    HandshakeExtensionMessage::create
    (reinterpret_cast<const unsigned char*>(in.c_str()),
     in.size());
  CPPUNIT_ASSERT_EQUAL(std::string("aria2"), m->getClientVersion());
  // port number in string is not allowed
  CPPUNIT_ASSERT_EQUAL((uint16_t)0, m->getTCPPort());
  // extension ID in string is not allowed
  CPPUNIT_ASSERT_EQUAL((uint8_t)0, m->getExtensionMessageID("ut_pex"));
}

} // namespace aria2
