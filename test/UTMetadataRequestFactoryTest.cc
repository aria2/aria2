#include "UTMetadataRequestFactory.h"

#include <vector>
#include <deque>

#include <cppunit/extensions/HelperMacros.h>

#include "MockPieceStorage.h"
#include "DownloadContext.h"
#include "Peer.h"
#include "BtMessage.h"
#include "extension_message_test_helper.h"
#include "BtHandshakeMessage.h"
#include "ExtensionMessage.h"
#include "UTMetadataRequestTracker.h"

namespace aria2 {

class UTMetadataRequestFactoryTest:public CppUnit::TestFixture {

  CPPUNIT_TEST_SUITE(UTMetadataRequestFactoryTest);
  CPPUNIT_TEST(testCreate);
  CPPUNIT_TEST_SUITE_END();
public:
  void testCreate();

  class MockPieceStorage2:public MockPieceStorage {
  public:
    std::set<size_t> missingIndexes;

    virtual SharedHandle<Piece> getMissingPiece(size_t index)
    {
      if(missingIndexes.find(index) != missingIndexes.end()) {
	return SharedHandle<Piece>(new Piece(index, 0));
      } else {
	return SharedHandle<Piece>();
      }
    }
  };
};


CPPUNIT_TEST_SUITE_REGISTRATION(UTMetadataRequestFactoryTest);

void UTMetadataRequestFactoryTest::testCreate()
{
  UTMetadataRequestFactory factory;
  SharedHandle<DownloadContext> dctx
    (new DownloadContext(METADATA_PIECE_SIZE, METADATA_PIECE_SIZE*2));
  factory.setDownloadContext(dctx);
  SharedHandle<MockPieceStorage2> ps(new MockPieceStorage2());
  ps->missingIndexes.insert(0);
  ps->missingIndexes.insert(1);
  SharedHandle<WrapExtBtMessageFactory> messageFactory
    (new WrapExtBtMessageFactory());
  factory.setBtMessageFactory(messageFactory);
  SharedHandle<Peer> peer(new Peer("peer", 6880));
  peer->allocateSessionResource(0, 0);
  factory.setPeer(peer);
  SharedHandle<UTMetadataRequestTracker> tracker
    (new UTMetadataRequestTracker());
  factory.setUTMetadataRequestTracker(tracker);

  std::deque<SharedHandle<BtMessage> > msgs;

  factory.create(msgs, 1, ps);
  CPPUNIT_ASSERT_EQUAL((size_t)1, msgs.size());

  msgs.clear();

  ps->missingIndexes.clear();
  factory.create(msgs, 1, ps);
  CPPUNIT_ASSERT_EQUAL((size_t)0, msgs.size());
}

} // namespace aria2
