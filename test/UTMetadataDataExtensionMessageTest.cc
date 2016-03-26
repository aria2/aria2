#include "UTMetadataDataExtensionMessage.h"

#include <iostream>

#include <cppunit/extensions/HelperMacros.h>

#include "BtConstants.h"
#include "PieceStorage.h"
#include "DownloadContext.h"
#include "DirectDiskAdaptor.h"
#include "ByteArrayDiskWriter.h"
#include "DownloadContext.h"
#include "MockPieceStorage.h"
#include "UTMetadataRequestTracker.h"
#include "bittorrent_helper.h"
#include "MessageDigest.h"
#include "message_digest_helper.h"

namespace aria2 {

class UTMetadataDataExtensionMessageTest : public CppUnit::TestFixture {

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
  CPPUNIT_ASSERT_EQUAL(
      std::string("d8:msg_typei1e5:piecei1e10:total_sizei16384ee") + data,
      msg.getPayload());
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
  auto diskAdaptor = std::make_shared<DirectDiskAdaptor>();
  ByteArrayDiskWriter* diskWriter;
  {
    auto dw = make_unique<ByteArrayDiskWriter>();
    diskWriter = dw.get();
    diskAdaptor->setDiskWriter(std::move(dw));
  }
  auto pieceStorage = make_unique<MockPieceStorage>();
  pieceStorage->setDiskAdaptor(diskAdaptor);
  auto tracker = make_unique<UTMetadataRequestTracker>();
  auto dctx = make_unique<DownloadContext>();

  std::string piece0 = std::string(METADATA_PIECE_SIZE, '0');
  std::string piece1 = std::string(METADATA_PIECE_SIZE, '1');
  std::string metadata = piece0 + piece1;

  unsigned char infoHash[INFO_HASH_LENGTH];
  message_digest::digest(infoHash, INFO_HASH_LENGTH,
                         MessageDigest::sha1().get(), metadata.data(),
                         metadata.size());
  {
    auto attrs = make_unique<TorrentAttribute>();
    attrs->infoHash = std::string(&infoHash[0], &infoHash[20]);
    dctx->setAttribute(CTX_ATTR_BT, std::move(attrs));
  }
  UTMetadataDataExtensionMessage m(1);
  m.setPieceStorage(pieceStorage.get());
  m.setUTMetadataRequestTracker(tracker.get());
  m.setDownloadContext(dctx.get());

  m.setIndex(1);
  m.setData(piece1);

  tracker->add(1);
  m.doReceivedAction();
  CPPUNIT_ASSERT(!tracker->tracks(1));

  pieceStorage->setDownloadFinished(true);
  // If piece is not tracked, it is ignored.
  m.setIndex(0);
  m.setData(piece0);
  m.doReceivedAction();

  tracker->add(0);
  m.doReceivedAction();

  CPPUNIT_ASSERT_EQUAL(metadata, diskWriter->getString());
}

} // namespace aria2
