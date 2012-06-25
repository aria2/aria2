#include "PeerSessionResource.h"

#include <cppunit/extensions/HelperMacros.h>

#include "MockBtMessageDispatcher.h"
#include "Exception.h"
#include "util.h"

namespace aria2 {

class PeerSessionResourceTest:public CppUnit::TestFixture {

  CPPUNIT_TEST_SUITE(PeerSessionResourceTest);
  CPPUNIT_TEST(testPeerAllowedIndexSetContains);
  CPPUNIT_TEST(testAmAllowedIndexSetContains);
  CPPUNIT_TEST(testHasAllPieces);
  CPPUNIT_TEST(testHasPiece);
  CPPUNIT_TEST(testUpdateUploadLength);
  CPPUNIT_TEST(testUpdateDownloadLength);
  CPPUNIT_TEST(testExtendedMessageEnabled);
  CPPUNIT_TEST(testGetExtensionMessageID);
  CPPUNIT_TEST(testFastExtensionEnabled);
  CPPUNIT_TEST(testSnubbing);
  CPPUNIT_TEST(testAmChoking);
  CPPUNIT_TEST(testAmInterested);
  CPPUNIT_TEST(testPeerChoking);
  CPPUNIT_TEST(testPeerInterested);
  CPPUNIT_TEST(testChokingRequired);
  CPPUNIT_TEST(testOptUnchoking);
  CPPUNIT_TEST(testShouldBeChoking);
  CPPUNIT_TEST(testCountOutstandingRequest);
  CPPUNIT_TEST_SUITE_END();
public:
  void setUp() {}

  void tearDown() {}

  void testPeerAllowedIndexSetContains();
  void testAmAllowedIndexSetContains();
  void testHasAllPieces();
  void testHasPiece();
  void testUpdateUploadLength();
  void testUpdateDownloadLength();
  void testExtendedMessageEnabled();
  void testGetExtensionMessageID();
  void testFastExtensionEnabled();
  void testSnubbing();
  void testAmChoking();
  void testAmInterested();
  void testPeerChoking();
  void testPeerInterested();
  void testChokingRequired();
  void testOptUnchoking();
  void testShouldBeChoking();
  void testCountOutstandingRequest();
};


CPPUNIT_TEST_SUITE_REGISTRATION(PeerSessionResourceTest);

void PeerSessionResourceTest::testPeerAllowedIndexSetContains()
{
  PeerSessionResource res(1024, 1024*1024);

  res.addPeerAllowedIndex(567);
  res.addPeerAllowedIndex(789);

  CPPUNIT_ASSERT(res.peerAllowedIndexSetContains(567));
  CPPUNIT_ASSERT(res.peerAllowedIndexSetContains(789));
  CPPUNIT_ASSERT(!res.peerAllowedIndexSetContains(123));
}

void PeerSessionResourceTest::testAmAllowedIndexSetContains()
{
  PeerSessionResource res(1024, 1024*1024);
  
  res.addAmAllowedIndex(567);
  res.addAmAllowedIndex(789);

  CPPUNIT_ASSERT(res.amAllowedIndexSetContains(567));
  CPPUNIT_ASSERT(res.amAllowedIndexSetContains(789));
  CPPUNIT_ASSERT(!res.amAllowedIndexSetContains(123));
}

void PeerSessionResourceTest::testHasAllPieces()
{
  PeerSessionResource res(1024, 1024*1024);

  CPPUNIT_ASSERT(!res.hasAllPieces());
  res.markSeeder();
  CPPUNIT_ASSERT(res.hasAllPieces());
}

void PeerSessionResourceTest::testHasPiece()
{
  PeerSessionResource res(1024, 1024*1024);

  CPPUNIT_ASSERT(!res.hasPiece(300));
  res.updateBitfield(300, 1);
  CPPUNIT_ASSERT(res.hasPiece(300));
  res.updateBitfield(300, 0);
  CPPUNIT_ASSERT(!res.hasPiece(300));
}  

void PeerSessionResourceTest::testUpdateUploadLength()
{
  PeerSessionResource res(1024, 1024*1024);

  CPPUNIT_ASSERT_EQUAL((int64_t)0LL, res.uploadLength());
  res.updateUploadLength(100);
  res.updateUploadLength(200);
  CPPUNIT_ASSERT_EQUAL((int64_t)300LL, res.uploadLength());
}

void PeerSessionResourceTest::testUpdateDownloadLength()
{
  PeerSessionResource res(1024, 1024*1024);

  CPPUNIT_ASSERT_EQUAL((int64_t)0LL, res.downloadLength());
  res.updateDownloadLength(100);
  res.updateDownloadLength(200);
  CPPUNIT_ASSERT_EQUAL((int64_t)300LL, res.downloadLength());
}

void PeerSessionResourceTest::testExtendedMessageEnabled()
{ 
  PeerSessionResource res(1024, 1024*1024);

  CPPUNIT_ASSERT(!res.extendedMessagingEnabled());
  res.extendedMessagingEnabled(true);
  CPPUNIT_ASSERT(res.extendedMessagingEnabled());
  res.extendedMessagingEnabled(false);
  CPPUNIT_ASSERT(!res.extendedMessagingEnabled());
}

void PeerSessionResourceTest::testGetExtensionMessageID()
{
  PeerSessionResource res(1024, 1024*1024);

  res.addExtension("a2", 9);
  CPPUNIT_ASSERT_EQUAL((uint8_t)9, res.getExtensionMessageID("a2"));
  CPPUNIT_ASSERT_EQUAL((uint8_t)0, res.getExtensionMessageID("non"));

  CPPUNIT_ASSERT_EQUAL(std::string("a2"), res.getExtensionName(9));
  CPPUNIT_ASSERT_EQUAL(std::string(""), res.getExtensionName(10));
}

void PeerSessionResourceTest::testFastExtensionEnabled()
{
  PeerSessionResource res(1024, 1024*1024);

  CPPUNIT_ASSERT(!res.fastExtensionEnabled());
  res.fastExtensionEnabled(true);
  CPPUNIT_ASSERT(res.fastExtensionEnabled());
  res.fastExtensionEnabled(false);
  CPPUNIT_ASSERT(!res.fastExtensionEnabled());
}

void PeerSessionResourceTest::testSnubbing()
{
  PeerSessionResource res(1024, 1024*1024);

  CPPUNIT_ASSERT(!res.snubbing());
  res.snubbing(true);
  CPPUNIT_ASSERT(res.snubbing());
  res.snubbing(false);
  CPPUNIT_ASSERT(!res.snubbing());
}

void PeerSessionResourceTest::testAmChoking()
{
  PeerSessionResource res(1024, 1024*1024);

  CPPUNIT_ASSERT(res.amChoking());
  res.amChoking(false);
  CPPUNIT_ASSERT(!res.amChoking());
  res.amChoking(true);
  CPPUNIT_ASSERT(res.amChoking());
}

void PeerSessionResourceTest::testAmInterested()
{
  PeerSessionResource res(1024, 1024*1024);

  CPPUNIT_ASSERT(!res.amInterested());
  res.amInterested(true);
  CPPUNIT_ASSERT(res.amInterested());
  res.amInterested(false);
  CPPUNIT_ASSERT(!res.amInterested());
}

void PeerSessionResourceTest::testPeerChoking()
{
  PeerSessionResource res(1024, 1024*1024);

  CPPUNIT_ASSERT(res.peerChoking());
  res.peerChoking(false);
  CPPUNIT_ASSERT(!res.peerChoking());
  res.peerChoking(true);
  CPPUNIT_ASSERT(res.peerChoking());
}

void PeerSessionResourceTest::testPeerInterested()
{
  PeerSessionResource res(1024, 1024*1024);

  CPPUNIT_ASSERT(!res.peerInterested());
  res.peerInterested(true);
  CPPUNIT_ASSERT(res.peerInterested());
  res.peerInterested(false);
  CPPUNIT_ASSERT(!res.peerInterested());
}

void PeerSessionResourceTest::testChokingRequired()
{
  PeerSessionResource res(1024, 1024*1024);

  CPPUNIT_ASSERT(res.chokingRequired());
  res.chokingRequired(false);
  CPPUNIT_ASSERT(!res.chokingRequired());
  res.chokingRequired(true);
  CPPUNIT_ASSERT(res.chokingRequired());
}

void PeerSessionResourceTest::testOptUnchoking()
{
  PeerSessionResource res(1024, 1024*1024);

  CPPUNIT_ASSERT(!res.optUnchoking());
  res.optUnchoking(true);
  CPPUNIT_ASSERT(res.optUnchoking());
  res.optUnchoking(false);
  CPPUNIT_ASSERT(!res.optUnchoking());
}

void PeerSessionResourceTest::testShouldBeChoking()
{
  PeerSessionResource res(1024, 1024*1024);
  
  CPPUNIT_ASSERT(res.shouldBeChoking());
  res.chokingRequired(false);
  CPPUNIT_ASSERT(!res.shouldBeChoking());
  res.chokingRequired(true);
  res.optUnchoking(true);
  CPPUNIT_ASSERT(!res.shouldBeChoking());
}

void PeerSessionResourceTest::testCountOutstandingRequest()
{
  PeerSessionResource res(1024, 1024*1024);
  SharedHandle<MockBtMessageDispatcher> dispatcher
    (new MockBtMessageDispatcher());
  res.setBtMessageDispatcher(dispatcher.get());

  CPPUNIT_ASSERT_EQUAL((size_t)0, res.countOutstandingUpload());
}

} // namespace aria2
